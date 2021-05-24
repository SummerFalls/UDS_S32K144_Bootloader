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
**  SF - Single Frame
**  FF - First Frame
**  FC - Flow Control
**  CF - Consecutive Frame
*********************************************************/

typedef enum
{
    IDLE,        /* CAN TP IDLE */
    RX_SF,       /* Wait single frame */
    RX_FF,       /* Wait first frame */
    RX_FC,       /* Wait flow control frame */
    RX_CF,       /* Wait consecutive frame */

    TX_SF,       /* TX single frame */
    TX_FF,       /* TX first frame */
    TX_FC,       /* TX flow control */
    TX_CF,       /* TX consecutive frame */

    WAITING_TX,  /* Waiting TX message */

    WAIT_CONFIRM /* Wait confirm */
} tCanTpWorkStatus;

typedef enum
{
    SF, /* Single frame value */
    FF, /* First frame value */
    CF, /* Consecutive frame value */
    FC  /* Flow control value */
} tNetWorkFrameType;

typedef enum
{
    CONTINUE_TO_SEND, /* Continue to send */
    WAIT_FC,          /* Wait flow control */
    OVERFLOW_BUF      /* Overflow buffer */
} tFlowStatus;

typedef enum
{
    N_OK = 0,       /* This value means that the service execution has completed successfully;
                       it can be issued to a service user on both the sender and receiver side */

    N_TIMEOUT_A,    /* This value is issued to the protocol user when the timer N_Ar/N_As has passed its time-out
                       value N_Asmax/N_Armax; it can be issued to service user on both the sender and receiver side. */

    N_TIMEOUT_Bs,   /* This value is issued to the service user when the timer N_Bs has passed its time-out value
                       N_Bsmax; it can be issued to the service user on the sender side only. */

    N_TIMEOUT_Cr,   /* This value is issued to the service user when the timer N_Cr has passed its time-out value
                       N_Crmax; it can be issued to the service user on the receiver side only. */

    N_WRONG_SN,     /* This value is issued to the service user upon reception of an unexpected sequence number
                       (PCI.SN) value; it can be issued to the service user on the receiver side only. */

    N_INVALID_FS,   /* This value is issued to the service user when an invalid or unknown FlowStatus value has
                       been received in a flow control (FC) N_PDU; it can be issued to the service user on the sender side only. */

    N_UNEXP_PDU,    /* This value is issued to the service user upon reception of an unexpected protocol data unit;
                       it can be issued to the service user on the receiver side only. */

    N_WTF_OVRN,     /* This value is issued to the service user upon reception of flow control WAIT frame that
                       exceeds the maximum counter N_WFTmax. */

    N_BUFFER_OVFLW, /* This value is issued to the service user upon reception of a flow control (FC) N_PDU with
                       FlowStatus = OVFLW. It indicates that the buffer on the receiver side of a segmented
                       message transmission cannot store the number of bytes specified by the FirstFrame
                       DataLength (FF_DL) parameter in the FirstFrame and therefore the transmission of the
                       segmented message was aborted. It can be issued to the service user on the sender side only. */

    N_ERROR         /* This is the general error value. It shall be issued to the service user when an error has been
                       detected by the network layer and no other parameter value can be used to better describe
                       the error. It can be issued to the service user on both the sender and receiver side. */
} tN_Result;

typedef enum
{
    CANTP_TX_MSG_IDLE = 0, /* CAN TP TX message idle */
    CANTP_TX_MSG_SUCC,     /* CAN TP TX message successful */
    CANTP_TX_MSG_FAIL,     /* CAN TP TX message fail */
    CANTP_TX_MSG_WAITING   /* CAN TP waiting TX message */
} tCanTPTxMsgStatus;

typedef struct
{
    tUdsId xCanTpId;                 /* CAN TP message ID */
    tCanTpDataLen xPduDataLen;       /* PDU data len(RX/TX data len) */
    tCanTpDataLen xFFDataLen;        /* RX/TX FF data len */
    uint8 aDataBuf[MAX_CF_DATA_LEN]; /* RX/TX data buffer */
} tCanTpDataInfo;

typedef struct
{
    uint8 ucSN;               /* SN */
    uint8 ucBlockSize;        /* Block size */
    tNetTime xSTmin;          /* STmin */
    tNetTime xMaxWatiTimeout; /* Timeout time */
    tCanTpDataInfo stCanTpDataInfo;
} tCanTpInfo;

typedef struct
{
    uint8 isFree;            /* RX message status. TRUE = not received message. */
    tUdsId xMsgId;           /* Received message ID */
    uint8 msgLen;            /* Received message len */
    uint8 aMsgBuf[DATA_LEN]; /* Message data buffer */
} tCanTpMsg;

typedef tN_Result (*tpfCanTpFun)(tCanTpMsg *, tCanTpWorkStatus *);
typedef struct
{
    tCanTpWorkStatus eCanTpStaus;
    tpfCanTpFun pfCanTpFun;
} tCanTpFunInfo;

static tCanTpInfo gs_stCanTPTxDataInfo;      /* CAN TP TX data */
static tNetTime gs_xCanTPTxSTmin = 0u;       /* TX STmin */
static uint32 gs_CANTPTxMsgMaxWaitTime = 0u; /* TX message max wait time, RX / TX frame both used waiting status */
static tCanTpInfo gs_stCanTPRxDataInfo;      /* CAN TP RX data */
static tCanTpWorkStatus gs_eCanTpWorkStatus = IDLE;
static volatile tCanTPTxMsgStatus gs_eCANTPTxMsStatus = CANTP_TX_MSG_IDLE;
static tpfNetTxCallBack gs_pfCANTPTxMsgCallBack = NULL_PTR;

