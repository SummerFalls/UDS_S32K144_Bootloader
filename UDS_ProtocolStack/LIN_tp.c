/*
 * @ 名称: LIN_tp.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "LIN_tp.h"

#ifdef EN_LIN_TP
#include "TP_cfg.h"

/*********************************************************
**  SF  --  signle frame
**  FF  --  first frame
**  FC  --  flow control
**  CF  --  consective frame
*********************************************************/

typedef enum {
    LINTP_IDLE,      /*LINTP_IDLE*/
    LINTP_RX_SF,   /*wait signle frame*/
    LINTP_RX_FF,   /*wait first frame*/
    LINTP_RX_CF,   /*wait consective frame*/

    LINTP_TX_SF,     /*tx signle frame*/
    LINTP_TX_FF,     /*tx first frame*/
    LINTP_TX_CF,     /*tx consective frame*/

    LINTP_WAITTING_TX, /*watting tx message*/

    WAIT_CONFIRM /*wait confrim*/
} tLINTpWorkStatus;

typedef enum {
    SF,        /*signle frame value*/
    FF,        /*first frame value*/
    CF        /*consective frame value*/
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
    LINTP_TX_MSG_IDLE = 0, /*LIN TP tx message idle*/
    LINTP_TX_MSG_SUCC,     /*LIN TP tx message successful*/
    LINTP_TX_MSG_FAIL,     /*LIN TP tx message fail*/
    LINTP_TX_MSG_WAITTING /*LIN TP waitting tx message*/
} tLINTPTxMsgStatus;


typedef struct {
    tUdsId xLINTpId;                           /*can tp message id*/
    tLINTpDataLen xPduDataLen;                 /*pdu data len(Rx/Tx data len)*/
    tLINTpDataLen xFFDataLen;                  /*Rx/Tx FF data len*/
    uint8 aDataBuf[MAX_CF_DATA_LEN]; /*Rx/Tx data buf*/
} tLINTpDataInfo;

typedef struct {
    uint8 ucSN;          /*SN*/
    uint8 ucBlockSize;   /*Block size*/
    tNetTime xSTmin;             /*STmin*/
    tNetTime xMaxWatiTimeout;    /*timeout time*/
    tLINTpDataInfo stLINTpDataInfo;
} tLINTpInfo;

typedef struct {
    uint8 isFree;            /*rx message status. TRUE = not received messag.*/
    tUdsId xMsgId;                     /*received message id*/
    uint8 msgLen;            /*received message len*/
    uint8 aMsgBuf[DATA_LEN]; /*message data buf*/
} tLINTpMsg;

typedef tN_Result (*tpfLINTpFun)(tLINTpMsg *, tLINTpWorkStatus *);
typedef struct {
    tLINTpWorkStatus eLINTpStaus;
    tpfLINTpFun pfLINTpFun;
} tLINTpFunInfo;

/***********************Global value*************************/
static tLINTpInfo gs_stLINTPTxDataInfo; /*can tp tx data*/
static tNetTime gs_xLINTPTxSTmin = 0u; /*tx STmin*/
static tLINTpInfo gs_stLINTPRxDataInfo; /*can tp rx data*/
static uint32 gs_LINTPTxMsgMaxWaitTime = 0u;/*tx message max wait time, RX / TX frame both used waitting status*/
static tLINTpWorkStatus gs_eLINTpWorkStatus = LINTP_IDLE;
static tLINTPTxMsgStatus gs_eLINTPTxMsStatus = LINTP_TX_MSG_IDLE;
static tpfNetTxCallBack gs_pfLINTPTxMsgCallBack = NULL_PTR;
/*********************************************************/

