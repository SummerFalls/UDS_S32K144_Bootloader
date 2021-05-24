/*
 * @ 名称: can_tp_cfg.c
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
#include "can_tp_cfg.h"
#include "multi_cyc_fifo.h"
//#include "can_driver.h"
static tpfAbortTxMsg gs_pfCANTPAbortTxMsg = NULL_PTR;
static tpfNetTxCallBack gs_pfTxMsgSuccessfulCallBack = NULL_PTR;


static uint8 CANTP_TxMsg(const tUdsId i_xTxId,
                         const uint16 i_DataLen,
                         const uint8 *i_pDataBuf,
                         const tpfNetTxCallBack i_pfNetTxCallBack,
                         const uint32 txBlockingMaxtime);

static uint8 CANTP_RxMsg(tUdsId *o_pxRxId,
                         uint8 *o_pRxDataLen,
                         uint8 *o_pRxBuf);

static void CANTP_AbortTxMsg(void);

/* Clear CAN TP TX BUS FIFO */
static boolean CANTP_ClearTXBUSFIFO(void);

/* UDS Network layer config info */
const tUdsCANNetLayerCfg g_stCANUdsNetLayerCfgInfo =
{
    1u,                 /* Called CAN TP main function period */
    RX_FUN_ADDR_ID,     /* RX FUN ID */
    RX_PHY_ADDR_ID,     /* RX PHY ID */
    TX_RESP_ADDR_ID,    /* TX RESP ID */
    0u,                 /* BS = block size */
    1u,                 /* STmin */
    25u,                /* N_As */
    25u,                /* N_Ar */
    75u,                /* N_Bs */
    0u,                 /* N_Br */
    100u,               /* N_Cs < 0.9 N_Cr */
    150u,               /* N_Cr */
    50u,                /* TX Max blocking time(ms). > 0 mean timeout for TX. equal 0 is not waiting. */
    CANTP_TxMsg,        /* CAN TP TX */
    CANTP_RxMsg,        /* CAN TP RX */
    CANTP_AbortTxMsg,   /* Abort TX message */
};

/* CAN TP TX message: there not use CAN driver TxFIFO, directly invoked CAN send function */
static uint8 CANTP_TxMsg(const tUdsId i_xTxId,
                         const uint16 i_DataLen,
                         const uint8 *i_pDataBuf,
                         const tpfNetTxCallBack i_pfNetTxCallBack,
                         const uint32 txBlockingMaxtime)
{
    tLen xCanWriteDataLen = 0u;
    tErroCode eStatus;
    uint8 aMsgBuf[8] = {0};
    tTPTxMsgHeader TxMsgInfo;
    const uint32 msgInfoLen = sizeof(tTPTxMsgHeader) + sizeof(aMsgBuf);
    ASSERT(NULL_PTR == i_pDataBuf);

    if (i_DataLen > 8u)
    {
        return FALSE;
    }

    GetCanWriteLen(TX_BUS_FIFO, &xCanWriteDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && (msgInfoLen <= xCanWriteDataLen))
    {
        TxMsgInfo.TxMsgID = i_xTxId;
        TxMsgInfo.TxMsgLength = sizeof(aMsgBuf);
        TxMsgInfo.TxMsgCallBack = (uint32)i_pfNetTxCallBack;
        fsl_memcpy(&aMsgBuf[0u], i_pDataBuf, i_DataLen);
        WriteDataInFifo(TX_BUS_FIFO, (uint8 *)&TxMsgInfo, sizeof(tTPTxMsgHeader), &eStatus);

        if (ERRO_NONE != eStatus)
        {
            ClearFIFO(TX_BUS_FIFO, &eStatus);
            return FALSE;
        }

        WriteDataInFifo(TX_BUS_FIFO, (uint8 *)aMsgBuf, 8, &eStatus);

        if (ERRO_NONE != eStatus)
        {
            ClearFIFO(TX_BUS_FIFO, &eStatus);
            return FALSE;
        }
    }

    return TRUE;
    //ret = TransmitCANMsg(i_xTxId, i_DataLen, i_pDataBuf, i_pfNetTxCallBack, txBlockingMaxtime);
}