#define CanTpTimeToCount(xTime) ((xTime) / g_stCANUdsNetLayerCfgInfo.ucCalledPeriod)
#define IsSF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) == SF) ? TRUE : FALSE)
#define IsFF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) == FF) ? TRUE : FALSE)
#define IsCF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) == CF) ? TRUE : FALSE)
#define IsFC(xNetWorkFrameType) ((((xNetWorkFrameType)>> 4u) == FC) ? TRUE : FALSE)
#define IsRxSNValid(xSN) ((gs_stCanTPRxDataInfo.ucSN == ((xSN) & 0x0Fu)) ? TRUE : FALSE)
#define AddWaitSN()\
    do{\
        gs_stCanTPRxDataInfo.ucSN++;\
        if(gs_stCanTPRxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stCanTPRxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

#define GetFrameLen(pucRxData, pxDataLen)\
    do{\
        if(TRUE == IsFF(pucRxData[0u]))\
        {\
            *(pxDataLen) = ((uint16)(pucRxData[0u] & 0x0fu) << 8u) | (uint16)pucRxData[1u];\
        }\
        else\
        {\
            *(pxDataLen) = (uint8)(pucRxData[0u] & 0x0fu);\
        }\
    }while(0u)

/* Get RX SF frame message length */
static boolean GetRXSFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen);

/* Get RX FF frame message length */
static boolean GetRXFFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen);

/* Check received message length valid or not? */
#define IsRxMsgLenValid(address_type, frameLen, RXCANMsgLen) ((address_type == NORMAL_ADDRESSING) ? (frameLen <= RXCANMsgLen - 1) : (frameLen <= RXCANMsgLen - 2))

/* Save FF data len */
#define SaveFFDataLen(i_xRxFFDataLen) (gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen = i_xRxFFDataLen)

/* Set BS */
#define SetBlockSize(pucBSBuf, xBlockSize) (*(pucBSBuf) = (uint8)(xBlockSize))

/* Add block size */
#define AddBlockSize()\
    do{\
        if(0u != g_stCANUdsNetLayerCfgInfo.xBlockSize)\
        {\
            gs_stCanTPRxDataInfo.ucBlockSize++;\
        }\
    }while(0u)

/* Set STmin */
#define SetSTmin(pucSTminBuf, xSTmin) (*(pucSTminBuf) = (uint8)(xSTmin))

/* Set wait STmin */
#define SetWaitSTmin() (gs_stCanTPRxDataInfo.xSTmin = CanTpTimeToCount(g_stCANUdsNetLayerCfgInfo.xSTmin))

/* Set wait frame time */
#define SetRxWaitFrameTime(xWaitTimeout)\
    do{\
        (gs_stCanTPRxDataInfo.xMaxWatiTimeout = CanTpTimeToCount(xWaitTimeout));\
        gs_CANTPTxMsgMaxWaitTime = gs_stCanTPRxDataInfo.xMaxWatiTimeout;\
    }while(0u);

/* RX frame set RX msg wait time */
#define RXFrame_SetRxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/* RX frame set TX msg wait time */
#define RXFrame_SetTxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/* Set wait SN */
#define SetWaitSN(xSN) (gs_stCanTPRxDataInfo.ucSN = xSN)

/* Set FS */
#define SetFS(pucFsBuf, xFlowStatus) (*(pucFsBuf) = (*(pucFsBuf) & 0xF0u) | (uint8)(xFlowStatus))

/* Clear receive data buffer */
#define ClearRxDataBuf()\
    do{\
        fsl_memset(&gs_stCanTPRxDataInfo, 0u, sizeof(gs_stCanTPRxDataInfo));\
    }while(0u)

/* Add received data len */
#define AddRxDataLen(xRxDataLen) (gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen += (xRxDataLen))

/* Is received consecutive frame all. */
#define IsReceiveCFAll(xCFDataLen) (((gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen + (uint8)(xCFDataLen))\
                                    >= gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/* Is STmin timeout? */
#define IsSTminTimeOut() ((0u == gs_stCanTPRxDataInfo.xSTmin) ? TRUE : FALSE)

/* Is wait Flow control timeout? */
#define IsWaitFCTimeout()  ((0u == gs_stCanTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/* Is wait consecutive frame timeout? */
#define IsWaitCFTimeout() ((0u == gs_stCanTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/* Is block size overflow */
#define IsRxBlockSizeOverflow() (((0u != g_stCANUdsNetLayerCfgInfo.xBlockSize) &&\
                                  (gs_stCanTPRxDataInfo.ucBlockSize >= g_stCANUdsNetLayerCfgInfo.xBlockSize))\
                                 ? TRUE : FALSE)

/* Is transmitted data len overflow max SF? */
#define IsTxDataLenOverflowSF() ((gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen > TX_SF_DATA_MAX_LEN) ? TRUE : FALSE)

/* Is transmitted data less than min? */
#define IsTxDataLenLessSF() ((0u == gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/* Set transmitted SF data len */
#define SetTxSFDataLen(pucSFDataLenBuf, xTxSFDataLen)\
    do{\
        *(pucSFDataLenBuf) &= 0xF0u;\
        (*(pucSFDataLenBuf) |= (xTxSFDataLen));\
    }while(0u)

/* Set transmitted FF data len */
#define SetTxFFDataLen(pucTxFFDataLenBuf, xTxFFDataLen)\
    do{\
        *(pucTxFFDataLenBuf + 0u) &= 0xF0u;\
        *(pucTxFFDataLenBuf + 0u) |= (uint8)((xTxFFDataLen) >> 8u);\
        *(pucTxFFDataLenBuf + 1u) |= (uint8)(xTxFFDataLen);\
    }while(0u)

/* Add TX data len */
#define AddTxDataLen(xTxDataLen) (gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen += (xTxDataLen))

/* Set TX STmin */
#define SetTxSTmin() (gs_stCanTPTxDataInfo.xSTmin = CanTpTimeToCount(gs_xCanTPTxSTmin))

/* Save TX STmin */
#define SaveTxSTmin(xTxSTmin) (gs_xCanTPTxSTmin = xTxSTmin)

/* Is TX STmin timeout? */
#define IsTxSTminTimeout() ((0u == gs_stCanTPTxDataInfo.xSTmin) ? TRUE : FALSE)

