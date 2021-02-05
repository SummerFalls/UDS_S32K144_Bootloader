/*
 * @ 名称: LIN_tp_cfg.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "LIN_tp_cfg.h"

#ifdef EN_LIN_TP

#include "multi_cyc_fifo.h"

static tpfAbortTxMsg gs_pfLINTPAbortTxMsg = NULL_PTR;
static tpfNetTxCallBack gs_pfTxMsgSuccessfulCallBack = NULL_PTR;

static uint8 LINTP_TxMsg(const tUdsId i_xTxId,
                              const uint16 i_DataLen,
                              const uint8* i_pDataBuf,
                              const tpfNetTxCallBack i_pfNetTxCallBack,
                              const uint32 txBlockingMaxtime);

static uint8 LINTP_RxMsg(tUdsId * o_pxRxId,
                               uint8 * o_pRxDataLen,
                               uint8 *o_pRxBuf);

static void LINTP_AbortTxMsg(void);

/*clear LIN TP TX BUS FIFO*/
static boolean LINTP_ClearTXBUSFIFO(void);


/*uds netwrok layer cfg info */
const tUdsLINNetLayerCfg g_stUdsLINNetLayerCfgInfo =
{
    1u,       /*called LIN tp period*/
    RX_BOARD_ID, /*LIN TP rx boardcast IDs*/
    RX_FUN_ID,   /*LIN tp rx function ID*/
    RX_PHY_ID,   /*LIN tp rx phy ID*/
    TX_ID,   /*LIN tp tx ID*/
    0u,       /*BS = block size*/
    0u,      /*STmin*/
    300u,      /*N_As*/
    300u,      /*N_Ar*/
    300u,     /*N_Bs*/
    0u,       /*N_Br*/
    300u,       /*N_Cs*/
    500u,     /*N_Cr*/
    0u,       /*max blocking time 0ms, > 0u mean waitting send successful. equal 0 is not waitting.*/
    LINTP_TxMsg, /*LIN tp tx*/
    LINTP_RxMsg, /*LIN tp rx*/
    LINTP_AbortTxMsg,   /*abort tx message*/
};


/*LIN tp tx message: there not use CAN driver TxFIFO, directly invoked CAN send function*/
static uint8 LINTP_TxMsg(const tUdsId i_xTxId,
                              const uint16 i_DataLen,
                              const uint8* i_pDataBuf,
                              const tpfNetTxCallBack i_pfNetTxCallBack,
                              const uint32 txBlockingMaxtime)
{
    tLen xCanWriteDataLen = 0u;
    tErroCode eStatus;
    uint8 aMsgBuf[8] = {0};
    tTPTxMsgHeader TxMsgInfo;
    const uint32 msgInfoLen = sizeof(tTPTxMsgHeader) + sizeof(aMsgBuf);

    ASSERT(NULL_PTR == i_pDataBuf);

    if(i_DataLen > 7u)
    {
        return FALSE;
    }

    GetCanWriteLen(TX_BUS_FIFO, &xCanWriteDataLen, &eStatus);
    if((ERRO_NONE == eStatus) && (msgInfoLen <= xCanWriteDataLen))
    {
        TxMsgInfo.TxMsgID = i_xTxId;
        TxMsgInfo.TxMsgLength = sizeof(aMsgBuf);
        TxMsgInfo.TxMsgCallBack = (uint32)i_pfNetTxCallBack;

        aMsgBuf[0u] = (uint8)i_xTxId;
        fsl_memcpy(&aMsgBuf[1u], i_pDataBuf, i_DataLen);

        WriteDataInFifo(TX_BUS_FIFO, (uint8 *)&TxMsgInfo, sizeof(tTPTxMsgHeader),&eStatus);
        if(ERRO_NONE != eStatus)
        {
            ClearFIFO(TX_BUS_FIFO, &eStatus);

            return FALSE;
        }

        WriteDataInFifo(TX_BUS_FIFO, (uint8 *)aMsgBuf,8,&eStatus);
        if(ERRO_NONE != eStatus)
        {
            ClearFIFO(TX_BUS_FIFO, &eStatus);
            return FALSE;
        }
    }

    return TRUE;
}

