/*
 * @ 名称: can_tp.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "includes.h"

#ifdef EN_CAN_TP
#include "can_tp.h"
#include "TP_cfg.h"

/*********************************************************
**  SF  --  signle frame
**  FF  --  first frame
**  FC  --  flow control
**  CF  --  consective frame
*********************************************************/

typedef enum {
    IDLE,      /*idle*/
    RX_SF,   /*wait signle frame*/
    RX_FF,   /*wait first frame*/
    RX_FC,   /*wait flow control frame*/
    RX_CF,   /*wait consective frame*/

    TX_SF,     /*tx signle frame*/
    TX_FF,     /*tx first frame*/
    TX_FC,     /*tx flow control*/
    TX_CF,     /*tx consective frame*/

    WAITTING_TX, /*watting tx message*/

    WAIT_CONFIRM /*wait confrim*/
} tCanTpWorkStatus;

typedef enum {
    SF,        /*signle frame value*/
    FF,        /*first frame value*/
    CF,        /*consective frame value*/
    FC         /*flow control value*/
} tNetWorkFrameType;

typedef enum {
    CONTINUE_TO_SEND, /*continue to send*/
    WAIT_FC,          /*wait flow control*/
    OVERFLOW_BUF      /*overflow buf*/
} tFlowStatus;

typedef enum {
    N_OK = 0,    /*This value means that the service execution has completed successfully; it can be issued to a service user on both the sender and receiver side*/
    N_TIMEOUT_A, /*This value is issued to the protocol user when the timer N_Ar/N_As has passed its time-out
                            value N_Asmax/N_Armax; it can be issued to service user on both the sender and receiver side.*/
    N_TIMEOUT_Bs, /*This value is issued to the service user when the timer N_Bs has passed its time-out value
                                N_Bsmax; it can be issued to the service user on the sender side only.*/
    N_TIMEOUT_Cr, /*This value is issued to the service user when the timer N_Cr has passed its time-out value
                    N_Crmax; it can be issued to the service user on the receiver side only.*/
    N_WRONG_SN,   /*This value is issued to the service user upon reception of an unexpected sequence number
                    (PCI.SN) value; it can be issued to the service user on the receiver side only.*/
    N_INVALID_FS, /*This value is issued to the service user when an invalid or unknown FlowStatus value has
                    been received in a flow control (FC) N_PDU; it can be issued to the service user on the sender side only.*/
    N_UNEXP_PDU,  /*This value is issued to the service user upon reception of an unexpected protocol data unit;
                    it can be issued to the service user on the receiver side only.*/
    N_WTF_OVRN,   /*This value is issued to the service user upon reception of flow control WAIT frame that
                    exceeds the maximum counter N_WFTmax.*/
    N_BUFFER_OVFLW, /*This value is issued to the service user upon reception of a flow control (FC) N_PDU with
                    FlowStatus = OVFLW. It indicates that the buffer on the receiver side of a segmented
                    message transmission cannot store the number of bytes specified by the FirstFrame
                    DataLength (FF_DL) parameter in the FirstFrame and therefore the transmission of the
                    segmented message was aborted. It can be issued to the service user on the sender side
                    only.*/
    N_ERROR       /*This is the general error value. It shall be issued to the service user when an error has been
                    detected by the network layer and no other parameter value can be used to better describe
                    the error. It can be issued to the service user on both the sender and receiver side.*/
} tN_Result;

typedef enum {
    CANTP_TX_MSG_IDLE = 0, /*CAN TP tx message idle*/
    CANTP_TX_MSG_SUCC,     /*CAN TP tx message successful*/
    CANTP_TX_MSG_FAIL,     /*CAN TP tx message fail*/
    CANTP_TX_MSG_WAITTING /*CAN TP waitting tx message*/
} tCanTPTxMsgStatus;

typedef struct {
    tUdsId xCanTpId;                           /*can tp message id*/
    tCanTpDataLen xPduDataLen;                 /*pdu data len(Rx/Tx data len)*/
    tCanTpDataLen xFFDataLen;                  /*Rx/Tx FF data len*/
    uint8 aDataBuf[MAX_CF_DATA_LEN]; /*Rx/Tx data buf*/
} tCanTpDataInfo;

typedef struct {
    uint8 ucSN;          /*SN*/
    uint8 ucBlockSize;   /*Block size*/
    tNetTime xSTmin;             /*STmin*/
    tNetTime xMaxWatiTimeout;    /*timeout time*/
    tCanTpDataInfo stCanTpDataInfo;
} tCanTpInfo;

typedef struct {
    uint8 isFree;            /*rx message status. TRUE = not received messag.*/
    tUdsId xMsgId;                     /*received message id*/
    uint8 msgLen;            /*received message len*/
    uint8 aMsgBuf[DATA_LEN]; /*message data buf*/
} tCanTpMsg;

typedef tN_Result (*tpfCanTpFun)(tCanTpMsg *, tCanTpWorkStatus *);
typedef struct {
    tCanTpWorkStatus eCanTpStaus;
    tpfCanTpFun pfCanTpFun;
} tCanTpFunInfo;

/***********************Global value*************************/
static tCanTpInfo gs_stCanTPTxDataInfo; /*can tp tx data*/
static tNetTime gs_xCanTPTxSTmin = 0u; /*tx STmin*/
static uint32 gs_CANTPTxMsgMaxWaitTime = 0u;/*tx message max wait time, RX / TX frame both used waitting status*/
static tCanTpInfo gs_stCanTPRxDataInfo; /*can tp rx data*/
static tCanTpWorkStatus gs_eCanTpWorkStatus = IDLE;
static volatile tCanTPTxMsgStatus gs_eCANTPTxMsStatus = CANTP_TX_MSG_IDLE;
static tpfNetTxCallBack gs_pfCANTPTxMsgCallBack = NULL_PTR;
/*********************************************************/