/* Set TX wait frame time */
#define SetTxWaitFrameTime(xWaitTime)\
    do{\
        (gs_stCanTPTxDataInfo.xMaxWatiTimeout = CanTpTimeToCount(xWaitTime));\
        gs_CANTPTxMsgMaxWaitTime = gs_stCanTPTxDataInfo.xMaxWatiTimeout;\
    }while(0u);

/* TX frame set TX message wait time */
#define TXFrame_SetTxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/* TX frame set TX message wait time */
#define TXFrame_SetRxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/* Is TX wait frame timeout? */
#define IsTxWaitFrameTimeout() ((0u == gs_stCanTPTxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/* Is TX message wait frame timeout? */
#define IsTxMsgWaitingFrameTimeout() ((0u == gs_CANTPTxMsgMaxWaitTime) ? TRUE : FALSE)

/* Get FS */
#define GetFS(ucFlowStaus, pxFlowStatusBuf) (*(pxFlowStatusBuf) = (ucFlowStaus) & 0x0Fu)

/* Set TX SN */
#define SetTxSN(pucSNBuf) (*(pucSNBuf) = gs_stCanTPTxDataInfo.ucSN | (*(pucSNBuf) & 0xF0u))

/* Add TX SN */
#define AddTxSN()\
    do{\
        gs_stCanTPTxDataInfo.ucSN++;\
        if(gs_stCanTPTxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stCanTPTxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

/* Is TX all */
#define IsTxAll() ((gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen >= \
                    gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/* Save received message ID */
#define SaveRxMsgId(xMsgId) (gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId = (xMsgId))

/* Clear CAN TP RX msg buffer */
#define ClearCanTpRxMsgBuf(pMsgInfo)\
    do{\
        (pMsgInfo)->isFree = TRUE;\
        (pMsgInfo)->msgLen = 0u;\
        (pMsgInfo)->xMsgId = 0u;\
    }while(0u)

/* Get cur CAN TP status */
#define GetCurCANTPStatus() (gs_eCanTpWorkStatus)

/* Set cur CAN TP status */
#define SetCurCANTPSatus(status)\
    do{\
        gs_eCanTpWorkStatus = status;\
    }while(0u)

/* Get cur CAN TP status PTR */
#define GetCurCANTPStatusPtr() (&gs_eCanTpWorkStatus)

/* CAN TP IDLE */
static tN_Result CANTP_DoCanTpIdle(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Do receive single frame */
static tN_Result CANTP_DoReceiveSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Do receive first frame */
static tN_Result CANTP_DoReceiveFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Do receive consecutive frame */
static tN_Result CANTP_DoReceiveCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Transmit FC callback */
static void CANTP_DoTransmitFCCallBack(void);

/* Transmit flow control frame */
static tN_Result CANTP_DoTransmitFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Transmit single frame */
static tN_Result CANTP_DoTransmitSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Transmit SF callback */
static void CANTP_DoTransmitSFCallBack(void);

/* Transmit FF callback */
static void CANTP_DoTransmitFFCallBack(void);

/* Transmit first frame */
static tN_Result CANTP_DoTransmitFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Wait flow control frame */
static tN_Result CANTP_DoReceiveFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Transmit CF callback */
static void CANTP_DoTransmitCFCallBack(void);

/* Transmit consecutive frame */
static tN_Result CANTP_DoTransmitCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);

/* Waiting TX message */
static tN_Result CANTP_DoWaitingTxMsg(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus);


/* Set transmit frame type */
static uint8 CANTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8 *o_pucFrameType);

/* Received a CAN TP frame, copy these data in FIFO. */
static uint8 CANTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxCanID,
                                          const tLen i_xRxDataLen,
                                          const uint8 *i_pDataBuf);

/* UDS transmitted a application frame data, copy these data in TX FIFO. */
static uint8 CANTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxCanID,
                                           uint8 *o_pTxDataLen,
                                           uint8 *o_pDataBuf);

/* CAN TP TX message callback */
static void CANTP_TxMsgSuccessfulCallBack(void);

/* CANP TP set TX message status */
static void CANTP_SetTxMsgStatus(const tCanTPTxMsgStatus i_eTxMsgStatus);

/* Register TX message successful callback */
static void CANTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack);

/* Do register TX message callback */
static void CANTP_DoRegisterTxMsgCallBack(void);

static const tCanTpFunInfo gs_astCanTpFunInfo[] =
{
    {IDLE, CANTP_DoCanTpIdle},
    {RX_SF, CANTP_DoReceiveSF},
    {RX_FF, CANTP_DoReceiveFF},
    {TX_FC, CANTP_DoTransmitFC},
    {RX_CF, CANTP_DoReceiveCF},

    {TX_SF, CANTP_DoTransmitSF},
    {TX_FF, CANTP_DoTransmitFF},
    {RX_FC, CANTP_DoReceiveFC},
    {TX_CF, CANTP_DoTransmitCF},
    {WAITING_TX, CANTP_DoWaitingTxMsg}
};

void CANTP_Init(void)
{
    tErroCode eStatus;
    ApplyFifo(RX_TP_QUEUE_LEN, RX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        TPDebugPrintf("Apply RX_TP_QUEUE_ID failed!\n");

        while (1)
        {
        }
    }

    ApplyFifo(TX_TP_QUEUE_LEN, TX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        TPDebugPrintf("Apply TX_TP_QUEUE_ID failed\n");

        while (1)
        {
        }
    }

    ApplyFifo(RX_BUS_FIFO_LEN, RX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        TPDebugPrintf("Apply RX_BUS_FIFO failed!\n");

        while (1)
        {
        }
    }

    ApplyFifo(TX_BUS_FIFO_LEN, TX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        TPDebugPrintf("Apply TX_BUS_FIFOfailed!\n");

        while (1)
        {
        }
    }
}