/***********************Static function***********************/
#define LINTpTimeToCount(xTime) ((xTime) / g_stUdsLINNetLayerCfgInfo.ucCalledPeriod)
#define IsSF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) ==  SF) ? TRUE : FALSE)
#define IsFF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) ==  FF) ? TRUE : FALSE)
#define IsCF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) ==  CF) ? TRUE : FALSE)
#define IsFC(xNetWorkFrameType) ((((xNetWorkFrameType)>> 4u) ==  FC) ? TRUE : FALSE)
#define IsRevSNValid(xSN) ((gs_stLINTPRxDataInfo.ucSN == ((xSN) & 0x0Fu)) ? TRUE : FALSE)
#define AddWaitSN()\
    do{\
        gs_stLINTPRxDataInfo.ucSN++;\
        if(gs_stLINTPRxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stLINTPRxDataInfo.ucSN = 0u;\
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

/*save FF data len*/
#define SaveFFDataLen(i_xRevFFDataLen) (gs_stLINTPRxDataInfo.stLINTpDataInfo.xFFDataLen = i_xRevFFDataLen)

/*set BS*/
#define SetBlockSize(pucBSBuf, xBlockSize) (*(pucBSBuf) = (uint8)(xBlockSize))

/*add block size*/
#define AddBlockSize()\
    do{\
        if(0u != g_stUdsLINNetLayerCfgInfo.xBlockSize)\
        {\
            gs_stLINTPRxDataInfo.ucBlockSize++;\
        }\
    }while(0u)

/*set STmin*/
#define SetSTmin(pucSTminBuf, xSTmin) (*(pucSTminBuf) = (uint8)(xSTmin))

/*set wait  STmin*/
#define SetWaitSTmin() (gs_stLINTPRxDataInfo.xSTmin = LINTpTimeToCount(g_stUdsLINNetLayerCfgInfo.xSTmin))

/*set wait frame time*/
#define SetRxWaitFrameTime(xWaitTimeout) do{\
        (gs_stLINTPRxDataInfo.xMaxWatiTimeout = LINTpTimeToCount(xWaitTimeout));\
        gs_LINTPTxMsgMaxWaitTime = gs_stLINTPRxDataInfo.xMaxWatiTimeout;\
    }while(0u);

/*RX frame set Rx msg wait time*/
#define RXFrame_SetRxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/*RX frame set Tx msg wait time*/
#define RXFrame_SetTxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)


/*set wait SN*/
#define SetWaitSN(xSN) (gs_stLINTPRxDataInfo.ucSN = xSN)

/*set FS*/
#define SetFS(pucFsBuf, xFlowStatus) (*(pucFsBuf) = (*(pucFsBuf) & 0xF0u) | (uint8)(xFlowStatus))

/*clear receive data buf*/
#define ClearRevDataBuf()\
    do{\
        fsl_memset(&gs_stLINTPRxDataInfo,0u,sizeof(gs_stLINTPRxDataInfo));\
    }while(0u)

/*add rev data len*/
#define AddRevDataLen(xRevDataLen) (gs_stLINTPRxDataInfo.stLINTpDataInfo.xPduDataLen += (xRevDataLen))

/*Is rev conective frame all.*/
#define IsReciveCFAll(xCFDataLen) (((gs_stLINTPRxDataInfo.stLINTpDataInfo.xPduDataLen + (uint8)(xCFDataLen))\
                                    >= gs_stLINTPRxDataInfo.stLINTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/*Is STmin timeout?*/
#define IsSTminTimeOut() ((0u == gs_stLINTPRxDataInfo.xSTmin) ? TRUE : FALSE)

/*Is wait Flow control timeout?*/
#define IsWaitFCTimeout()  ((0u == gs_stLINTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*Is wait conective frame timeout?*/
#define IsWaitCFTimeout() ((0u == gs_stLINTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*Is block sizeo overflow*/
#define IsRxBlockSizeOverflow() (((0u != g_stUdsLINNetLayerCfgInfo.xBlockSize) &&\
                                  (gs_stLINTPRxDataInfo.ucBlockSize >= g_stUdsLINNetLayerCfgInfo.xBlockSize))\
                                 ? TRUE : FALSE)

/*Is transmitted data len overflow max SF?*/
#define IsTxDataLenOverflowSF() ((gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen > SF_DATA_MAX_LEN) ? TRUE : FALSE)

/*Is transmitted data less than min?*/
#define IsTxDataLenLessSF() ((0u == gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen) ? TRUE : FALSE)

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
#define AddTxDataLen(xTxDataLen) (gs_stLINTPTxDataInfo.stLINTpDataInfo.xPduDataLen += (xTxDataLen))

/*set tx STmin */
#define SetTxSTmin() (gs_stLINTPTxDataInfo.xSTmin = LINTpTimeToCount(gs_xLINTPTxSTmin))

/*save Tx STmin*/
#define SaveTxSTmin(xTxSTmin) (gs_xLINTPTxSTmin = xTxSTmin)

/*Is Tx STmin timeout?*/
#define IsTxSTminTimeout() ((0u == gs_stLINTPTxDataInfo.xSTmin) ? TRUE : FALSE)

/*Set tx wait frame time*/
#define SetTxWaitFrameTime(xWaitTime) do{\
        (gs_stLINTPTxDataInfo.xMaxWatiTimeout = LINTpTimeToCount(xWaitTime));\
        gs_LINTPTxMsgMaxWaitTime = gs_stLINTPTxDataInfo.xMaxWatiTimeout;\
    }while(0u);

/*Is Tx wait frame timeout?*/
#define IsTxWaitFrameTimeout() ((0u == gs_stLINTPTxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*TX frame set tx message wait time*/
#define TXFrame_SetTxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/*TX frame set Tx message wait time*/
#define TXFrame_SetRxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/*Is Tx wait frame timeout?*/
#define IsTxWaitFrameTimeout() ((0u == gs_stLINTPTxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/*check timer in watting status*/
#define IsTxMsgWaittingFrameTimeout() ((0u == gs_LINTPTxMsgMaxWaitTime) ? TRUE : FALSE)