/***********************Static function***********************/
#define CanTpTimeToCount(xTime) ((xTime) / g_stCANUdsNetLayerCfgInfo.ucCalledPeriod)
#define IsSF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) ==  SF) ? TRUE : FALSE)
#define IsFF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) ==  FF) ? TRUE : FALSE)
#define IsCF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) ==  CF) ? TRUE : FALSE)
#define IsFC(xNetWorkFrameType) ((((xNetWorkFrameType)>> 4u) ==  FC) ? TRUE : FALSE)
#define IsRevSNValid(xSN) ((gs_stCanTPRxDataInfo.ucSN == ((xSN) & 0x0Fu)) ? TRUE : FALSE)
#define AddWaitSN()\
    do{\
        gs_stCanTPRxDataInfo.ucSN++;\
        if(gs_stCanTPRxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stCanTPRxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

#define GetFrameLen(pucRevData, pxDataLen)\
    do{\
        if(TRUE == IsFF(pucRevData[0u]))\
        {\
            *(pxDataLen) = ((uint16)(pucRevData[0u] & 0x0fu) << 8u) | (uint16)pucRevData[1u];\
        }\
        else\
        {\
            *(pxDataLen) = (uint8)(pucRevData[0u] & 0x0fu);\
        }\
    }while(0u)

/*get RX SF frame message length*/
static boolean GetRXSFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen);

/*get RX FF frame message length*/
static boolean GetRXFFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen);

/*check received message length valid or not?*/
#define IsRxMsgLenValid(address_type, frameLen, RXCANMsgLen) ((address_type == NORMAL_ADDRESSING) ? (frameLen <= RXCANMsgLen - 1) : (frameLen <= RXCANMsgLen - 2))

/*save FF data len*/
#define SaveFFDataLen(i_xRevFFDataLen) (gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen = i_xRevFFDataLen)

/*set BS*/
#define SetBlockSize(pucBSBuf, xBlockSize) (*(pucBSBuf) = (uint8)(xBlockSize))

/*add block size*/
#define AddBlockSize()\
    do{\
        if(0u != g_stCANUdsNetLayerCfgInfo.xBlockSize)\
        {\
            gs_stCanTPRxDataInfo.ucBlockSize++;\
        }\
    }while(0u)

/*set STmin*/
#define SetSTmin(pucSTminBuf, xSTmin) (*(pucSTminBuf) = (uint8)(xSTmin))

/*set wait  STmin*/
#define SetWaitSTmin() (gs_stCanTPRxDataInfo.xSTmin = CanTpTimeToCount(g_stCANUdsNetLayerCfgInfo.xSTmin))

/*set wait frame time*/
#define SetRxWaitFrameTime(xWaitTimeout) do{\
        (gs_stCanTPRxDataInfo.xMaxWatiTimeout = CanTpTimeToCount(xWaitTimeout));\
        gs_CANTPTxMsgMaxWaitTime = gs_stCanTPRxDataInfo.xMaxWatiTimeout;\
    }while(0u);

/*RX frame set Rx msg wait time*/
#define RXFrame_SetRxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/*RX frame set Tx msg wait time*/
#define RXFrame_SetTxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/*set wait SN*/
#define SetWaitSN(xSN) (gs_stCanTPRxDataInfo.ucSN = xSN)

/*set FS*/
#define SetFS(pucFsBuf, xFlowStatus) (*(pucFsBuf) = (*(pucFsBuf) & 0xF0u) | (uint8)(xFlowStatus))

/*clear receive data buf*/
#define ClearRevDataBuf()\
    do{\
        fsl_memset(&gs_stCanTPRxDataInfo,0u,sizeof(gs_stCanTPRxDataInfo));\
    }while(0u)

/*add rev data len*/
#define AddRevDataLen(xRevDataLen) (gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen += (xRevDataLen))

/*Is rev conective frame all.*/
#define IsReciveCFAll(xCFDataLen) (((gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen + (uint8)(xCFDataLen))\
                                    >= gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/*Is STmin timeout?*/
#define IsSTminTimeOut() ((0u == gs_stCanTPRxDataInfo.xSTmin) ? TRUE : FALSE)

/*Is wait Flow control timeout?*/
#define IsWaitFCTimeout()  ((0u == gs_stCanTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*Is wait conective frame timeout?*/
#define IsWaitCFTimeout() ((0u == gs_stCanTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*Is block sizeo overflow*/
#define IsRxBlockSizeOverflow() (((0u != g_stCANUdsNetLayerCfgInfo.xBlockSize) &&\
                                  (gs_stCanTPRxDataInfo.ucBlockSize >= g_stCANUdsNetLayerCfgInfo.xBlockSize))\
                                 ? TRUE : FALSE)

/*Is transmitted data len overflow max SF?*/
#define IsTxDataLenOverflowSF() ((gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen > TX_SF_DATA_MAX_LEN) ? TRUE : FALSE)

/*Is transmitted data less than min?*/
#define IsTxDataLenLessSF() ((0u == gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/*set transmitted SF data len*/
#define SetTxSFDataLen(pucSFDataLenBuf, xTxSFDataLen) \
    do{\
        *(pucSFDataLenBuf) &= 0xF0u;\
        (*(pucSFDataLenBuf) |= (xTxSFDataLen));\
    }while(0u)

/*set transmitted FF data len*/
#define SetTxFFDataLen(pucTxFFDataLenBuf, xTxFFDataLen)\
    do{\
        *(pucTxFFDataLenBuf + 0u) &= 0xF0u;\
        *(pucTxFFDataLenBuf + 0u) |= (uint8)((xTxFFDataLen) >> 8u);\
        *(pucTxFFDataLenBuf + 1u) |= (uint8)(xTxFFDataLen);\
    }while(0u)

/*add Tx data len*/
#define AddTxDataLen(xTxDataLen) (gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen += (xTxDataLen))

