/*
 * @ 名称: LIN_tp_cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef LIN_TP_CFG_H_
#define LIN_TP_CFG_H_

#include "includes.h"

#ifdef EN_LIN_TP
#include "TP_cfg.h"

/*******************************************************
**  Description : ISO 17987-2 config file
*******************************************************/
typedef uint32 tUdsId;
typedef uint32 tUdsLen;
typedef unsigned short tNetTime;
typedef unsigned short tBlockSize;
typedef void (*tpfNetTxCallBack)(void);
typedef unsigned char (*tNetTxMsg)(const tUdsId, const unsigned short, const unsigned char *, const tpfNetTxCallBack, const uint32);
typedef unsigned char (*tNetRx)(tUdsId *, unsigned char *, unsigned char *);
typedef unsigned short tLINTpDataLen;

/*abort tx message*/
typedef void (*tpfAbortTxMsg)(void);

#define DATA_LEN (7u)

#define SF_DATA_MAX_LEN (6u)  /*max signle frame data len*/
#define FF_DATA_MIN_LEN (7u)  /*min fiirst frame data len*/

#define CF_DATA_MAX_LEN (6u)  /*signle conective frame max data len*/

#define MAX_CF_DATA_LEN (150u) /*max first frame data len */

typedef struct
{
    unsigned char ucCalledPeriod;/*called LIN tp main function period*/
    tUdsId xRxBoardcastId;           /*rx boardcast ID*/
    tUdsId xRxFunId;             /*rx function ID*/
    tUdsId xRxPhyId;             /*Rx phy ID*/
    tUdsId xTxId;                /*Tx ID*/
    tBlockSize xBlockSize;       /*BS*/
    tNetTime xSTmin;             /*STmin*/
    tNetTime xNAs;               /*N_As*/
    tNetTime xNAr;               /*N_Ar*/
    tNetTime xNBs;               /*N_Bs*/
    tNetTime xNBr;               /*N_Br*/
    tNetTime xNCs;               /*N_Cs*/
    tNetTime xNCr;               /*N_Cr*/
    uint32 txBlockingMaxTimeMs;  /*TX message blocking max time (MS)*/
    tNetTxMsg pfNetTxMsg;/*net tx message with non blocking*/
    tNetRx pfNetRx;              /*net rx*/
    tpfAbortTxMsg pfAbortTXMsg;  /*abort tx message*/
}tUdsLINNetLayerCfg;

/*uds netwrok layer cfg info */
extern const tUdsLINNetLayerCfg g_stUdsLINNetLayerCfgInfo;


/*get config LIN TP tx ID*/
extern tUdsId LINTP_GetConfigTxMsgID(void);


/*get config LIN TP recevie function message ID*/
extern tUdsId LINTP_GetConfigRxMsgFUNID(void);

/*get config LIN TP recevie physical message ID*/
extern tUdsId LINTP_GetConfigRxMsgPHYID(void);

/*Get LIN TP config Tx  handle*/
extern tNetTxMsg LINTP_GetConfigTxHandle(void);

/*Get LIN TP config Rx  handle*/
extern tNetRx LINTP_GetConfigRxHandle(void);

/*Is received message valid?*/
extern boolean LINTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID);

/*get config LIN TP recevie broadcast message ID*/
extern tUdsId LINTP_GetConfigRxMsgBroadcastID(void);

/*write data in LIN TP*/
extern boolean LINTP_DriverWriteDataInLINTP(const uint32 i_RxNAD, const uint32 i_dataLen, const uint8 *i_pDataBuf);

/*Driver read data from LINTP*/
extern boolean LINTP_DriverReadDataFromLINTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader);

/*register abort tx message to BUS*/
extern void LINTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg);

/*do tx message successful callback*/
extern void LINTP_DoTxMsgSuccessfulCallBack(void);

#endif /*#ifdef EN_LIN_TP*/

#endif /* LIN_TP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