/*Get FS*/
#define GetFS(ucFlowStaus, pxFlowStatusBuf) (*(pxFlowStatusBuf) = (ucFlowStaus) & 0x0Fu)

/*set tx SN*/
#define SetTxSN(pucSNBuf) (*(pucSNBuf) = gs_stLINTPTxDataInfo.ucSN | (*(pucSNBuf) & 0xF0u))

/*Add Tx SN*/
#define AddTxSN()\
    do{\
        gs_stLINTPTxDataInfo.ucSN++;\
        if(gs_stLINTPTxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stLINTPTxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

/*Is Tx all*/
#define IsTxAll() ((gs_stLINTPTxDataInfo.stLINTpDataInfo.xPduDataLen >= \
                    gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/*save received message ID*/
#define SaveRxMsgId(xMsgId) (gs_stLINTPRxDataInfo.stLINTpDataInfo.xLINTpId = (xMsgId))

/*clear can tp Rx msg buf*/
#define ClearLINTpRxMsgBuf(pMsgInfo)\
    do{\
        (pMsgInfo)->isFree = TRUE;\
        (pMsgInfo)->msgLen = 0u;\
        (pMsgInfo)->xMsgId = 0u;\
    }while(0u)

/*get cur CAN TP status*/
#define GetCurLINTpStatus() (gs_eLINTpWorkStatus)

/*set cur CAN TP status*/
#define SetCurLINTpSatus(status) \
    do{\
        gs_eLINTpWorkStatus = status;\
    }while(0u)

/*get cur CAN TP status PTR*/
#define GetCurLINTpStatusPtr() (&gs_eLINTpWorkStatus)

/*can tp LINTP_IDLE*/
static tN_Result LINTP_DoLINTPIdle(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*do receive signle frame*/
static tN_Result LINTP_DoReceiveSF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*do receive first frame*/
static tN_Result LINTP_DoReceiveFF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*do receive conective frame*/
static tN_Result LINTP_DoReceiveCF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*transmit signle frame*/
static tN_Result LINTP_DoTransmitSF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*transmit SF callback*/
static void LINTP_DoTransmitSFCallBack(void);

/*transmit FF callback*/
static void LINTP_DoTransmitFFCallBack(void);

/*transmitt first frame*/
static tN_Result LINTP_DoTransmitFF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*transmit CF callback*/
static void LINTP_DoTransmitCFCallBack(void);

/*transmit conective frame*/
static tN_Result LINTP_DoTransmitCF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);

/*waitting tx message*/
static tN_Result LINTP_DoWaittingTxMsg(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus);


/*set transmit frame type*/
static uint8 LINTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8 *o_pucFrameType);

/*received a can tp frame, copy these data in fifo.*/
static uint8 LINTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxCanID,
                                          const tLen i_xRxDataLen,
                                          const uint8 *i_pucDataBuf
                                         );

/*uds transmitted a application frame data, copy these data in TX fifo.*/
static uint8 LINTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxCanID,
                                           uint8 *o_pucTxDataLen,
                                           uint8 *o_pucDataBuf);

/*LIN TP TX message callback*/
static void LINTP_TxMsgSuccessfulCallBack(void);

/*LIN TP set TX message status*/
static void LINTP_SetTxMsgStatus(const tLINTPTxMsgStatus i_eTxMsgStatus);

/*Register tx message successful callback*/
static void LINTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack);

/*Do register tx message callback*/
static void LINTP_DoRegisterTxMsgCallBack(void);

/*********************************************************/

static const tLINTpFunInfo gs_astLINTpFunInfo[] = {
    {LINTP_IDLE, LINTP_DoLINTPIdle},
    {LINTP_RX_SF, LINTP_DoReceiveSF},
    {LINTP_RX_FF, LINTP_DoReceiveFF},
    {LINTP_RX_CF, LINTP_DoReceiveCF},

    {LINTP_TX_SF, LINTP_DoTransmitSF},
    {LINTP_TX_FF, LINTP_DoTransmitFF},
    {LINTP_TX_CF, LINTP_DoTransmitCF},
    {LINTP_WAITTING_TX, LINTP_DoWaittingTxMsg}
};

void LINTP_Init(void)
{
    tErroCode eStatus;

    ApplyFifo(RX_TP_QUEUE_LEN, RX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply fifo RX erro!\n");

        while (1) {

        }
    }

    ApplyFifo(TX_TP_QUEUE_LEN, TX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus) {
#ifdef EN_TP_DEBUG
        TPDebugPrintf("apply fifo TX erro code!\n");
#endif

        while (1) {

        }
    }

    ApplyFifo(RX_BUS_FIFO_LEN, RX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply RX fifo from BUS erro code!\n");

        while (1) {

        }
    }

#ifdef EN_LIN_TP
    ApplyFifo(TX_BUS_FIFO_LEN, TX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus) {
        TPDebugPrintf("apply TX fifo TO BUS erro code!\n");

        while (1) {

        }
    }

#endif
}