/*set tx STmin */
#define SetTxSTmin() (gs_stCanTPTxDataInfo.xSTmin = CanTpTimeToCount(gs_xCanTPTxSTmin))

/*save Tx STmin*/
#define SaveTxSTmin(xTxSTmin) (gs_xCanTPTxSTmin = xTxSTmin)

/*Is Tx STmin timeout?*/
#define IsTxSTminTimeout() ((0u == gs_stCanTPTxDataInfo.xSTmin) ? TRUE : FALSE)

/*Set tx wait frame time*/
#define SetTxWaitFrameTime(xWaitTime) do{\
        (gs_stCanTPTxDataInfo.xMaxWatiTimeout = CanTpTimeToCount(xWaitTime));\
        gs_CANTPTxMsgMaxWaitTime = gs_stCanTPTxDataInfo.xMaxWatiTimeout;\
    }while(0u);


/*TX frame set tx message wait time*/
#define TXFrame_SetTxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/*TX frame set Tx message wait time*/
#define TXFrame_SetRxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/*Is Tx wait frame timeout?*/
#define IsTxWaitFrameTimeout() ((0u == gs_stCanTPTxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*Is Tx message wait frame timeout?*/
#define IsTxMsgWaittingFrameTimeout() ((0u == gs_CANTPTxMsgMaxWaitTime) ? TRUE : FALSE)

/*Get FS*/
#define GetFS(ucFlowStaus, pxFlowStatusBuf) (*(pxFlowStatusBuf) = (ucFlowStaus) & 0x0Fu)

/*set tx SN*/
#define SetTxSN(pucSNBuf) (*(pucSNBuf) = gs_stCanTPTxDataInfo.ucSN | (*(pucSNBuf) & 0xF0u))

/*Add Tx SN*/
#define AddTxSN()\
    do{\
        gs_stCanTPTxDataInfo.ucSN++;\
        if(gs_stCanTPTxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stCanTPTxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

/*Is Tx all*/
#define IsTxAll() ((gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen >= \
                    gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/*save received message ID*/
#define SaveRxMsgId(xMsgId) (gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId = (xMsgId))

/*clear can tp Rx msg buf*/
#define ClearCanTpRxMsgBuf(pMsgInfo)\
    do{\
        (pMsgInfo)->isFree = TRUE;\
        (pMsgInfo)->msgLen = 0u;\
        (pMsgInfo)->xMsgId = 0u;\
    }while(0u)

/*get cur CAN TP status*/
#define GetCurCANTPStatus() (gs_eCanTpWorkStatus)

/*set cur CAN TP status*/
#define SetCurCANTPSatus(status) \
    do{\
        gs_eCanTpWorkStatus = status;\
    }while(0u)

/*get cur CAN TP status PTR*/
#define GetCurCANTPStatusPtr() (&gs_eCanTpWorkStatus)

/*can tp IDLE*/
static tN_Result CANTP_DoCanTpIdle(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*do receive signle frame*/
static tN_Result CANTP_DoReceiveSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*do receive first frame*/
static tN_Result CANTP_DoReceiveFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*do receive conective frame*/
static tN_Result CANTP_DoReceiveCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*transmit FC callback*/
static void CANTP_DoTransmitFCCallBack(void);

/*Transmit flow control frame*/
static tN_Result CANTP_DoTransmitFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*transmit signle frame*/
static tN_Result CANTP_DoTransmitSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*transmit SF callback*/
static void CANTP_DoTransmitSFCallBack(void);

/*transmit FF callback*/
static void CANTP_DoTransmitFFCallBack(void);

/*transmitt first frame*/
static tN_Result CANTP_DoTransmitFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*wait flow control frame*/
static tN_Result CANTP_DoReceiveFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*transmit CF callback*/
static void CANTP_DoTransmitCFCallBack(void);

/*transmit conective frame*/
static tN_Result CANTP_DoTransmitCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/*waitting tx message*/
static tN_Result CANTP_DoWaittingTxMsg(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);


/*set transmit frame type*/
static uint8 CANTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8 *o_pucFrameType);

/*received a can tp frame, copy these data in fifo.*/
static uint8 CANTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxCanID,
                                          const tLen i_xRxDataLen,
                                          const uint8 *i_pDataBuf
                                         );

/*uds transmitted a application frame data, copy these data in TX fifo.*/
static uint8 CANTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxCanID,
                                           uint8 *o_pTxDataLen,
                                           uint8 *o_pDataBuf);

/*CAN TP TX message callback*/
static void CANTP_TxMsgSuccessfulCallBack(void);

/*CANP TP set TX message status*/
static void CANTP_SetTxMsgStatus(const tCanTPTxMsgStatus i_eTxMsgStatus);

/*Register tx message successful callback*/
static void CANTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack);

/*Do register tx message callback*/
static void CANTP_DoRegisterTxMsgCallBack(void);

/*********************************************************/

static const tCanTpFunInfo gs_astCanTpFunInfo[] = {
    {IDLE, CANTP_DoCanTpIdle},
    {RX_SF, CANTP_DoReceiveSF},
    {RX_FF, CANTP_DoReceiveFF},
    {TX_FC, CANTP_DoTransmitFC},
    {RX_CF, CANTP_DoReceiveCF},

    {TX_SF, CANTP_DoTransmitSF},
    {TX_FF, CANTP_DoTransmitFF},
    {RX_FC, CANTP_DoReceiveFC},
    {TX_CF, CANTP_DoTransmitCF},
    {WAITTING_TX, CANTP_DoWaittingTxMsg}
};

/*can TP init*/
void CANTP_Init(void)
{
    tErroCode eStatus;

    ApplyFifo(RX_TP_QUEUE_LEN, RX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply RX_TP_QUEUE_ID failed!\n");

        while (1) {

        }
    }

    ApplyFifo(TX_TP_QUEUE_LEN, TX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply TX_TP_QUEUE_ID failed\n");

        while (1) {

        }
    }

    ApplyFifo(RX_BUS_FIFO_LEN, RX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply RX_BUS_FIFO failed!\n");

        while (1) {

        }
    }

    ApplyFifo(TX_BUS_FIFO_LEN, TX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply TX_BUS_FIFOfailed!\n");

        while (1) {

        }
    }

}