/*LIN tp rx message: read rx msg from CAN driver RxFIFO*/
static uint8 LINTP_RxMsg(tUdsId * o_pxRxId,
                         uint8 * o_pRxDataLen,
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
    if((ERRO_NONE == eStatus) && (headerLen <= xCanRxDataLen))
    {
        ReadDataFromFifo(RX_BUS_FIFO,
                         headerLen,
                        (uint8 *)&stRxCanMsg,
                        &xReadDataLen,
                        &eStatus);

        if((ERRO_NONE == eStatus) && (headerLen <= xCanRxDataLen))
        {
            ReadDataFromFifo(RX_BUS_FIFO,
                             stRxCanMsg.rxDataLen,
                            (uint8 *)&stRxCanMsg.aucDataBuf,
                            &xCanRxDataLen,
                            &eStatus);


            if(TRUE != LINTP_IsReceivedMsgIDValid(stRxCanMsg.rxDataId))
            {
                return FALSE;
            }

            *o_pxRxId = stRxCanMsg.rxDataId;
            *o_pRxDataLen = stRxCanMsg.rxDataLen;

            for(ucIndex = 0u; ucIndex < stRxCanMsg.rxDataLen; ucIndex++)
            {
                o_pRxBuf[ucIndex] = stRxCanMsg.aucDataBuf[ucIndex];
            }


            return TRUE;
         }

    }

    return FALSE;
}