/*can tp system tick control. This function should period called by system.*/
void LINTP_SytstemTickControl(void)
{
    if (gs_stLINTPRxDataInfo.xSTmin) {
        gs_stLINTPRxDataInfo.xSTmin--;
    }

    if (gs_stLINTPRxDataInfo.xMaxWatiTimeout) {
        gs_stLINTPRxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_stLINTPTxDataInfo.xSTmin) {
        gs_stLINTPTxDataInfo.xSTmin--;
    }

    if (gs_stLINTPTxDataInfo.xMaxWatiTimeout) {
        gs_stLINTPTxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_LINTPTxMsgMaxWaitTime) {
        gs_LINTPTxMsgMaxWaitTime--;
    }
}

/*uds network man function*/
void LINTP_MainFun(void)
{
    uint8 index = 0u;
    const uint8 findCnt = sizeof(gs_astLINTpFunInfo) / sizeof(gs_astLINTpFunInfo[0u]);
    tLINTpMsg stRxLINTpMsg = {TRUE, 0u, 0u, {0u}};
    tN_Result result = N_OK;

    /*In waitting TX message, cannot read message from FIFO. Because, In waitting message will lost read messages.*/
    if (LINTP_WAITTING_TX != GetCurLINTpStatus()) {
        /*read msg from CAN driver RxFIFO*/
        if (TRUE == g_stUdsLINNetLayerCfgInfo.pfNetRx(&stRxLINTpMsg.xMsgId,
                                                      &stRxLINTpMsg.msgLen,
                                                      stRxLINTpMsg.aMsgBuf)) {
            /*check received message ID*/
            if (TRUE == LINTP_IsReceivedMsgIDValid(stRxLINTpMsg.xMsgId)) {
                stRxLINTpMsg.isFree = FALSE;

            } else {
                TPDebugPrintf("recevied invalid message ID\n");
            }
        }
    }

    while (index < findCnt) {
        if (GetCurLINTpStatus() == gs_astLINTpFunInfo[index].eLINTpStaus) {
            if (NULL_PTR != gs_astLINTpFunInfo[index].pfLINTpFun) {
                result = gs_astLINTpFunInfo[index].pfLINTpFun(&stRxLINTpMsg, GetCurLINTpStatusPtr());
            }
        }

        /*if received not equal N_OK, return IDLE status*/
        if (N_UNEXP_PDU != result) {
            if (N_OK != result) {
                SetCurLINTpSatus(LINTP_IDLE);
            }

            index++;
        } else {
            /*if received unexpect PDU, then jump to LINTP_IDLE and restart do process.*/
            index = 0u;
        }
    }

    ClearLINTpRxMsgBuf(&stRxLINTpMsg);

    /*check register tx message callback*/
    LINTP_DoRegisterTxMsgCallBack();
}

/*received a can tp frame, copy these data in UDS RX fifo.*/
static uint8 LINTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxCanID,
                                          const tLen i_xRxDataLen,
                                          const uint8 *i_pucDataBuf)
{
    tErroCode eStatus;
    tLen xCanWriteLen = 0u;

    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    ASSERT(NULL_PTR == i_pucDataBuf);

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
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8 *)i_pucDataBuf, i_xRxDataLen, &eStatus);

    if (ERRO_NONE != eStatus) {
        return FALSE;
    }

    return TRUE;
}

/*uds transmitted a application frame data, copy these data in TX fifo.*/
static uint8 LINTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxCanID,
                                           uint8 *o_pucTxDataLen,
                                           uint8 *o_pucDataBuf)
{
    tErroCode eStatus;
    tLen xRealReadLen = 0u;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    ASSERT(NULL_PTR == o_pxTxCanID);
    ASSERT(NULL_PTR == o_pucTxDataLen);
    ASSERT(NULL_PTR == o_pucDataBuf);

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
                     o_pucDataBuf,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || exchangeMsgInfo.dataLen != xRealReadLen) {
        return FALSE;
    }

    *o_pxTxCanID = exchangeMsgInfo.msgID;

    *o_pucTxDataLen = exchangeMsgInfo.dataLen;

    TP_RegisterTransmittedAFrmaeMsgCallBack(exchangeMsgInfo.pfCallBack);

    return TRUE;
}