/* CAN TP system tick control. This function should period called by system. */
void CANTP_SytstemTickControl(void)
{
    if (gs_stCanTPRxDataInfo.xSTmin)
    {
        gs_stCanTPRxDataInfo.xSTmin--;
    }

    if (gs_stCanTPRxDataInfo.xMaxWatiTimeout)
    {
        gs_stCanTPRxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_stCanTPTxDataInfo.xSTmin)
    {
        gs_stCanTPTxDataInfo.xSTmin--;
    }

    if (gs_stCanTPTxDataInfo.xMaxWatiTimeout)
    {
        gs_stCanTPTxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_CANTPTxMsgMaxWaitTime)
    {
        gs_CANTPTxMsgMaxWaitTime--;
    }
}

/* UDS network man function */
void CANTP_MainFun(void)
{
    uint8 index = 0u;
    const uint8 findCnt = sizeof(gs_astCanTpFunInfo) / sizeof(gs_astCanTpFunInfo[0u]);
    tCanTpMsg stRxCanTpMsg = {TRUE, 0u, 0u, {0u}};
    tN_Result result = N_OK;

    /* In waiting TX message, cannot read message from FIFO. Because, In waiting message will lost read messages. */
    if (WAITING_TX != GetCurCANTPStatus())
    {
        /* Read msg from CAN driver RxFIFO */
        if (TRUE == g_stCANUdsNetLayerCfgInfo.pfNetRx(&stRxCanTpMsg.xMsgId,
                                                      &stRxCanTpMsg.msgLen,
                                                      stRxCanTpMsg.aMsgBuf))
        {
            /* Check received message ID valid? */
            if (TRUE == CANTP_IsReceivedMsgIDValid(stRxCanTpMsg.xMsgId))
            {
                stRxCanTpMsg.isFree = FALSE;
            }
        }
    }

    while (index < findCnt)
    {
        if (GetCurCANTPStatus() == gs_astCanTpFunInfo[index].eCanTpStaus)
        {
            if (NULL_PTR != gs_astCanTpFunInfo[index].pfCanTpFun)
            {
                result = gs_astCanTpFunInfo[index].pfCanTpFun(&stRxCanTpMsg, GetCurCANTPStatusPtr());
            }
        }

        /* If received unexpected PDU, then jump to IDLE and restart do progresses. */
        if (N_UNEXP_PDU != result)
        {
            if (N_OK != result)
            {
                SetCurCANTPSatus(IDLE);
            }

            index++;
        }
        else
        {
            index = 0u;
        }
    }

    ClearCanTpRxMsgBuf(&stRxCanTpMsg);
    /* Check CAN TP TX message successful? */
    CANTP_DoRegisterTxMsgCallBack();
}

/* Received a CAN TP frame, copy these data in UDS RX FIFO. */
static uint8 CANTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxCanID,
                                          const tLen i_xRxDataLen,
                                          const uint8 *i_pDataBuf)
{
    tErroCode eStatus;
    tLen xCanWriteLen = 0u;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;
    ASSERT(NULL_PTR == i_pDataBuf);

    if (0u == i_xRxDataLen)
    {
        return FALSE;
    }

    /* Check can write data len */
    GetCanWriteLen(RX_TP_QUEUE_ID, &xCanWriteLen, &eStatus);

    if ((ERRO_NONE != eStatus) || (xCanWriteLen < (i_xRxDataLen + sizeof(tUDSAndTPExchangeMsgInfo))))
    {
        return FALSE;
    }

    exchangeMsgInfo.msgID = i_xRxCanID;
    exchangeMsgInfo.dataLen = i_xRxDataLen;
    exchangeMsgInfo.pfCallBack = NULL_PTR;
    /* Write data UDS transmit ID and data len */
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8 *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    /* Write data in FIFO */
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8 *)i_pDataBuf, i_xRxDataLen, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    return TRUE;
}

/* UDS transmitted a application frame data, copy these data in TX FIFO. */
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
    /* Can read data from buffer */
    GetCanReadLen(TX_TP_QUEUE_ID, &xRealReadLen, &eStatus);

    if ((ERRO_NONE != eStatus) || (0u == xRealReadLen) || (xRealReadLen < sizeof(tUDSAndTPExchangeMsgInfo)))
    {
        return FALSE;
    }

    /* Read receive ID */
    ReadDataFromFifo(TX_TP_QUEUE_ID,
                     sizeof(tUDSAndTPExchangeMsgInfo),
                     (uint8 *)&exchangeMsgInfo,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || sizeof(tUDSAndTPExchangeMsgInfo) != xRealReadLen)
    {
        return FALSE;
    }

    /* Read data from FIFO */
    ReadDataFromFifo(TX_TP_QUEUE_ID,
                     exchangeMsgInfo.dataLen,
                     o_pDataBuf,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || exchangeMsgInfo.dataLen != xRealReadLen)
    {
        return FALSE;
    }

    *o_pxTxCanID = exchangeMsgInfo.msgID;
    *o_pTxDataLen = exchangeMsgInfo.dataLen;
    TP_RegisterTransmittedAFrmaeMsgCallBack(exchangeMsgInfo.pfCallBack);
    return TRUE;
}

/* CAN TP IDLE */
static tN_Result CANTP_DoCanTpIdle(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 txDataLen = (uint8)gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen;
    ASSERT(NULL_PTR == m_peNextStatus);
    /* Clear CAN TP data */
    fsl_memset((void *)&gs_stCanTPRxDataInfo, 0u, sizeof(tCanTpInfo));
    fsl_memset((void *)&gs_stCanTPTxDataInfo, 0u, sizeof(tCanTpInfo));
    /* Clear waiting time */
    gs_CANTPTxMsgMaxWaitTime = 0u;
    /* Set NULL to transmitted message callback */
    TP_RegisterTransmittedAFrmaeMsgCallBack(NULL_PTR);

    /* If receive can TP message, judge type. Only received SF or FF message. Other frames ignore. */
    if (FALSE == m_stMsgInfo->isFree)
    {
        if (TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u]))
        {
            *m_peNextStatus = RX_SF;
        }
        else if (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u]))
        {
            *m_peNextStatus = RX_FF;
        }
        else
        {
            TPDebugPrintf("\n %s received invalid message!\n", __func__);
        }
    }
    else
    {
        /* Judge have message can will TX. */
        if (TRUE == CANTP_CopyAFrameFromFifoToBuf(&gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                  &txDataLen,
                                                  gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf))
        {
            gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen = txDataLen;

            if (TRUE == IsTxDataLenOverflowSF())
            {
                *m_peNextStatus = TX_FF;
            }
            else
            {
                *m_peNextStatus = TX_SF;
            }
        }
    }

    return N_OK;
}