/*can tp system tick control. This function should period called by system.*/
void CANTP_SytstemTickControl(void)
{
    if (gs_stCanTPRxDataInfo.xSTmin) {
        gs_stCanTPRxDataInfo.xSTmin--;
    }

    if (gs_stCanTPRxDataInfo.xMaxWatiTimeout) {
        gs_stCanTPRxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_stCanTPTxDataInfo.xSTmin) {
        gs_stCanTPTxDataInfo.xSTmin--;
    }

    if (gs_stCanTPTxDataInfo.xMaxWatiTimeout) {
        gs_stCanTPTxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_CANTPTxMsgMaxWaitTime) {
        gs_CANTPTxMsgMaxWaitTime--;
    }
}

/*uds network man function*/
void CANTP_MainFun(void)
{
    uint8 index = 0u;
    const uint8 findCnt = sizeof(gs_astCanTpFunInfo) / sizeof(gs_astCanTpFunInfo[0u]);
    tCanTpMsg stRxCanTpMsg = {TRUE, 0u, 0u, {0u}};
    tN_Result result = N_OK;

    /*In waitting TX message, cannot read message from FIFO. Because, In waitting message will lost read messages.*/
    if (WAITTING_TX != GetCurCANTPStatus()) {
        /*read msg from CAN driver RxFIFO*/
        if (TRUE == g_stCANUdsNetLayerCfgInfo.pfNetRx(&stRxCanTpMsg.xMsgId,
                                                      &stRxCanTpMsg.msgLen,
                                                      stRxCanTpMsg.aMsgBuf)) {
            /*check received message ID valid?*/
            if (TRUE == CANTP_IsReceivedMsgIDValid(stRxCanTpMsg.xMsgId)) {
                stRxCanTpMsg.isFree = FALSE;
            }
        }
    }

    while (index < findCnt) {
        if (GetCurCANTPStatus() == gs_astCanTpFunInfo[index].eCanTpStaus) {
            if (NULL_PTR != gs_astCanTpFunInfo[index].pfCanTpFun) {
                result = gs_astCanTpFunInfo[index].pfCanTpFun(&stRxCanTpMsg, GetCurCANTPStatusPtr());
            }
        }

        /*if received unexpect PDU, then jump to IDLE and restart do progrocess.*/
        if (N_UNEXP_PDU != result) {
            if (N_OK != result) {
                SetCurCANTPSatus(IDLE);
            }

            index++;
        } else {
            index = 0u;
        }

    }

    ClearCanTpRxMsgBuf(&stRxCanTpMsg);

    /*check CAN TP tx message successful?*/
    CANTP_DoRegisterTxMsgCallBack();
}

/*received a can tp frame, copy these data in UDS RX fifo.*/
static uint8 CANTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxCanID,
                                          const tLen i_xRxDataLen,
                                          const uint8 *i_pDataBuf)
{
    tErroCode eStatus;
    tLen xCanWriteLen = 0u;

    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    ASSERT(NULL_PTR == i_pDataBuf);

    if (0u == i_xRxDataLen) {
        return FALSE;
    }

    /*check can wirte data len*/
    GetCanWriteLen(RX_TP_QUEUE_ID, &xCanWriteLen, &eStatus);

    if ((ERRO_NONE != eStatus) || (xCanWriteLen < (i_xRxDataLen + sizeof(tUDSAndTPExchangeMsgInfo)))) {
        return FALSE;
    }

    exchangeMsgInfo.msgID = i_xRxCanID;
    exchangeMsgInfo.dataLen = i_xRxDataLen;
    exchangeMsgInfo.pfCallBack = NULL_PTR;

    /*write data uds transmitt ID and data len*/
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8 *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);

    if (ERRO_NONE != eStatus) {
        return FALSE;
    }

    /*write data in fifo*/
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8 *)i_pDataBuf, i_xRxDataLen, &eStatus);

    if (ERRO_NONE != eStatus) {
        return FALSE;
    }

    return TRUE;
}

/*uds transmitted a application frame data, copy these data in TX fifo.*/
static uint8 CANTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxCanID,
                                           uint8 *o_pTxDataLen,
                                           uint8 *o_pDataBuf)
{
    tErroCode eStatus;
    tLen xRealReadLen = 0u;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    ASSERT(NULL_PTR == o_pxTxCanID);
    ASSERT(NULL_PTR == o_pTxDataLen);
    ASSERT(NULL_PTR == o_pDataBuf);

    /*can read data from buf*/
    GetCanReadLen(TX_TP_QUEUE_ID, &xRealReadLen, &eStatus);

    if ((ERRO_NONE != eStatus) || (0u == xRealReadLen) || (xRealReadLen < sizeof(tUDSAndTPExchangeMsgInfo))) {
        return FALSE;
    }

    /*read receive ID*/
    ReadDataFromFifo(TX_TP_QUEUE_ID,
                     sizeof(tUDSAndTPExchangeMsgInfo),
                     (uint8 *)&exchangeMsgInfo,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || sizeof(tUDSAndTPExchangeMsgInfo) != xRealReadLen) {
        return FALSE;
    }

    /*read data from fifo*/
    ReadDataFromFifo(TX_TP_QUEUE_ID,
                     exchangeMsgInfo.dataLen,
                     o_pDataBuf,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || exchangeMsgInfo.dataLen != xRealReadLen) {
        return FALSE;
    }

    *o_pxTxCanID = exchangeMsgInfo.msgID;

    *o_pTxDataLen = exchangeMsgInfo.dataLen;

    TP_RegisterTransmittedAFrmaeMsgCallBack(exchangeMsgInfo.pfCallBack);

    return TRUE;
}

