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
**  Description : ISO 17987-2 configuration file
*******************************************************/

typedef uint32 tUdsId;
typedef uint32 tUdsLen;
typedef unsigned short tNetTime;
typedef unsigned short tBlockSize;
typedef void (*tpfNetTxCallBack)(void);
typedef unsigned char (*tNetTxMsg)(const tUdsId, const unsigned short, const unsigned char *, const tpfNetTxCallBack, const uint32);
typedef unsigned char (*tNetRx)(tUdsId *, unsigned char *, unsigned char *);
typedef unsigned short tLINTpDataLen;
typedef void (*tpfAbortTxMsg)(void);

#define DATA_LEN (7u)

#define SF_DATA_MAX_LEN (6u)   /* Max Single Frame data len */
#define FF_DATA_MIN_LEN (7u)   /* Min First Frame data len */
#define CF_DATA_MAX_LEN (6u)   /* Single Consecutive frame max data len */
#define MAX_CF_DATA_LEN (150u) /* Max First Frame data len */

typedef struct
{
    unsigned char ucCalledPeriod; /* Called LIN TP main function period */
    tUdsId xRxBoardcastId;        /* RX broadcast ID */
    tUdsId xRxFunId;              /* RX FUN ID */
    tUdsId xRxPhyId;              /* RX PHY ID */
    tUdsId xTxId;                 /* TX RESP ID */
    tBlockSize xBlockSize;        /* BS = block size */
    tNetTime xSTmin;              /* STmin */
    tNetTime xNAs;                /* N_As */
    tNetTime xNAr;                /* N_Ar */
    tNetTime xNBs;                /* N_Bs */
    tNetTime xNBr;                /* N_Br */
    tNetTime xNCs;                /* N_Cs < 0.9 N_Cr */
    tNetTime xNCr;                /* N_Cr */
    uint32 txBlockingMaxTimeMs;   /* TX message blocking max time (MS) */
    tNetTxMsg pfNetTxMsg;         /* Net TX message with non blocking */
    tNetRx pfNetRx;               /* Net RX */
    tpfAbortTxMsg pfAbortTXMsg;   /* Abort TX message */
} tUdsLINNetLayerCfg;

/* UDS network layer config info */
extern const tUdsLINNetLayerCfg g_stUdsLINNetLayerCfgInfo;


tUdsId LINTP_GetConfigTxMsgID(void);

tUdsId LINTP_GetConfigRxMsgFUNID(void);

tUdsId LINTP_GetConfigRxMsgPHYID(void);

tNetTxMsg LINTP_GetConfigTxHandle(void);

tNetRx LINTP_GetConfigRxHandle(void);

boolean LINTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID);

tUdsId LINTP_GetConfigRxMsgBroadcastID(void);

boolean LINTP_DriverWriteDataInLINTP(const uint32 i_RxNAD, const uint32 i_dataLen, const uint8 *i_pDataBuf);

boolean LINTP_DriverReadDataFromLINTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader);

void LINTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg);

void LINTP_DoTxMsgSuccessfulCallBack(void);

#endif /* EN_LIN_TP */

#endif /* LIN_TP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