/* Do receive single frame */
static tN_Result CANTP_DoReceiveSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint32 SFLen = 0u;
    ASSERT(NULL_PTR == m_peNextStatus);

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree))
    {
        return N_ERROR;
    }

    if (TRUE != IsSF(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_ERROR;
    }

    /* Get RX frame: SF length */
    if (TRUE != GetRXSFFrameMsgLength(m_stMsgInfo->msgLen, m_stMsgInfo->aMsgBuf, &SFLen))
    {
        TPDebugPrintf("SF:GetRXSFFrameMsgLength failed!\n");
        return N_ERROR;
    }

    /* Write data to UDS FIFO */
    if (FALSE == CANTP_CopyAFrameDataInRxFifo(m_stMsgInfo->xMsgId,
                                              SFLen,
                                              &m_stMsgInfo->aMsgBuf[1u]))
    {
        TPDebugPrintf("Copy data error!\n");
        return N_ERROR;
    }

    *m_peNextStatus = IDLE;
    return N_OK;
}

/* Do receive first frame */
static tN_Result CANTP_DoReceiveFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint32 FFDataLen = 0u;
    ASSERT(NULL_PTR == m_peNextStatus);

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree))
    {
        return N_ERROR;
    }

    if (TRUE != IsFF(m_stMsgInfo->aMsgBuf[0u]))
    {
        TPDebugPrintf("Received not FF\n");
        return N_ERROR;
    }

    /* Get FF Data len */
    if (TRUE != GetRXFFFrameMsgLength(m_stMsgInfo->msgLen, m_stMsgInfo->aMsgBuf, &FFDataLen))
    {
        TPDebugPrintf("FF:GetRXFrameMsgLength failed!\n");
        return N_ERROR;
    }

    /* Save received msg ID */
    SaveRxMsgId(m_stMsgInfo->xMsgId);
    /* Write data in global buffer. When receive all data, write these data in FIFO. */
    SaveFFDataLen(FFDataLen);
    /* Set wait flow control time */
    RXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBr);
    /* Copy data in global buffer */
    fsl_memcpy(gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf, (const void *)&m_stMsgInfo->aMsgBuf[2u], m_stMsgInfo->msgLen - 2u);
    AddRxDataLen(m_stMsgInfo->msgLen - 2u);
    /* Jump to next status */
    *m_peNextStatus = TX_FC;
    ClearCanTpRxMsgBuf(m_stMsgInfo);
    return N_OK;
}


/* Do receive consecutive frame */
static tN_Result CANTP_DoReceiveCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    ASSERT(NULL_PTR == m_peNextStatus);

    /* Is timeout RX wait timeout? If wait timeout receive CF over. */
    if (TRUE == IsWaitCFTimeout())
    {
        TPDebugPrintf("Wait consecutive frame timeout!\n");
        *m_peNextStatus = IDLE;
        return N_TIMEOUT_Cr;
    }

    if (0u == m_stMsgInfo->msgLen || TRUE == m_stMsgInfo->isFree)
    {
        /* Waiting CF message, It's normally for not received CAN message in the step. */
        return N_OK;
    }

    /* Check received message is SF or FF? If received SF or FF, start new receive progresses. */
    if ((TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u])) || (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u])))
    {
        TPDebugPrintf("In receive progresses: received SF\n");
        *m_peNextStatus = IDLE;
        return N_UNEXP_PDU;
    }

    if (gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId != m_stMsgInfo->xMsgId)
    {
#ifdef EN_TP_DEBUG
        TPDebugPrintf("Msg ID invalid in CF! F RX ID = %X, RX ID = %X\n",
                      gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId, m_stMsgInfo->xMsgId);
#endif
        return N_ERROR;
    }

    if (TRUE != IsCF(m_stMsgInfo->aMsgBuf[0u]))
    {
#ifdef EN_TP_DEBUG
        TPDebugPrintf("Msg type invalid in CF %X!\n", m_stMsgInfo->aMsgBuf[0u]);
#endif
        return N_ERROR;
    }

    /* Get received SN. If SN invalid, return FALSE. */
    if (TRUE != IsRxSNValid(m_stMsgInfo->aMsgBuf[0u]))
    {
        TPDebugPrintf("Msg SN invalid in CF!\n");
        return N_WRONG_SN;
    }

    /* Check receive CF all? If receive all, copy data in FIFO and clear receive
    buffer information. Else count SN and add receive data len. */
    if (TRUE == IsReceiveCFAll(m_stMsgInfo->msgLen - 1u))
    {
        /* Copy all data in FIFO and receive over. */
        fsl_memcpy(&gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen - gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen);
        /* Copy all data in FIFO */
        (void)CANTP_CopyAFrameDataInRxFifo(gs_stCanTPRxDataInfo.stCanTpDataInfo.xCanTpId,
                                           gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen,
                                           gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf);
        *m_peNextStatus = IDLE;
    }
    else
    {
        /* If is block size overflow. */
        if (TRUE == IsRxBlockSizeOverflow())
        {
            RXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBr);
            *m_peNextStatus = TX_FC;
        }
        else
        {
            /* Count SN and set STmin, wait timeout time */
            AddWaitSN();
            /* Set wait frame time */
            RXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCr);
        }

        /* Copy data in global FIFO */
        fsl_memcpy(&gs_stCanTPRxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPRxDataInfo.stCanTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   m_stMsgInfo->msgLen - 1u);
        AddRxDataLen(m_stMsgInfo->msgLen - 1u);
    }

    return N_OK;
}