/*write data in LIN TP*/
boolean LINTP_DriverWriteDataInLINTP(const uint32 i_RxNAD, const uint32 i_dataLen, const uint8 *i_pDataBuf)
{
    tLen xCanWriteDataLen = 0u;
    tErroCode eStatus;
    tRxMsgInfo stRxCanMsg;
    const uint32 headerLen = sizeof(stRxCanMsg.rxDataId) + sizeof(stRxCanMsg.rxDataLen);

    ASSERT(NULL_PTR == i_pDataBuf);

    if(i_dataLen > 7u)
    {
        return FALSE;
    }

    GetCanWriteLen(RX_BUS_FIFO, &xCanWriteDataLen, &eStatus);
    if((ERRO_NONE == eStatus) && ((i_dataLen + headerLen) <= xCanWriteDataLen))
    {
        stRxCanMsg.rxDataId = i_RxNAD;
        stRxCanMsg.rxDataLen = i_dataLen;

        WriteDataInFifo(RX_BUS_FIFO, (uint8 *)&stRxCanMsg, headerLen, &eStatus);
        if(ERRO_NONE != eStatus)
        {
            return FALSE;
        }

        WriteDataInFifo(RX_BUS_FIFO, (uint8 *)i_pDataBuf, stRxCanMsg.rxDataLen, &eStatus);
        if(ERRO_NONE != eStatus)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*Driver read data from LINTP*/
boolean LINTP_DriverReadDataFromLINTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader)
{
    boolean result = FALSE;
    tLen xCanRxDataLen = 0u;
    tErroCode eStatus;
    tTPTxMsgHeader TxMsgInfo;
    const uint32 msgInfoLen = sizeof(tTPTxMsgHeader);

    ASSERT(NULL_PTR == o_pReadDataBuf);
    ASSERT(NULL_PTR == o_pstTxMsgHeader);
    ASSERT(8u != i_readDataLen);

    GetCanReadLen(TX_BUS_FIFO, &xCanRxDataLen, &eStatus);
    if((ERRO_NONE == eStatus) && (xCanRxDataLen >= (i_readDataLen + msgInfoLen)))
    {
            /*read header an*/
            ReadDataFromFifo(TX_BUS_FIFO,
                             sizeof(tTPTxMsgHeader),
                             (uint8 *)&TxMsgInfo,
                            &xCanRxDataLen,
                            &eStatus);
            if((ERRO_NONE == eStatus) && (xCanRxDataLen == sizeof(tTPTxMsgHeader)))
            {
                result = TRUE;
            }

            if(TRUE == result)
            {
                ReadDataFromFifo(TX_BUS_FIFO,
                                 i_readDataLen,
                                o_pReadDataBuf,
                                &xCanRxDataLen,
                                &eStatus);
                if((ERRO_NONE == eStatus) && (xCanRxDataLen == i_readDataLen))
                {
                    result = TRUE;

                    *o_pstTxMsgHeader = TxMsgInfo;

                    /*storage callback, if user want to TX message callback please call TP_DoTxMsgSuccesfulCallback or self call callback*/
                    gs_pfTxMsgSuccessfulCallBack = (tpfNetTxCallBack)TxMsgInfo.TxMsgCallBack;
                }
            }
    }

    return result;
}

/*clear LIN TP TX BUS FIFO*/
static boolean LINTP_ClearTXBUSFIFO(void)
{
    boolean result = FALSE;
    tErroCode eStatus = ERRO_NONE;

    ClearFIFO(TX_BUS_FIFO, &eStatus);
    if(ERRO_NONE == eStatus)
    {
        result = TRUE;
    }

    return result;
}

/*get config LIN TP tx ID*/
tUdsId LINTP_GetConfigTxMsgID(void)
{
    return g_stUdsLINNetLayerCfgInfo.xTxId;
}

/*get config LIN TP recevie function message ID*/
tUdsId LINTP_GetConfigRxMsgFUNID(void)
{
    return g_stUdsLINNetLayerCfgInfo.xRxFunId;
}


/*get config LIN TP recevie physical message ID*/
tUdsId LINTP_GetConfigRxMsgPHYID(void)
{
    return g_stUdsLINNetLayerCfgInfo.xRxPhyId;

}

/*get config LIN TP recevie broadcast message ID*/
tUdsId LINTP_GetConfigRxMsgBroadcastID(void)
{
    return g_stUdsLINNetLayerCfgInfo.xRxBoardcastId;
}

/*Get LIN TP config Tx  handle*/
tNetTxMsg LINTP_GetConfigTxHandle(void)
{
    return g_stUdsLINNetLayerCfgInfo.pfNetTxMsg;
}

/*Get LIN TP config Rx  handle*/
tNetRx LINTP_GetConfigRxHandle(void)
{
    return g_stUdsLINNetLayerCfgInfo.pfNetRx;
}

/*Is received message valid?*/
boolean LINTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID)
{
    boolean result = FALSE;

    if((i_receiveMsgID == LINTP_GetConfigRxMsgFUNID())

        || (i_receiveMsgID == LINTP_GetConfigRxMsgPHYID())
        || (i_receiveMsgID == LINTP_GetConfigRxMsgBroadcastID()))
    {
        result = TRUE;
    }

    return result;
}

/*abort LIN BUS tx message*/
static void LINTP_AbortTxMsg(void)
{
    if(NULL_PTR != gs_pfLINTPAbortTxMsg)
    {
        (gs_pfLINTPAbortTxMsg)();

        /*clear call back*/
        gs_pfTxMsgSuccessfulCallBack = NULL_PTR;
    }

    /*Read all data from TX BUS FIFO*/
    if(TRUE != LINTP_ClearTXBUSFIFO())
    {
        TPDebugPrintf("AbortTxMsg: Clear TX BUS FIFO failed!\n");
    }
}

/*register abort tx message to BUS*/
void LINTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg)
{
    gs_pfLINTPAbortTxMsg = (tpfAbortTxMsg)i_pfAbortTxMsg;
}

/*do tx message successful callback*/
void LINTP_DoTxMsgSuccessfulCallBack(void)
{
    if(NULL_PTR != gs_pfTxMsgSuccessfulCallBack)
    {
        (gs_pfTxMsgSuccessfulCallBack)();

        gs_pfTxMsgSuccessfulCallBack = NULL_PTR;
    }
}
#endif /*#ifdef EN_LIN_TP*/

/* -------------------------------------------- END OF FILE -------------------------------------------- */
