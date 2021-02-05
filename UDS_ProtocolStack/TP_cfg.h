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

/*tx message call back*/
typedef void (*tpfUDSTxMsgCallBack)(uint8);

/*a signle message buf len*/
#define MAX_MESSAGE_LEN (64u)

typedef struct {
    uint32 rxDataLen;      /*RX can harware data len*/
    uint32 rxDataId;      /*RX data len*/
    uint8 aucDataBuf[MAX_MESSAGE_LEN];   /*RX data buf*/
} tRxMsgInfo;

typedef struct {
    uint32 msgID;                   /*message ID*/
    uint32 dataLen;                 /*data length*/
    tpfUDSTxMsgCallBack pfCallBack; /*call back*/
} tUDSAndTPExchangeMsgInfo;

#define RX_TP_QUEUE_ID ('R')  /*TP RX  FIFO ID*/
#define TX_TP_QUEUE_ID ('T')  /*TP TX  FIFO ID*/

/*defined FIFO length*/
#define TX_TP_QUEUE_LEN (50u)  /*UDS send message to  TP max length*/
#define RX_TP_QUEUE_LEN (150)  /*UDS read message from TP max length*/

typedef enum {
    TX_MSG_SUCCESSFUL = 0u,
    TX_MSG_FAILD,
    TX_MSG_TIMEOUT
} tTxMsgStatus;

typedef struct {
    uint32 TxMsgID;       /*Tx message ID*/
    uint32 TxMsgLength;   /*TX message length*/
    uint32 TxMsgCallBack; /*Tx message callback*/
} tTPTxMsgHeader;


/*Get TP config TX message ID*/
extern uint32 TP_GetConfigTxMsgID(void);

/*Get TP config recieve Funcation ID*/
extern uint32 TP_GetConfigRxMsgFUNID(void);

/*Get TP config receive pyhiscal ID*/
extern uint32 TP_GetConfigRxMsgPHYID(void);

#ifdef EN_LIN_TP
/*Get TP config received boardcast ID*/
extern uint32 TP_GetConfigRxMsgBoardcastID(void);
#endif

/*register transmit a frame message call back*/
extern void TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack);

/*do transmited a frame message call back*/
extern void TP_DoTransmittedAFrameMsgCallBack(const uint8 i_result);

/*Driver write data in TP*/
extern boolean TP_DriverWriteDataInTP(const uint32 i_RxID, const uint32 i_RxDataLen, const uint8 *i_pRxDataBuf);

/*Driver read data from TP for Tx message to BUS*/
extern boolean TP_DriverReadDataFromTP(const uint32 i_readDataLen, uint8 *o_pReadDatabuf, uint32 *o_pTxMsgID, uint32 *o_pTxMsgLength);

/*register abort tx message*/
extern void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void));

/*do TP tx message successful callback*/
extern void TP_DoTxMsgSuccesfulCallback(void);

#endif /* TP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