/* Transmit FC callback */
static void CANTP_DoTransmitFCCallBack(void)
{
    if (gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen > MAX_CF_DATA_LEN)
    {
        SetCurCANTPSatus(IDLE);
    }
    else
    {
        /* Set wait STmin */
        RXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCr);
        SetCurCANTPSatus(RX_CF);
    }
}

/* Transmit flow control frame */
static tN_Result CANTP_DoTransmitFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aucTransDataBuf[DATA_LEN] = {0u};

    /* Is wait FC timeout? */
    if (TRUE != IsWaitFCTimeout())
    {
        TPDebugPrintf("\n Waiting transmit FC not timeout!\n");
        /* Waiting timeout for transmit FC */
        return N_OK;
    }

    /* Set frame type */
    (void)CANTP_SetFrameType(FC, &aucTransDataBuf[0u]);

    /* Check current buffer. */
    if (gs_stCanTPRxDataInfo.stCanTpDataInfo.xFFDataLen > MAX_CF_DATA_LEN)
    {
        /* Set FS */
        SetFS(&aucTransDataBuf[1u], OVERFLOW_BUF);
    }
    else
    {
        SetFS(&aucTransDataBuf[1u], CONTINUE_TO_SEND);
    }

    /* Set BS */
    SetBlockSize(&aucTransDataBuf[1u], g_stCANUdsNetLayerCfgInfo.xBlockSize);
    /* Add block size */
    AddBlockSize();
    /* Add wait SN */
    AddWaitSN();
    /* Set STmin */
    SetSTmin(&aucTransDataBuf[2u], g_stCANUdsNetLayerCfgInfo.xSTmin);
    /* Set wait next frame  max time */
    RXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAr);
    /* CAN TP set TX message status and register TX message successful callback. */
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitFCCallBack);

    /* Transmit flow control */
    if (TRUE == g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(g_stCANUdsNetLayerCfgInfo.xTxId,
                                                     sizeof(aucTransDataBuf),
                                                     aucTransDataBuf,
                                                     CANTP_TxMsgSuccessfulCallBack,
                                                     g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
    {
        *m_peNextStatus = WAITING_TX;
        return N_OK;
    }

    /* CAN TP set TX message status and register TX message successful callback. */
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
    CANTP_RegisterTxMsgCallBack(NULL_PTR);
    /* Transmit message failed and do idle */
    *m_peNextStatus = IDLE;
    return N_ERROR;
}

/* Transmit SF callback */
static void CANTP_DoTransmitSFCallBack(void)
{
    TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);
    SetCurCANTPSatus(IDLE);
}

/* Transmit single frame */
static tN_Result CANTP_DoTransmitSF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aDataBuf[DATA_LEN] = {0u};
    uint8 txLen = 0u;
    ASSERT(NULL_PTR == m_peNextStatus);

    /* Check transmit data len. If data len overflow Max SF, return FALSE. */
    if (TRUE == IsTxDataLenOverflowSF())
    {
        *m_peNextStatus = TX_FF;
        return N_ERROR;
    }

    if (TRUE == IsTxDataLenLessSF())
    {
        *m_peNextStatus = IDLE;
        return N_ERROR;
    }

    /* Set transmitted frame type */
    (void)CANTP_SetFrameType(SF, &aDataBuf[0u]);
    /* Set transmitted data len */
    SetTxSFDataLen(&aDataBuf[0u], gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen);
    txLen = aDataBuf[0u] + 1u;
    /* Copy data in TX buffer */
    fsl_memcpy(&aDataBuf[1u],
               gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf,
               gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen);
    /* CAN TP set TX message status and register TX message successful callback. */
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitSFCallBack);

    /* Request transmitted application message. */
    if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                     txLen,
                                                     aDataBuf,
                                                     CANTP_TxMsgSuccessfulCallBack,
                                                     g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
    {
        /* CAN TP set TX message status and register TX message successful callback. */
        CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
        CANTP_RegisterTxMsgCallBack(NULL_PTR);
        /* Send message error */
        *m_peNextStatus = IDLE;
        /* Request transmitted application message failed. */
        return N_ERROR;
    }

    /* Set wait send frame successful max time */
    TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAs);
    /* Jump to idle and clear transmitted message. */
    *m_peNextStatus = WAITING_TX;
    return N_OK;
}

/* Transmit FF callback */
static void CANTP_DoTransmitFFCallBack(void)
{
    /* Add TX data len */
    AddTxDataLen(FF_DATA_MIN_LEN - 2);
    /* Set TX wait time */
    TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBs);
    /* Jump to idle and clear transmitted message. */
    AddTxSN();
    SetCurCANTPSatus(RX_FC);
}


/* Transmit first frame */
static tN_Result CANTP_DoTransmitFF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aDataBuf[DATA_LEN] = {0u};
    ASSERT(NULL_PTR == m_peNextStatus);

    /* Check transmit data len. If data len overflow less than SF, return FALSE. */
    if (TRUE != IsTxDataLenOverflowSF())
    {
        *m_peNextStatus = TX_SF;
        return N_BUFFER_OVFLW;
    }

    /* Set transmitted frame type */
    (void)CANTP_SetFrameType(FF, &aDataBuf[0u]);
    /* Set transmitted data len */
    SetTxFFDataLen(aDataBuf, gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen);
    /* CAN TP set TX message status and register TX message successful callback. */
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitFFCallBack);
    /* Copy data in TX buffer */
    fsl_memcpy(&aDataBuf[2u], gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf, FF_DATA_MIN_LEN - 2);

    /* Request transmitted application message. */
    if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                     sizeof(aDataBuf),
                                                     aDataBuf,
                                                     CANTP_TxMsgSuccessfulCallBack,
                                                     g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
    {
        /* CAN TP set TX message status and register TX message successful callback. */
        CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
        CANTP_RegisterTxMsgCallBack(NULL_PTR);
        /* Send message error */
        *m_peNextStatus = IDLE;
        /* Request transmitted application message failed. */
        return N_ERROR;
    }

    /* Set wait send frame successful max time */
    TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAs);
    /* Jump to idle and clear transmitted message. */
    *m_peNextStatus = WAITING_TX;
    return N_OK;
}