/*can tp IDLE*/
static tN_Result CANTP_DoCanTpIdle(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 txDataLen = (uint8)gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*clear can tp data*/
    fsl_memset((void *)&gs_stCanTPRxDataInfo, 0u, sizeof(tCanTpInfo));
    fsl_memset((void *)&gs_stCanTPTxDataInfo, 0u, sizeof(tCanTpInfo));

    /*clear waitting time*/
    gs_CANTPTxMsgMaxWaitTime = 0u;

    /*set NULL to transmitted message callback*/
    TP_RegisterTransmittedAFrmaeMsgCallBack(NULL_PTR);

    /*If receive can tp message, judge type. Only received SF or FF message.
    Others frame ignore.*/
    if (FALSE == m_stMsgInfo->isFree) {
        if (TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u])) {
            *m_peNextStatus = RX_SF;
        } else if (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u])) {
            *m_peNextStatus = RX_FF;
        } else {
            TPDebugPrintf("\n %s received invalid message!\n", __func__);
        }
    } else {
        /*Judge have message can will tx.*/
        if (TRUE == CANTP_CopyAFrameFromFifoToBuf(&gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                  &txDataLen,
                                                  gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf)) {
            gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen = txDataLen;

            if (TRUE == IsTxDataLenOverflowSF()) {
                *m_peNextStatus = TX_FF;
            } else {
                *m_peNextStatus = TX_SF;
            }
        }
    }

    return N_OK;
}

/*do receive signle frame*/
static tN_Result CANTP_DoReceiveSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint32 SFLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree)) {
        return N_ERROR;
    }

    if (TRUE != IsSF(m_stMsgInfo->aMsgBuf[0u])) {
        return N_ERROR;
    }

    /*Get RX frame: SF length*/
    if (TRUE != GetRXSFFrameMsgLength(m_stMsgInfo->msgLen, m_stMsgInfo->aMsgBuf, &SFLen)) {
        TPDebugPrintf("SF:GetRXSFFrameMsgLength failed!\n");

        return N_ERROR;
    }

    /*write data to UDS fifo*/
    if (FALSE == CANTP_CopyAFrameDataInRxFifo(m_stMsgInfo->xMsgId,
                                              SFLen,
                                              &m_stMsgInfo->aMsgBuf[1u])) {
        TPDebugPrintf("copy data erro!\n");

        return N_ERROR;
    }

    *m_peNextStatus = IDLE;

    return N_OK;
}

/*do receive first frame*/
static tN_Result CANTP_DoReceiveFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint32 FFDataLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree)) {
        return N_ERROR;
    }

    if (TRUE != IsFF(m_stMsgInfo->aMsgBuf[0u])) {
        TPDebugPrintf("Received not FF\n");

        return N_ERROR;
    }

    /*get FF Data len*/
    if (TRUE != GetRXFFFrameMsgLength(m_stMsgInfo->msgLen, m_stMsgInfo->aMsgBuf, &FFDataLen)) {
        TPDebugPrintf("FF:GetRXFrameMsgLength failed!\n");

        return N_ERROR;
    }

    /*save received msg ID*/
    SaveRxMsgId(m_stMsgInfo->xMsgId);

    /*write data in global buf. When receive all data, write these data in fifo.*/
    SaveFFDataLen(FFDataLen);

    /*set wait flow control time*/
    RXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBr);

    /*copy data in golbal buf*/
    fsl_memcpy(gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf, (const void *)&m_stMsgInfo->aMsgBuf[2u], m_stMsgInfo->msgLen - 2u);

    AddRevDataLen(m_stMsgInfo->msgLen - 2u);

    /*jump to next status*/
    *m_peNextStatus = TX_FC;

    ClearCanTpRxMsgBuf(m_stMsgInfo);

    return N_OK;
}


/*do receive conective frame*/
static tN_Result CANTP_DoReceiveCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    ASSERT(NULL_PTR == m_peNextStatus);

    /*Is timeout rx wait timeout? If wait time out receive CF over.*/
    if (TRUE == IsWaitCFTimeout()) {
        TPDebugPrintf("wait conective frame timeout!\n");

        *m_peNextStatus = IDLE;

        return N_TIMEOUT_Cr;
    }

    if (0u == m_stMsgInfo->msgLen || TRUE == m_stMsgInfo->isFree) {
        /*waitting CF message, It's normally for not received CAN message in the step.*/
        return N_OK;
    }

    /*check received msssage is SF or FF? If received SF or FF, start new receive progrocess.*/
    if ((TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u])) || (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u]))) {
        TPDebugPrintf("In receive progrocess: received SF\n");

        *m_peNextStatus = IDLE;

        return N_UNEXP_PDU;
    }

    if (gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId != m_stMsgInfo->xMsgId) {
#ifdef EN_TP_DEBUG
        TPDebugPrintf("Msg ID invalid in CF! F RX ID = %X, RX ID = %X\n",
                      gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId, m_stMsgInfo->xMsgId);
#endif

        return N_ERROR;
    }

    if (TRUE != IsCF(m_stMsgInfo->aMsgBuf[0u])) {
#ifdef EN_TP_DEBUG
        TPDebugPrintf("Msg type invalid in CF %X!\n", m_stMsgInfo->aMsgBuf[0u]);
#endif

        return N_ERROR;
    }

    /*Get rev SN. If SN invalid, return FALSE.*/
    if (TRUE != IsRevSNValid(m_stMsgInfo->aMsgBuf[0u])) {
        TPDebugPrintf("Msg SN invalid in CF!\n");

        return N_WRONG_SN;
    }

    /*check receive cf all? If receive all, copy data in fifo and clear receive
    buf information. Else count SN and add receive data len.*/
    if (TRUE == IsReciveCFAll(m_stMsgInfo->msgLen - 1u)) {
        /*copy all data in fifo and receive over. */
        fsl_memcpy(&gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen - gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen);

        /*copy all data in FIFO*/
        (void)CANTP_CopyAFrameDataInRxFifo(gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId,
                                           gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen,
                                           gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf);

        *m_peNextStatus = IDLE;

    } else {
        /*If is block size overflow.*/
        if (TRUE == IsRxBlockSizeOverflow()) {
            RXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBr);

            *m_peNextStatus = TX_FC;
        } else {
            /*count Sn and set STmin, wait timeout time*/
            AddWaitSN();

            /*set wait frame time*/
            RXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCr);
        }

        /*Copy data in global fifo*/
        fsl_memcpy(&gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   m_stMsgInfo->msgLen - 1u);

        AddRevDataLen(m_stMsgInfo->msgLen - 1u);
    }

    return N_OK;
}