/*can tp LINTP_IDLE*/
static tN_Result LINTP_DoLINTPIdle(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    uint8 TxDataLen = (uint8)gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*clear can tp data*/
    fsl_memset((void *)&gs_stLINTPRxDataInfo, 0u, sizeof(tLINTpInfo));
    fsl_memset((void *)&gs_stLINTPTxDataInfo, 0u, sizeof(tLINTpInfo));

    /*clear waitting time*/
    gs_LINTPTxMsgMaxWaitTime = 0u;

    /*set NULL to transmitted message callback*/
    TP_RegisterTransmittedAFrmaeMsgCallBack(NULL_PTR);

    /*If receive can tp message, judge type. Only received SF or FF message.
    Others frame ignore.*/
    if (FALSE == m_stMsgInfo->isFree) {
        if (TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u])) {
            *m_peNextStatus = LINTP_RX_SF;
        }

        if (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u])) {
            *m_peNextStatus = LINTP_RX_FF;
        }
    } else {
        /*Judge have message can will tx.*/
        if (TRUE == LINTP_CopyAFrameFromFifoToBuf(&gs_stLINTPTxDataInfo.stLINTpDataInfo.xLINTpId,
                                                  &TxDataLen,
                                                  gs_stLINTPTxDataInfo.stLINTpDataInfo.aDataBuf)) {
            gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen = TxDataLen;

            if (TRUE == IsTxDataLenOverflowSF()) {
                *m_peNextStatus = LINTP_TX_FF;
            } else {
                *m_peNextStatus = LINTP_TX_SF;
            }
        }
    }

    return N_OK;
}

/*do receive signle frame*/
static tN_Result LINTP_DoReceiveSF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    uint8 SFLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree)) {
        return N_ERROR;
    }

    if (TRUE != IsSF(m_stMsgInfo->aMsgBuf[0u])) {
        return N_ERROR;
    }

    GetFrameLen(m_stMsgInfo->aMsgBuf, &SFLen);

    if (SFLen > SF_DATA_MAX_LEN) {
        return N_ERROR;
    }

    /*write data to UDS fifo*/
    if (FALSE == LINTP_CopyAFrameDataInRxFifo(m_stMsgInfo->xMsgId,
                                              SFLen,
                                              &m_stMsgInfo->aMsgBuf[1u])) {
        TPDebugPrintf("copy data erro!\n");

        *m_peNextStatus = LINTP_IDLE;

        return N_ERROR;
    }

    *m_peNextStatus = LINTP_IDLE;

    return N_OK;
}

/*do receive first frame*/
static tN_Result LINTP_DoReceiveFF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    uint16 FFDataLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree)) {
        return N_ERROR;
    }

    if (TRUE != IsFF(m_stMsgInfo->aMsgBuf[0u])) {
        TPDebugPrintf("Received not FF\n");

        return N_ERROR;
    }

    /*get FF Data len*/
    GetFrameLen(m_stMsgInfo->aMsgBuf, &FFDataLen);

    if (FFDataLen < FF_DATA_MIN_LEN) {
        TPDebugPrintf("Received not FF data len less than min.\n");

#ifdef EN_TP_DEBUG
        TPDebugPrintf("Received FF data len = %d\n", FFDataLen);
#endif

        return N_ERROR;
    }

    /*save received msg ID*/
    SaveRxMsgId(m_stMsgInfo->xMsgId);

    /*write data in global buf. When receive all data, write these data in fifo.*/
    SaveFFDataLen(FFDataLen);

    /*set wait conective frame*/
    RXFrame_SetRxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNCr);

    /*copy data in golbal buf*/
    fsl_memcpy(gs_stLINTPRxDataInfo.stLINTpDataInfo.aDataBuf, (const void *)&m_stMsgInfo->aMsgBuf[2u], m_stMsgInfo->msgLen - 2u);

    AddRevDataLen(m_stMsgInfo->msgLen - 2u);

    /*count Sn and set STmin, wait timeout time*/
    AddWaitSN();

    /*jump to next status*/
    *m_peNextStatus = LINTP_RX_CF;

    ClearLINTpRxMsgBuf(m_stMsgInfo);

    return N_OK;
}