/* Wait flow control frame */
static tN_Result CANTP_DoReceiveFC(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    tFlowStatus eFlowStatus;
    ASSERT(NULL_PTR == m_peNextStatus);

    /* If TX message wait FC timeout jump to IDLE. */
    if (TRUE == IsTxWaitFrameTimeout())
    {
        TPDebugPrintf("Wait flow control timeout.\n");
        *m_peNextStatus = IDLE;
        return N_TIMEOUT_Cr;
    }

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree))
    {
        /* Waiting received FC. It's normally for waiting CAN message and return OK. */
        return N_OK;
    }

    if (TRUE != IsFC(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_ERROR;
    }

    /* Get flow status */
    GetFS(m_stMsgInfo->aMsgBuf[0u], &eFlowStatus);

    if (OVERFLOW_BUF == eFlowStatus)
    {
        *m_peNextStatus = IDLE;
        return N_BUFFER_OVFLW;
    }

    /* Wait flow control */
    if (WAIT_FC == eFlowStatus)
    {
        /* Set TX wait time */
        TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBs);
        return N_OK;
    }

    /* Continue to send */
    if (CONTINUE_TO_SEND == eFlowStatus)
    {
        SetBlockSize(&gs_stCanTPTxDataInfo.ucBlockSize, m_stMsgInfo->aMsgBuf[1u]);
        SaveTxSTmin(m_stMsgInfo->aMsgBuf[2u]);
        TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCs);
        /* Remove Add TX SN, because this SN is added in send First frame callback */
#if 0
        AddTxSN();
#endif
    }
    else
    {
        /* Received error Flow control */
        *m_peNextStatus = IDLE;
        return N_INVALID_FS;
    }

    *m_peNextStatus = TX_CF;
    return N_OK;
}

/* Transmit CF callback */
static void CANTP_DoTransmitCFCallBack(void)
{
    if (TRUE == IsTxAll())
    {
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);
        SetCurCANTPSatus(IDLE);
        return;
    }

    /* Set transmitted next frame min time. */
    SetTxSTmin();

    if (gs_stCanTPTxDataInfo.ucBlockSize)
    {
        gs_stCanTPTxDataInfo.ucBlockSize--;

        /* Block size is equal 0,  waiting  flow control message. if not equal 0, continual send CF message. */
        if (0u == gs_stCanTPTxDataInfo.ucBlockSize)
        {
            SetCurCANTPSatus(RX_FC);
            TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNBs);
            return;
        }
    }

    AddTxSN();
    /* Set TX next frame max time. */
    TXFrame_SetRxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNCs);
    SetCurCANTPSatus(TX_CF);
}


/* Transmit Consecutive Frame */
static tN_Result CANTP_DoTransmitCF(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    uint8 aTxDataBuf[DATA_LEN] = {0u};
    uint8 TxLen = 0u;
    uint8 aTxAllLen = 0u;
    ASSERT(NULL_PTR == m_peNextStatus);

    /* Is TX STmin timeout? */
    if (FALSE == IsTxSTminTimeout())
    {
        /* Waiting STmin timeout. It's normally in the step. */
        return N_OK;
    }

    /* Is transmitted timeout? */
    if (TRUE == IsTxWaitFrameTimeout())
    {
        *m_peNextStatus = IDLE;
        return N_TIMEOUT_Bs;
    }

    (void)CANTP_SetFrameType(CF, &aTxDataBuf[0u]);
    SetTxSN(&aTxDataBuf[0u]);
    TxLen = gs_stCanTPTxDataInfo.stCanTpDataInfo.xFFDataLen - gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen;
    /* CAN TP set TX message status and register TX message successful callback. */
    CANTP_SetTxMsgStatus(CANTP_TX_MSG_WAITING);
    CANTP_RegisterTxMsgCallBack(CANTP_DoTransmitCFCallBack);

    if (TxLen >= CF_DATA_MAX_LEN)
    {
        fsl_memcpy(&aTxDataBuf[1u],
                   &gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen],
                   CF_DATA_MAX_LEN);

        /* Request transmitted application message. */
        if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                         sizeof(aTxDataBuf),
                                                         aTxDataBuf,
                                                         CANTP_TxMsgSuccessfulCallBack,
                                                         g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
        {
            /* CAN TP set TX message status and register TX message successful callback. */
            CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
            CANTP_RegisterTxMsgCallBack(NULL_PTR);
            /* Send message error */
            *m_peNextStatus = IDLE;
            /* Request transmitted application message failed. */
            return N_ERROR;
        }

        AddTxDataLen(CF_DATA_MAX_LEN);
    }
    else
    {
        fsl_memcpy(&aTxDataBuf[1u],
                   &gs_stCanTPTxDataInfo.stCanTpDataInfo.aDataBuf[gs_stCanTPTxDataInfo.stCanTpDataInfo.xPduDataLen],
                   TxLen);
        aTxAllLen = TxLen + 1u;

        /* Request transmitted application message. */
        if (TRUE != g_stCANUdsNetLayerCfgInfo.pfNetTxMsg(gs_stCanTPTxDataInfo.stCanTpDataInfo.xCanTpId,
                                                         aTxAllLen,
                                                         aTxDataBuf,
                                                         CANTP_TxMsgSuccessfulCallBack,
                                                         g_stCANUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
        {
            /* CAN TP set TX message status and register TX message successful callback. */
            CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
            CANTP_RegisterTxMsgCallBack(NULL_PTR);
            /* Send message error */
            *m_peNextStatus = IDLE;
            /* Request transmitted application message failed. */
            return N_ERROR;
        }

        AddTxDataLen(TxLen);
    }

    /* Set wait send frame successful max time */
    TXFrame_SetTxMsgWaitTime(g_stCANUdsNetLayerCfgInfo.xNAs);
    *m_peNextStatus = WAITING_TX;
    return N_OK;
}

/* Waiting TX message */
static tN_Result CANTP_DoWaitingTxMsg(tCanTpMsg *m_stMsgInfo, tCanTpWorkStatus *m_peNextStatus)
{
    /* Check is waiting timeout? */
    if (TRUE == IsTxMsgWaitingFrameTimeout())
    {
        /* Abort CAN bus send message */
        if (NULL_PTR != g_stCANUdsNetLayerCfgInfo.pfAbortTXMsg)
        {
            (g_stCANUdsNetLayerCfgInfo.pfAbortTXMsg) ();
        }

        /* Tell up layer, TX message timeout */
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_TIMEOUT);
        /* CAN TP set TX message status and register TX message successful callback. */
        CANTP_SetTxMsgStatus(CANTP_TX_MSG_FAIL);
        CANTP_RegisterTxMsgCallBack(NULL_PTR);
        *m_peNextStatus = IDLE;
    }

    return N_OK;
}