/*transmit FC callback*/
static void CANTP_DoTransmitFCCallBack(void)
{

    if (gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen > MAX_CF_DATA_LEN) {
        SetCurCANTPSatus(IDLE);
    } else {
        /*set wait STmin*/
        RXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCr);
        SetCurCANTPSatus(RX_CF);
    }
}

/*Transmit flow control frame*/
static tN_Result CANTP_DoTransmitFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aucTransDataBuf[DATA_LEN] = {0u};

    /*Is wait FC timeout?*/
    if (TRUE != IsWaitFCTimeout()) {
        TPDebugPrintf("\n waitting transmitt FC not timeout!\n");

        /*waitting timeout for transmit FC*/
        return N_OK;

    }

    /*set frame type*/
    (void)CANTP_SetFrameType(FC, &aucTransDataBuf[0u]);

    /*Check current buf. */
    if (gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen > MAX_CF_DATA_LEN) {
        /*set FS*/
        SetFS(&aucTransDataBuf[1u], OVERFLOW_BUF);
    } else {
        SetFS(&aucTransDataBuf[1u], CONTINUE_TO_SEND);
    }

    /*set BS*/
    SetBlockSize(&aucTransDataBuf[1u], g_stCANUdsNetLayerCfgInfo.xBlockSize);

    /*add block size*/
    AddBlockSize();

    /*add wait SN*/
    AddWaitSN();

    /*set STmin*/
    SetSTmin(&aucTransDataBuf[2u], g_stCANUdsNetLayerCfgInfo.xSTmin);

    /*set wait next frame  max time*/
    RXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAr);

    /*CAN TP set tx message status and register tx message successful callback.*/
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITTING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitFCCallBack);

    /*transmit flow control*/
    if (TRUE == g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(g_stCANUdsNetLayerCfgInfo.xTxId,
                                                     sizeof(aucTransDataBuf),
                                                     aucTransDataBuf,
                                                     CANTP_TxMsgSuccessfulCallBack,
                                                     g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs)) {
        *m_peNextStatus = WAITTING_TX;

        return N_OK;
    }

    /*CAN TP set tx message status and register tx message successful callback.*/
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
    CANTP_RegisterTxMsgCallBack(NULL_PTR);

    /*transmit message failed and do idle*/
    *m_peNextStatus = IDLE;

    return N_ERROR;
}

/*transmit SF callback*/
static void CANTP_DoTransmitSFCallBack(void)
{
    TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);

    SetCurCANTPSatus(IDLE);
}

/*transmit signle frame*/
static tN_Result CANTP_DoTransmitSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aDataBuf[DATA_LEN] = {0u};
    uint8 txLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*Check transmit data len. If data len overflow Max SF, return FALSE.*/
    if (TRUE == IsTxDataLenOverflowSF()) {
        *m_peNextStatus = TX_FF;

        return N_ERROR;
    }

    if (TRUE == IsTxDataLenLessSF()) {
        *m_peNextStatus = IDLE;

        return N_ERROR;
    }

    /*set transmitted frame type*/
    (void)CANTP_SetFrameType(SF, &aDataBuf[0u]);

    /*set transmitted data len*/
    SetTxSFDataLen(&aDataBuf[0u], gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen);
    txLen = aDataBuf[0u] + 1u;

    /*copy data in tx buf*/
    fsl_memcpy(&aDataBuf[1u],
               gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf,
               gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen);

    /*CAN TP set tx message status and register tx message successful callback.*/
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITTING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitSFCallBack);

    /*request transmitted application message.*/
    if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                     txLen,
                                                     aDataBuf,
                                                     CANTP_TxMsgSuccessfulCallBack,
                                                     g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs)) {
        /*CAN TP set tx message status and register tx message successful callback.*/
        CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
        CANTP_RegisterTxMsgCallBack(NULL_PTR);

        /*send message error*/
        *m_peNextStatus = IDLE;

        /*request transmitted application message failed.*/
        return N_ERROR;
    }

    /*set wait send frame successful max time*/
    TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAs);

    /*jump to idle and clear transmitted message.*/
    *m_peNextStatus = WAITTING_TX;

    return N_OK;
}

/*transmit FF callback*/
static void CANTP_DoTransmitFFCallBack(void)
{
    /*add tx data len*/
    AddTxDataLen(FF_DATA_MIN_LEN - 2);

    /*set Tx wait time*/
    TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBs);

    /*jump to idle and clear transmitted message.*/
    AddTxSN();

    SetCurCANTPSatus(RX_FC);
}


