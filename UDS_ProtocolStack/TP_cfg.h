/*
 * @ 名称: TP_cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef TP_CFG_H_
#define TP_CFG_H_

#include "includes.h"

/* TX message callback */
typedef void (*tpfUDSTxMsgCallBack)(uint8);

/* Single message buffer len */
#define MAX_MESSAGE_LEN (64u)

typedef struct
{
    uint32 rxDataLen;                   /* RX CAN hardware data len */
    uint32 rxDataId;                    /* RX data len */
    uint8 aucDataBuf[MAX_MESSAGE_LEN];  /* RX data buffer */
} tRxMsgInfo;

typedef struct
{
    uint32 msgID;                   /* Message ID */
    uint32 dataLen;                 /* Data length */
    tpfUDSTxMsgCallBack pfCallBack; /* Callback */
} tUDSAndTPExchangeMsgInfo;

#define RX_TP_QUEUE_ID ('R')   /* TP RX FIFO ID */
#define TX_TP_QUEUE_ID ('T')   /* TP TX FIFO ID */

/* Define FIFO length */
#define TX_TP_QUEUE_LEN (50u)  /* UDS send message to TP max length */
#define RX_TP_QUEUE_LEN (150)  /* UDS read message from TP max length */

typedef enum
{
    TX_MSG_SUCCESSFUL = 0u,
    TX_MSG_FAILD,
    TX_MSG_TIMEOUT
} tTxMsgStatus;

typedef struct
{
    uint32 TxMsgID;       /* TX message ID */
    uint32 TxMsgLength;   /* TX message length */
    uint32 TxMsgCallBack; /* TX message callback */
} tTPTxMsgHeader;

uint32 TP_GetConfigTxMsgID(void);

uint32 TP_GetConfigRxMsgFUNID(void);

uint32 TP_GetConfigRxMsgPHYID(void);

#ifdef EN_LIN_TP
uint32 TP_GetConfigRxMsgBoardcastID(void);
#endif

void TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack);

void TP_DoTransmittedAFrameMsgCallBack(const uint8 i_result);

boolean TP_DriverWriteDataInTP(const uint32 i_RxID, const uint32 i_RxDataLen, const uint8 *i_pRxDataBuf);

boolean TP_DriverReadDataFromTP(const uint32 i_readDataLen, uint8 *o_pReadDatabuf, uint32 *o_pTxMsgID, uint32 *o_pTxMsgLength);

void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void));

void TP_DoTxMsgSuccesfulCallback(void);

#endif /* TP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