/* CAN TP RX message: read RX msg from CAN driver RxFIFO */
static uint8 CANTP_RxMsg(tUdsId *o_pxRxId,
                         uint8 *o_pRxDataLen,
                         uint8 *o_pRxBuf)
{
    tLen xCanRxDataLen = 0u;
    tLen xReadDataLen = 0u;
    tErroCode eStatus;
    tRxMsgInfo stRxCanMsg = {0u};
    uint8 ucIndex = 0u;
    const uint32 headerLen = sizeof(stRxCanMsg.rxDataId) + sizeof(stRxCanMsg.rxDataLen);
    ASSERT(NULL_PTR == o_pxRxId);
    ASSERT(NULL_PTR == o_pRxBuf);
    ASSERT(NULL_PTR == o_pRxDataLen);
    GetCanReadLen(RX_BUS_FIFO, &xCanRxDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && (headerLen <= xCanRxDataLen))
    {
        ReadDataFromFifo(RX_BUS_FIFO,
                         headerLen,
                         (uint8 *)&stRxCanMsg,
                         &xReadDataLen,
                         &eStatus);

        if ((ERRO_NONE == eStatus) && (headerLen <= xCanRxDataLen))
        {
            ReadDataFromFifo(RX_BUS_FIFO,
                             stRxCanMsg.rxDataLen,
                             (uint8 *)&stRxCanMsg.aucDataBuf,
                             &xCanRxDataLen,
                             &eStatus);

            if (TRUE != CANTP_IsReceivedMsgIDValid(stRxCanMsg.rxDataId))
            {
                return FALSE;
            }

            *o_pxRxId = stRxCanMsg.rxDataId;
            *o_pRxDataLen = stRxCanMsg.rxDataLen;

            for (ucIndex = 0u; ucIndex < stRxCanMsg.rxDataLen; ucIndex++)
            {
                o_pRxBuf[ucIndex] = stRxCanMsg.aucDataBuf[ucIndex];
            }

            return TRUE;
        }
    }
    else
    {
        if ((0u != xCanRxDataLen) || (ERRO_NONE != eStatus))
        {
            TPDebugPrintf("\n %s write message in FIFO failed! status = %d, FIFO avaliable Length=%d\n", __func__, eStatus, xCanRxDataLen);
        }
    }

    return FALSE;
}

/* Get config CAN TP TX Response Address ID */
tUdsId CANTP_GetConfigTxMsgID(void)
{
    return g_stCANUdsNetLayerCfgInfo.xTxId;
}

/* Get config CAN TP RX Function Address ID */
tUdsId CANTP_GetConfigRxMsgFUNID(void)
{
    return g_stCANUdsNetLayerCfgInfo.xRxFunId;
}

boolean CANTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID)
{
    boolean result = FALSE;

    if ((i_receiveMsgID == CANTP_GetConfigRxMsgFUNID())
            || (i_receiveMsgID == CANTP_GetConfigRxMsgPHYID()))
    {
        result = TRUE;
    }

    return result;
}

/* Get config CAN TP RX Physical Address ID */
tUdsId CANTP_GetConfigRxMsgPHYID(void)
{
    return g_stCANUdsNetLayerCfgInfo.xRxPhyId;
}

/* Get CAN TP config TX handler */
tNetTxMsg CANTP_GetConfigTxHandle(void)
{
    return g_stCANUdsNetLayerCfgInfo.pfNetTxMsg;
}

/* Get CAN TP config RX handler */
tNetRx CANTP_GetConfigRxHandle(void)
{
    return g_stCANUdsNetLayerCfgInfo.pfNetRx;
}

/* Abort CAN BUS TX message */
static void CANTP_AbortTxMsg(void)
{
    TPDebugPrintf("CANTP_AbortTxMsg\n");

    if (NULL_PTR != gs_pfCANTPAbortTxMsg)
    {
        (gs_pfCANTPAbortTxMsg)();
        gs_pfTxMsgSuccessfulCallBack = NULL_PTR;
    }

    if (TRUE != CANTP_ClearTXBUSFIFO())
    {
        TPDebugPrintf("CANTP_AbortTxMsg: Clear TX BUS FIFO failed!\n");
    }
}