/*do receive conective frame*/
static tN_Result LINTP_DoReceiveCF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    ASSERT(NULL_PTR == m_peNextStatus);

    /*Is timeout rx wait timeout? If wait time out receive CF over.*/
    if (TRUE == IsWaitCFTimeout()) {
        TPDebugPrintf("wait conective frame timeout!\n");

        *m_peNextStatus = LINTP_IDLE;

        return N_TIMEOUT_Cr;
    }

    if (0u == m_stMsgInfo->msgLen || TRUE == m_stMsgInfo->isFree) {
        /* It's normally return N_OK when waitting received LIN message.*/
        return N_OK;
    }

    /*check received msssage is SF or FF? If received SF or FF, start new receive progrocess.*/
    if ((TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u])) || (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u]))) {
        *m_peNextStatus = LINTP_IDLE;

        return N_UNEXP_PDU;
    }

    if (gs_stLINTPRxDataInfo.stLINTpDataInfo.xLINTpId != m_stMsgInfo->xMsgId) {
#ifdef EN_TP_DEBUG
        TPDebugPrintf("Msg ID invalid in CF! F RX ID = %X, RX ID = %X\n",
                      gs_stLINTPRxDataInfo.stLINTpDataInfo.xLINTpId, m_stMsgInfo->xMsgId);
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
        fsl_memcpy(&gs_stLINTPRxDataInfo.stLINTpDataInfo.aDataBuf[gs_stLINTPRxDataInfo.stLINTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   gs_stLINTPRxDataInfo.stLINTpDataInfo.xFFDataLen - gs_stLINTPRxDataInfo.stLINTpDataInfo.xPduDataLen);

        /*copy all data in FIFO*/
        (void)LINTP_CopyAFrameDataInRxFifo(gs_stLINTPRxDataInfo.stLINTpDataInfo.xLINTpId,
                                           gs_stLINTPRxDataInfo.stLINTpDataInfo.xFFDataLen,
                                           gs_stLINTPRxDataInfo.stLINTpDataInfo.aDataBuf);

        *m_peNextStatus = LINTP_IDLE;

    } else {
        /*count Sn and set STmin, wait timeout time*/
        AddWaitSN();

        /*set wait frame time*/
        RXFrame_SetRxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNCr);

        /*Copy data in global fifo*/
        fsl_memcpy(&gs_stLINTPRxDataInfo.stLINTpDataInfo.aDataBuf[gs_stLINTPRxDataInfo.stLINTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   m_stMsgInfo->msgLen - 1u);

        AddRevDataLen(m_stMsgInfo->msgLen - 1u);
    }

    return N_OK;
}

/*transmit SF callback*/
static void LINTP_DoTransmitSFCallBack(void)
{
    TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);

    SetCurLINTpSatus(LINTP_IDLE);
}

/*transmit signle frame*/
static tN_Result LINTP_DoTransmitSF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    uint8 aDataBuf[DATA_LEN] = {0u};
    uint8 TxLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*Check transmit data len. If data len overflow Max SF, return FALSE.*/
    if (TRUE == IsTxDataLenOverflowSF()) {
        *m_peNextStatus = LINTP_TX_FF;

        return N_ERROR;
    }

    if (TRUE == IsTxDataLenLessSF()) {
        *m_peNextStatus = LINTP_IDLE;

        return N_ERROR;
    }

    /*set transmitted frame type*/
    (void)LINTP_SetFrameType(SF, &aDataBuf[0u]);

    /*set transmitted data len*/
    SetTxSFDataLen(&aDataBuf[0u], gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen);
    TxLen = aDataBuf[0u] + 1u;

    /*copy data in tx buf*/
    fsl_memcpy(&aDataBuf[1u],
               gs_stLINTPTxDataInfo.stLINTpDataInfo.aDataBuf,
               gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen);

    /*set tx message status is waitting status and register callback*/
    LINTP_SetTxMsgStatus(LINTP_TX_MSG_WAITTING);
    LINTP_RegisterTxMsgCallBack(LINTP_DoTransmitSFCallBack);

    /*request transmitted application message.*/
    if (TRUE != g_stUdsLINNetLayerCfgInfo.pfNetTxMsg(gs_stLINTPTxDataInfo.stLINTpDataInfo.xLINTpId,
                                                     TxLen,
                                                     aDataBuf,
                                                     LINTP_TxMsgSuccessfulCallBack,
                                                     g_stUdsLINNetLayerCfgInfo.txBlockingMaxTimeMs)) {
        /*set tx message status is waitting status and register callback*/
        LINTP_SetTxMsgStatus(LINTP_TX_MSG_FAIL);
        LINTP_RegisterTxMsgCallBack(NULL_PTR);

        /*TX messsage error!*/
        *m_peNextStatus = LINTP_IDLE;

        /*request transmitted application message failed.*/
        return N_ERROR;
    }

    /*set tx frame max time.*/
    TXFrame_SetTxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNAs);

    /*jump to LINTP_IDLE and clear transmitted message.*/
    *m_peNextStatus = LINTP_WAITTING_TX;

    return N_OK;
}

/*transmit FF callback*/
static void LINTP_DoTransmitFFCallBack(void)
{
    /*add tx data len*/
    AddTxDataLen(FF_DATA_MIN_LEN - 2);

    /*set Tx wait time*/
    TXFrame_SetTxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNCs);

    AddTxSN();

    SetCurLINTpSatus(LINTP_TX_CF);
}