/* Set transmit frame type */
static uint8 CANTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8 *o_pucFrameType)
{
    ASSERT(NULL_PTR == o_pucFrameType);

    if (SF == i_eFrameType ||
            FF == i_eFrameType ||
            FC == i_eFrameType ||
            CF == i_eFrameType)
    {
        *o_pucFrameType &= 0x0Fu;
        *o_pucFrameType |= ((uint8)i_eFrameType << 4u);
        return TRUE;
    }

    return FALSE;
}

/* CAN TP TX message callback */
static void CANTP_TxMsgSuccessfulCallBack(void)
{
    gs_eCANTPTxMsStatus = CANTP_TX_MSG_SUCC;
}

/* CANP TP set TX message status */
static void CANTP_SetTxMsgStatus(const tCanTPTxMsgStatus i_eTxMsgStatus)
{
    gs_eCANTPTxMsStatus = i_eTxMsgStatus;
}

/* Register TX message successful callback */
static void CANTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack)
{
    gs_pfCANTPTxMsgCallBack = i_pfNetTxCallBack;
}

/* Do register TX message callback */
static void CANTP_DoRegisterTxMsgCallBack(void)
{
    tCanTPTxMsgStatus CANTPTxMsgStatus = CANTP_TX_MSG_IDLE;
    /* Get the TX message status with disable interrupt for protect the variable not changed by interrupt. */
    DisableAllInterrupts();
    CANTPTxMsgStatus = gs_eCANTPTxMsStatus;
    EnableAllInterrupts();

    if (CANTP_TX_MSG_SUCC == CANTPTxMsgStatus)
    {
        if (NULL_PTR != gs_pfCANTPTxMsgCallBack)
        {
            (gs_pfCANTPTxMsgCallBack)();
            gs_pfCANTPTxMsgCallBack = NULL_PTR;
        }
    }
    else if (CANTP_TX_MSG_FAIL == CANTPTxMsgStatus)
    {
        TPDebugPrintf("\n TX msg failed callback=%X, status=%d\n", gs_pfCANTPTxMsgCallBack, gs_eCANTPTxMsStatus);
        gs_eCANTPTxMsStatus = CANTP_TX_MSG_IDLE;
        /* If TX message failed, clear TX message callback */
        gs_pfCANTPTxMsgCallBack = NULL_PTR;
    }
    else
    {
        /* do nothing */
    }
}


/* Get RX SF frame message length */
static boolean GetRXSFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen)
{
    boolean result = FALSE;
    uint32 frameLen = 0u;
    ASSERT(NULL_PTR == i_pMsgBuf);
    ASSERT(NULL_PTR == o_pFrameLen);

    if ((i_RxMsgLen <= 1u) || (TRUE != IsSF(i_pMsgBuf[0u])))
    {
        return FALSE;
    }

    /* Check received single message length based on ISO15765-2 2016 */
    if (i_RxMsgLen <= 8u)
    {
        frameLen = i_pMsgBuf[0u] & 0x0Fu;

        if ((frameLen <= SF_CAN_DATA_MAX_LEN) && (frameLen > 0u))
        {
            result = IsRxMsgLenValid(NORMAL_ADDRESSING, frameLen, i_RxMsgLen);
        }
    }
    else
    {
        frameLen = i_pMsgBuf[0u] & 0x0Fu;

        if (0u == frameLen)
        {
            frameLen = i_pMsgBuf[1u];

            if ((frameLen <= SF_CANFD_DATA_MAX_LEN) && (frameLen > 0u))
            {
                result = IsRxMsgLenValid(NORMAL_ADDRESSING, frameLen, i_RxMsgLen);
            }
        }
    }

    if (TRUE == result)
    {
        *o_pFrameLen = frameLen;
    }

    return result;
}

/* Get RX FF frame message length */
static boolean GetRXFFFrameMsgLength(const uint32 i_RxMsgLen, const uint8 *i_pMsgBuf, uint32 *o_pFrameLen)
{
    boolean result = FALSE;
    uint32 frameLen = 0u;
    uint8 index = 0u;
    ASSERT(NULL_PTR == i_pMsgBuf);
    ASSERT(NULL_PTR == o_pFrameLen);

    if ((i_RxMsgLen < 8u) || (TRUE != IsFF(i_pMsgBuf[0u])))
    {
        return FALSE;
    }

    /* Check received single message length based on ISO15765-2 2016 */
    /* Calculate FF message length */
    frameLen = (uint32)((i_pMsgBuf[0u] & 0x0Fu) << 8u) | i_pMsgBuf[1u];

    if (0u == frameLen)
    {
        /* FF message length is over 4095 Bytes */
        for (index = 0u; index < 4; index++)
        {
            frameLen <<= 8u;
            frameLen |= i_pMsgBuf[index + 2u];
        }
    }

    if (frameLen < FF_DATA_MIN_LEN)
    {
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    if (TRUE == result)
    {
        *o_pFrameLen = frameLen;
    }

    return result;
}

#endif /* EN_CAN_TP */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
