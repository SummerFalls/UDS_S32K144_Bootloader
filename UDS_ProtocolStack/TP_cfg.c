/*
 * @ 名称: TP_cfg.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "TP_cfg.h"

#ifdef EN_CAN_TP
#include "can_tp_cfg.h"
#endif

#ifdef EN_LIN_TP
#include "LIN_tp_cfg.h"
#endif

static tpfUDSTxMsgCallBack gs_pfUDSTxMsgCallBack = NULL_PTR; /* TX message callback */


/* Get TP config TX message ID */
uint32 TP_GetConfigTxMsgID(void)
{
    uint32 txMsgID = 0u;
#ifdef EN_CAN_TP
    txMsgID = CANTP_GetConfigTxMsgID();
#endif
#ifdef EN_LIN_TP
    txMsgID = LINTP_GetConfigTxMsgID();
#endif
    return txMsgID;
}

/* Get TP config receive Function ID */
uint32 TP_GetConfigRxMsgFUNID(void)
{
    uint32 rxMsgFUNID = 0u;
#ifdef EN_CAN_TP
    rxMsgFUNID = CANTP_GetConfigRxMsgFUNID();
#endif
#ifdef EN_LIN_TP
    rxMsgFUNID = LINTP_GetConfigRxMsgFUNID();
#endif
    return rxMsgFUNID;
}

/* Get TP config receive physical ID */
uint32 TP_GetConfigRxMsgPHYID(void)
{
    uint32 rxMsgPHYID = 0u;
#ifdef EN_CAN_TP
    rxMsgPHYID = CANTP_GetConfigRxMsgPHYID();
#endif
#ifdef EN_LIN_TP
    rxMsgPHYID = LINTP_GetConfigRxMsgPHYID();
#endif
    return rxMsgPHYID;
}

#ifdef EN_LIN_TP
/* Get TP config received broadcast ID */
uint32 TP_GetConfigRxMsgBoardcastID(void)
{
    uint32 rxMsgBoardcastID = 0u;
    rxMsgBoardcastID = LINTP_GetConfigRxMsgBroadcastID();
    return rxMsgBoardcastID;
}
#endif


/* Register transmit a frame message callback */
void TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack)
{
    gs_pfUDSTxMsgCallBack = (tpfUDSTxMsgCallBack)i_pfTxMsgCallBack;
}

/* Do transmitted a frame message callback */
void TP_DoTransmittedAFrameMsgCallBack(const uint8 i_result)
{
    if (NULL_PTR != gs_pfUDSTxMsgCallBack)
    {
        (gs_pfUDSTxMsgCallBack)(i_result);
        gs_pfUDSTxMsgCallBack = NULL_PTR;
    }
}

/* Driver write data in TP for read message from BUS */
boolean TP_DriverWriteDataInTP(const uint32 i_RxID, const uint32 i_RxDataLen, const uint8 *i_pRxDataBuf)
{
    boolean result = FALSE;
    ASSERT(NULL_PTR == i_pRxDataBuf);
    ASSERT(0u == i_RxDataLen);
#ifdef EN_CAN_TP
    result = CANTP_DriverWriteDataInCANTP(i_RxID, i_RxDataLen, i_pRxDataBuf);
#endif
#ifdef EN_LIN_TP
    result = LINTP_DriverWriteDataInLINTP(i_pRxDataBuf[0u], i_RxDataLen - 1u, &i_pRxDataBuf[1u]);
#endif
    return result;
}

/* Driver read data from TP for TX message to BUS */
boolean TP_DriverReadDataFromTP(const uint32 i_readDataLen, uint8 *o_pReadDatabuf, uint32 *o_pTxMsgID, uint32 *o_pTxMsgLength)
{
    boolean result = FALSE;
    tTPTxMsgHeader TPTxMsgHeader;
    ASSERT(0u == i_readDataLen);
    ASSERT(NULL_PTR == o_pReadDatabuf);
    ASSERT(NULL_PTR == o_pTxMsgID);
    ASSERT(NULL_PTR == o_pTxMsgLength);
#ifdef EN_LIN_TP
    result = LINTP_DriverReadDataFromLINTP(i_readDataLen, o_pReadDatabuf, &TPTxMsgHeader);
#endif
#ifdef EN_CAN_TP
    result = CANTP_DriverReadDataFromCANTP(i_readDataLen, o_pReadDatabuf, &TPTxMsgHeader);
#endif

    if (TRUE == result)
    {
        *o_pTxMsgID = TPTxMsgHeader.TxMsgID;
        *o_pTxMsgLength = TPTxMsgHeader.TxMsgLength;
    }

    return result;
}

/* Register abort TX message */
void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void))
{
#ifdef EN_CAN_TP
    CANTP_RegisterAbortTxMsg((const tpfAbortTxMsg)i_pfAbortTxMsg);
#endif
#ifdef EN_LIN_TP
    LINTP_RegisterAbortTxMsg((const tpfAbortTxMsg)i_pfAbortTxMsg);
#endif
}

/* Do TP TX message successful callback */
void TP_DoTxMsgSuccesfulCallback(void)
{
#ifdef EN_LIN_TP
    LINTP_DoTxMsgSuccessfulCallBack();
#endif
#ifdef EN_CAN_TP
    CANTP_DoTxMsgSuccessfulCallBack();
#endif
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