/*transmitt first frame*/
static tN_Result CANTP_DoTransmitFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aDataBuf[DATA_LEN] = {0u};

    ASSERT(NULL_PTR == m_peNextStatus);

    /*Check transmit data len. If data len overflow less than SF, return FALSE.*/
    if (TRUE != IsTxDataLenOverflowSF()) {
        *m_peNextStatus = TX_SF;

        return N_BUFFER_OVFLW;
    }

    /*set transmitted frame type*/
    (void)CANTP_SetFrameType(FF, &aDataBuf[0u]);

    /*set transmitted data len*/
    SetTxFFDataLen(aDataBuf, gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen);

    /*CAN TP set tx message status and register tx message successful callback.*/
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITTING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitFFCallBack);

    /*copy data in tx buf*/
    fsl_memcpy(&aDataBuf[2u], gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf, FF_DATA_MIN_LEN - 2);

    /*request transmitted application message.*/
    if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                     sizeof(aDataBuf),
                                                     aDataBuf,
                                                     CANTP_TxMsgSuccessfulCallBack,
                                                     g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs)) {
        /*CAN TP set tx message status and register tx message successful callback.*/
        CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
        CANTP_RegisterTxMsgCallBack(NULL_PTR);

        /*send message error*/
        *m_peNextStatus = IDLE;

        /*request transmitted application message failed.*/
        return N_ERROR;
    }

    /*set wait send frame successful max time*/
    TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAs);

    /*jump to idle and clear transmitted message.*/
    *m_peNextStatus = WAITTING_TX;

    return N_OK;
}

/*wait flow control frame*/
static tN_Result CANTP_DoReceiveFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    tFlowStatus eFlowStatus;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*If tx message wait FC timeout jump to IDLE.*/
    if (TRUE == IsTxWaitFrameTimeout()) {
        TPDebugPrintf("Wait flow control timeout.\n");

        *m_peNextStatus = IDLE;

        return N_TIMEOUT_Cr;
    }

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree)) {
        /*waitting received FC. It's normally for waitting CAN message and return OK.*/
        return N_OK;
    }

    if (TRUE != IsFC(m_stMsgInfo->aMsgBuf[0u])) {
        return N_ERROR;
    }

    /*Get flow status*/
    GetFS(m_stMsgInfo->aMsgBuf[0u], &eFlowStatus);

    if (OVERFLOW_BUF == eFlowStatus) {
        *m_peNextStatus = IDLE;

        return N_BUFFER_OVFLW;
    }

    /*wait flow control*/
    if (WAIT_FC == eFlowStatus) {
        /*set Tx wait time*/
        TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBs);

        return N_OK;
    }

    /*contiune to send */
    if (CONTINUE_TO_SEND == eFlowStatus) {
        SetBlockSize(&gs_stCanTPTxDataInfo.ucBlockSize, m_stMsgInfo->aMsgBuf[1u]);

        SaveTxSTmin(m_stMsgInfo->aMsgBuf[2u]);

        TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCs);

        /*remove Add Tx SN, because this SN is added in send First frame callback*/
#if 0
        AddTxSN();
#endif
    } else {
        /*received erro Flow control*/

        *m_peNextStatus = IDLE;

        return N_INVALID_FS;
    }

    *m_peNextStatus = TX_CF;

    return N_OK;
}

/*transmit CF callback*/
static void CANTP_DoTransmitCFCallBack(void)
{
    if (TRUE == IsTxAll()) {
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);

        SetCurCANTPSatus(IDLE);

        return;
    }

    /*set transmitted next frame min time.*/
    SetTxSTmin();

    if (gs_stCanTPTxDataInfo.ucBlockSize) {
        gs_stCanTPTxDataInfo.ucBlockSize--;

        /*block size is equal 0,  waitting  flow control message. if not equal 0, continual send CF message.*/
        if (0u == gs_stCanTPTxDataInfo.ucBlockSize) {
            SetCurCANTPSatus(RX_FC);

            TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBs);

            return;
        }
    }

    AddTxSN();

    /*set tx next frame max time.*/
    TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCs);

    SetCurCANTPSatus(TX_CF);
}


/*transmit conective frame*/
static tN_Result CANTP_DoTransmitCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aTxDataBuf[DATA_LEN] = {0u};
    uint8 TxLen = 0u;
    uint8 aTxAllLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*Is Tx STmin timeout?*/
    if (FALSE == IsTxSTminTimeout()) {
        /*watting STmin timeout.  It's normally in the step.*/
        return N_OK;
    }

    /*Is transmitted timeout?*/
    if (TRUE == IsTxWaitFrameTimeout()) {
        *m_peNextStatus = IDLE;

        return N_TIMEOUT_Bs;
    }

    (void)CANTP_SetFrameType(CF, &aTxDataBuf[0u]);

    SetTxSN(&aTxDataBuf[0u]);

    TxLen = gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen - gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen;

    /*CAN TP set tx message status and register tx message successful callback.*/
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITTING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitCFCallBack);

    if (TxLen >= CF_DATA_MAX_LEN) {
        fsl_memcpy(&aTxDataBuf[1u],
                   &gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen],
                   CF_DATA_MAX_LEN);

        /*request transmitted application message.*/
        if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                         sizeof(aTxDataBuf),
                                                         aTxDataBuf,
                                                         CANTP_TxMsgSuccessfulCallBack,
                                                         g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs)) {
            /*CAN TP set tx message status and register tx message successful callback.*/
            CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
            CANTP_RegisterTxMsgCallBack(NULL_PTR);

            /*send message error*/
            *m_peNextStatus = IDLE;

            /*request transmitted application message failed.*/
            return N_ERROR;
        }

        AddTxDataLen(CF_DATA_MAX_LEN);
    }

    else {
        fsl_memcpy(&aTxDataBuf[1u],
                   &gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen],
                   TxLen);

        aTxAllLen = TxLen + 1u;

        /*request transmitted application message.*/
        if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                         aTxAllLen,
                                                         aTxDataBuf,
                                                         CANTP_TxMsgSuccessfulCallBack,
                                                         g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs)) {
            /*CAN TP set tx message status and register tx message successful callback.*/
            CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
            CANTP_RegisterTxMsgCallBack(NULL_PTR);

            /*send message error*/
            *m_peNextStatus = IDLE;

            /*request transmitted application message failed.*/
            return N_ERROR;
        }

        AddTxDataLen(TxLen);
    }

    /*set wait send frame successful max time*/
    TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAs);

    *m_peNextStatus = WAITTING_TX;

    return N_OK;
}