/*transmitt first frame*/
static tN_Result LINTP_DoTransmitFF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    uint8 aDataBuf[DATA_LEN] = {0u};

    ASSERT(NULL_PTR == m_peNextStatus);

    /*Check transmit data len. If data len overflow less than SF, return FALSE.*/
    if (TRUE != IsTxDataLenOverflowSF()) {
        *m_peNextStatus = LINTP_TX_SF;

        return N_BUFFER_OVFLW;
    }

    /*set transmitted frame type*/
    (void)LINTP_SetFrameType(FF, &aDataBuf[0u]);

    /*set transmitted data len*/
    SetTxFFDataLen(aDataBuf, gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen);

    /*copy data in tx buf*/
    fsl_memcpy(&aDataBuf[2u], gs_stLINTPTxDataInfo.stLINTpDataInfo.aDataBuf, FF_DATA_MIN_LEN - 2);

    /*set tx message status is waitting status and register callback*/
    LINTP_SetTxMsgStatus(LINTP_TX_MSG_WAITTING);
    LINTP_RegisterTxMsgCallBack(LINTP_DoTransmitFFCallBack);

    /*request transmitted application message.*/
    if (TRUE != g_stUdsLINNetLayerCfgInfo.pfNetTxMsg(gs_stLINTPTxDataInfo.stLINTpDataInfo.xLINTpId,
                                                     sizeof(aDataBuf),
                                                     aDataBuf,
                                                     LINTP_TxMsgSuccessfulCallBack,
                                                     g_stUdsLINNetLayerCfgInfo.txBlockingMaxTimeMs)) {
        /*set tx message status is waitting status and register callback*/
        LINTP_SetTxMsgStatus(LINTP_TX_MSG_FAIL);
        LINTP_RegisterTxMsgCallBack(NULL_PTR);

        /*TX messsage error!*/
        *m_peNextStatus = LINTP_IDLE;

        /*request transmitted application message failed.*/
        return N_ERROR;
    }

    /*set tx frame max time.*/
    TXFrame_SetTxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNAs);

    /*jump to LINTP_IDLE and clear transmitted message.*/
    *m_peNextStatus = LINTP_WAITTING_TX;

    return N_OK;
}

/*transmit CF callback*/
static void LINTP_DoTransmitCFCallBack(void)
{
    if (TRUE == IsTxAll()) {
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);

        SetCurLINTpSatus(LINTP_IDLE);

        return;
    }

    /*set transmitted next frame min time.*/
    SetTxSTmin();

    /*set tx next frame max time.*/
    TXFrame_SetTxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNCs);

    AddTxSN();

    SetCurLINTpSatus(LINTP_TX_CF);
}

/*transmit conective frame*/
static tN_Result LINTP_DoTransmitCF(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    uint8 aTxDataBuf[DATA_LEN] = {0u};
    uint8 TxLen = 0u;
    uint8 aTxAllLen = 0u;

    ASSERT(NULL_PTR == m_peNextStatus);

    /*Is Tx STmin timeout?*/
    if (FALSE == IsTxSTminTimeout()) {
        /*It's normally return N_OK, when waitting transmit CF message.*/
        return N_OK;
    }

    /*Is transmitted timeout?*/
    if (TRUE == IsTxWaitFrameTimeout()) {
        *m_peNextStatus = LINTP_IDLE;

        return N_TIMEOUT_Bs;
    }

    (void)LINTP_SetFrameType(CF, &aTxDataBuf[0u]);

    SetTxSN(&aTxDataBuf[0u]);

    TxLen = gs_stLINTPTxDataInfo.stLINTpDataInfo.xFFDataLen - gs_stLINTPTxDataInfo.stLINTpDataInfo.xPduDataLen;

    /*set tx message status is waitting status and register callback*/
    LINTP_SetTxMsgStatus(LINTP_TX_MSG_WAITTING);
    LINTP_RegisterTxMsgCallBack(LINTP_DoTransmitCFCallBack);

    TXFrame_SetTxMsgWaitTime(g_stUdsLINNetLayerCfgInfo.xNAs);

    if (TxLen >= (CF_DATA_MAX_LEN)) {
        fsl_memcpy(&aTxDataBuf[1u],
                   &gs_stLINTPTxDataInfo.stLINTpDataInfo.aDataBuf[gs_stLINTPTxDataInfo.stLINTpDataInfo.xPduDataLen],
                   (CF_DATA_MAX_LEN));

        /*request transmitted application message.*/
        if (TRUE != g_stUdsLINNetLayerCfgInfo.pfNetTxMsg(gs_stLINTPTxDataInfo.stLINTpDataInfo.xLINTpId,
                                                         sizeof(aTxDataBuf),
                                                         aTxDataBuf,
                                                         LINTP_TxMsgSuccessfulCallBack,
                                                         g_stUdsLINNetLayerCfgInfo.txBlockingMaxTimeMs)) {
            /*set tx message status is waitting status and register callback*/
            LINTP_SetTxMsgStatus(LINTP_TX_MSG_FAIL);
            LINTP_RegisterTxMsgCallBack(NULL_PTR);

            /*TX messsage error!*/
            *m_peNextStatus = LINTP_IDLE;

            /*request transmitted application message failed.*/
            return N_ERROR;
        }

        AddTxDataLen((CF_DATA_MAX_LEN));
    }

    else {
        fsl_memcpy(&aTxDataBuf[1u],
                   &gs_stLINTPTxDataInfo.stLINTpDataInfo.aDataBuf[gs_stLINTPTxDataInfo.stLINTpDataInfo.xPduDataLen],
                   TxLen);

        aTxAllLen = TxLen + 1u;

        /*request transmitted application message.*/
        if (TRUE != g_stUdsLINNetLayerCfgInfo.pfNetTxMsg(gs_stLINTPTxDataInfo.stLINTpDataInfo.xLINTpId,
                                                         aTxAllLen,
                                                         aTxDataBuf,
                                                         LINTP_TxMsgSuccessfulCallBack,
                                                         g_stUdsLINNetLayerCfgInfo.txBlockingMaxTimeMs)) {
            /*set tx message status is waitting status and register callback*/
            LINTP_SetTxMsgStatus(LINTP_TX_MSG_FAIL);
            LINTP_RegisterTxMsgCallBack(NULL_PTR);

            /*TX messsage error!*/
            *m_peNextStatus = LINTP_IDLE;

            /*request transmitted application message failed.*/
            return N_ERROR;
        }

        AddTxDataLen(TxLen);
    }

    *m_peNextStatus = LINTP_WAITTING_TX;

    return N_OK;
}