/* Register abort TX message to BUS */
void CANTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg)
{
    gs_pfCANTPAbortTxMsg = (tpfAbortTxMsg)i_pfAbortTxMsg;
}

/* Write data in CAN TP */
boolean CANTP_DriverWriteDataInCANTP(const uint32 i_RxID, const uint32 i_dataLen, const uint8 *i_pDataBuf)
{
    tLen xCanWriteDataLen = 0u;
    tErroCode eStatus;
    tRxMsgInfo stRxCanMsg;
    const uint32 headerLen = sizeof(stRxCanMsg.rxDataId) + sizeof(stRxCanMsg.rxDataLen);
    ASSERT(NULL_PTR == i_pDataBuf);

    if (i_dataLen > 8u)
    {
        return FALSE;
    }

    GetCanWriteLen(RX_BUS_FIFO, &xCanWriteDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && ((i_dataLen + headerLen) <= xCanWriteDataLen))
    {
        stRxCanMsg.rxDataId = i_RxID;
        stRxCanMsg.rxDataLen = i_dataLen;
        WriteDataInFifo(RX_BUS_FIFO, (uint8 *)&stRxCanMsg, headerLen, &eStatus);

        if (ERRO_NONE != eStatus)
        {
            return FALSE;
        }

        WriteDataInFifo(RX_BUS_FIFO, (uint8 *)i_pDataBuf, stRxCanMsg.rxDataLen, &eStatus);

        if (ERRO_NONE != eStatus)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean CANTP_DriverReadDataFromCANTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader)
{
    boolean result = FALSE;
    tLen xCanRxDataLen = 0u;
    tErroCode eStatus;
    tTPTxMsgHeader TxMsgInfo;
    const uint32 msgInfoLen = sizeof(tTPTxMsgHeader);
    ASSERT(NULL_PTR == o_pReadDataBuf);
    ASSERT(NULL_PTR == o_pstTxMsgHeader);
    ASSERT(0u == i_readDataLen);
    GetCanReadLen(TX_BUS_FIFO, &xCanRxDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && (xCanRxDataLen > msgInfoLen))
    {
        ReadDataFromFifo(TX_BUS_FIFO,
                         sizeof(tTPTxMsgHeader),
                         (uint8 *)&TxMsgInfo,
                         &xCanRxDataLen,
                         &eStatus);

        if ((ERRO_NONE == eStatus) && (xCanRxDataLen == sizeof(tTPTxMsgHeader)))
        {
            result = TRUE;
        }

        if (TRUE == result)
        {
            ReadDataFromFifo(TX_BUS_FIFO,
                             i_readDataLen,
                             o_pReadDataBuf,
                             &xCanRxDataLen,
                             &eStatus);

            if ((ERRO_NONE == eStatus) && (xCanRxDataLen == i_readDataLen) && (i_readDataLen >= TxMsgInfo.TxMsgLength))
            {
                result = TRUE;
                *o_pstTxMsgHeader = TxMsgInfo;

                /* Storage callback, if user want to TX message callback please call TP_DoTxMsgSuccesfulCallback or self call callback */
                gs_pfTxMsgSuccessfulCallBack = (tpfNetTxCallBack)TxMsgInfo.TxMsgCallBack;
            }
        }
    }

    return result;
}

/* Do TX message successful callback */
void CANTP_DoTxMsgSuccessfulCallBack(void)
{
    if (NULL_PTR != gs_pfTxMsgSuccessfulCallBack)
    {
        (gs_pfTxMsgSuccessfulCallBack)();
        gs_pfTxMsgSuccessfulCallBack = NULL_PTR;
    }
}

/* Clear CAN TP TX BUS FIFO */
static boolean CANTP_ClearTXBUSFIFO(void)
{
    boolean result = FALSE;
    tErroCode eStatus = ERRO_NONE;
    ClearFIFO(TX_BUS_FIFO, &eStatus);

    if (ERRO_NONE == eStatus)
    {
        result = TRUE;
    }

    return result;
}

#endif /* EN_CAN_TP */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