/*waitting tx message*/
static tN_Result CANTP_DoWaittingTxMsg(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    /*check is waitting timeout?*/
    if (TRUE == IsTxMsgWaittingFrameTimeout()) {
        /*abort CAN bus send message*/
        if (NULL_PTR != g_stCANUdsNetLayerCfgInfo.pfAbortTXMsg) {
            (g_stCANUdsNetLayerCfgInfo.pfAbortTXMsg) ();
        }

        /*tell up layer, tx message timeout*/
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_TIMEOUT);

        /*CAN TP set tx message status and register tx message successful callback.*/
        CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
        CANTP_RegisterTxMsgCallBack(NULL_PTR);

        *m_peNextStatus = IDLE;
    }

    return N_OK;
}

/*set transmit frame type*/
static uint8 CANTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8 *o_pucFrameType)
{
    ASSERT(NULL_PTR == o_pucFrameType);

    if (SF == i_eFrameType ||
            FF == i_eFrameType ||
            FC == i_eFrameType ||
            CF == i_eFrameType) {
        *o_pucFrameType &= 0x0Fu;

        *o_pucFrameType |= ((uint8)i_eFrameType << 4u);

        return TRUE;
    }

    return FALSE;
}

/*CAN TP TX message callback*/
static void CANTP_TxMsgSuccessfulCallBack(void)
{
    gs_eCANTPTxMsStatus = CANTP_TX_MSG_SUCC;
}

/*CANP TP set TX message status*/
static void CANTP_SetTxMsgStatus(const tCanTPTxMsgStatus i_eTxMsgStatus)
{
    gs_eCANTPTxMsStatus = i_eTxMsgStatus;
}

/*Register tx message successful callback*/
static void CANTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack)
{
    gs_pfCANTPTxMsgCallBack = i_pfNetTxCallBack;
}

/*Do register tx message callback*/
static void CANTP_DoRegisterTxMsgCallBack(void)
{
    tCanTPTxMsgStatus CANTPTxMsgStatus = CANTP_TX_MSG_IDLE;

    /*get the tx message status with disable interrupt for protect the variable not changeb by interrupt.*/
    DisableAllInterrupts();
    CANTPTxMsgStatus = gs_eCANTPTxMsStatus;
    EnableAllInterrupts();

    if (CANTP_TX_MSG_SUCC == CANTPTxMsgStatus) {
        if (NULL_PTR != gs_pfCANTPTxMsgCallBack) {
            (gs_pfCANTPTxMsgCallBack)();

            gs_pfCANTPTxMsgCallBack = NULL_PTR;
        }
    } else if (CANTP_TX_MSG_FAIL == CANTPTxMsgStatus) {
        TPDebugPrintf("\n TX msg failed callback=%X, status=%d\n", gs_pfCANTPTxMsgCallBack, gs_eCANTPTxMsStatus);
        gs_eCANTPTxMsStatus = CANTP_TX_MSG_IDLE;
        /*if tx message failled, clear tx message callback*/
        gs_pfCANTPTxMsgCallBack = NULL_PTR;
    } else {
        /*do nothing*/
    }
}


/*get RX SF frame message length*/
static boolean GetRXSFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen)
{
    boolean result = FALSE;
    uint32 frameLen = 0u;

    ASSERT(NULL_PTR == i_pMsgBuf);
    ASSERT(NULL_PTR == o_pFrameLen);

    if ((i_RxMsgLen <= 1u) || (TRUE != IsSF(i_pMsgBuf[0u]))) {
        return FALSE;
    }

    /*Check received single message length based on ISO15765-2 2016*/
    if (i_RxMsgLen <= 8u) {
        frameLen = i_pMsgBuf[0u] & 0x0Fu;

        if ((frameLen <= SF_CAN_DATA_MAX_LEN) && (frameLen > 0u)) {
            result = IsRxMsgLenValid(NORMAL_ADDRESSING, frameLen, i_RxMsgLen);
        }
    } else {
        frameLen = i_pMsgBuf[0u] & 0x0Fu;

        if (0u == frameLen) {
            frameLen = i_pMsgBuf[1u];

            if ((frameLen <= SF_CANFD_DATA_MAX_LEN) && (frameLen > 0u)) {
                result = IsRxMsgLenValid(NORMAL_ADDRESSING, frameLen, i_RxMsgLen);
            }
        }
    }

    if (TRUE == result) {
        *o_pFrameLen = frameLen;
    }

    return result;
}

/*get RX FF frame message length*/
static boolean GetRXFFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen)
{
    boolean result = FALSE;
    uint32 frameLen = 0u;
    uint8 index = 0u;

    ASSERT(NULL_PTR == i_pMsgBuf);
    ASSERT(NULL_PTR == o_pFrameLen);

    if ((i_RxMsgLen < 8u) || (TRUE != IsFF(i_pMsgBuf[0u]))) {
        return FALSE;
    }

    /*Check received single message length based on ISO15765-2 2016*/
    /*calculate FF message length*/
    frameLen = (uint32)((i_pMsgBuf[0u] & 0x0Fu) << 8u) | i_pMsgBuf[1u];

    if (0u == frameLen) {
        /*FF message length is over 4095 Bytes*/
        for (index = 0u; index < 4; index++) {
            frameLen <<= 8u;
            frameLen |= i_pMsgBuf[index + 2u];
        }
    }

    if (frameLen < FF_DATA_MIN_LEN) {
        result = FALSE;
    } else {
        result = TRUE;
    }

    if (TRUE == result) {
        *o_pFrameLen = frameLen;
    }

    return result;
}

#endif /*#ifdef EN_CAN_TP*/

/* -------------------------------------------- END OF FILE -------------------------------------------- */