/*waitting tx message*/
static tN_Result LINTP_DoWaittingTxMsg(tLINTpMsg *m_stMsgInfo, tLINTpWorkStatus *m_peNextStatus)
{
    /*check is waitting timeout?*/
    if (TRUE == IsTxMsgWaittingFrameTimeout()) {
        /*abort tx message*/
        if (NULL_PTR != g_stUdsLINNetLayerCfgInfo.pfAbortTXMsg) {
            (g_stUdsLINNetLayerCfgInfo.pfAbortTXMsg)();
        }

        /*tell up layer, tx message timeout*/
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_TIMEOUT);

        /*set tx message status is waitting status and register callback*/
        LINTP_SetTxMsgStatus(LINTP_TX_MSG_FAIL);
        LINTP_RegisterTxMsgCallBack(NULL_PTR);

        *m_peNextStatus = LINTP_IDLE;
    }

    return N_OK;
}

/*set transmit frame type*/
static uint8 LINTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8 *o_pucFrameType)
{
    ASSERT(NULL_PTR == o_pucFrameType);

    if (SF == i_eFrameType ||
            FF == i_eFrameType ||
            CF == i_eFrameType) {
        *o_pucFrameType &= 0x0Fu;

        *o_pucFrameType |= ((uint8)i_eFrameType << 4u);

        return TRUE;
    }

    return FALSE;
}

/*LIN TP TX message callback*/
static void LINTP_TxMsgSuccessfulCallBack(void)
{
    gs_eLINTPTxMsStatus = LINTP_TX_MSG_SUCC;
}

/*LIN TP set TX message status*/
static void LINTP_SetTxMsgStatus(const tLINTPTxMsgStatus i_eTxMsgStatus)
{
    gs_eLINTPTxMsStatus = i_eTxMsgStatus;
}

/*Register tx message successful callback*/
static void LINTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack)
{
    gs_pfLINTPTxMsgCallBack = i_pfNetTxCallBack;
}

/*Do register tx message callback*/
static void LINTP_DoRegisterTxMsgCallBack(void)
{
    tLINTPTxMsgStatus LINTPTxMsgStatus = LINTP_TX_MSG_IDLE;

    /*get the tx message status with disable interrupt for protect the variable not changeb by interrupt.*/
    DisableAllInterrupts();
    LINTPTxMsgStatus = gs_eLINTPTxMsStatus;
    EnableAllInterrupts();

    if (LINTP_TX_MSG_SUCC == LINTPTxMsgStatus) {
        if (NULL_PTR != gs_pfLINTPTxMsgCallBack) {
            (gs_pfLINTPTxMsgCallBack)();

            gs_pfLINTPTxMsgCallBack = NULL_PTR;
        }
    } else if (LINTP_TX_MSG_FAIL == LINTPTxMsgStatus) {
        TPDebugPrintf("\n TX msg failed callback=%X, status=%d\n", gs_pfLINTPTxMsgCallBack, gs_eLINTPTxMsStatus);

        gs_eLINTPTxMsStatus = LINTP_TX_MSG_IDLE;

        /*if tx message failled, clear tx message callback*/
        gs_pfLINTPTxMsgCallBack = NULL_PTR;
    } else {
        /*do nothing*/
    }
}

#endif /*#ifdef EN_LIN_TP*/

/* -------------------------------------------- END OF FILE -------------------------------------------- */
