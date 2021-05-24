/*
 * @ 名称: can_tp_cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef CAN_TP_CFG_H_
#define CAN_TP_CFG_H_

#include "includes.h"

#ifdef EN_CAN_TP

#include "TP_cfg.h"

/*******************************************************
**  Description : ISO 15765-2 configuration file
*******************************************************/

typedef uint32 tUdsId;
typedef uint32 tUdsLen;
typedef uint16 tNetTime;
typedef uint16 tBlockSize;
typedef void (*tpfNetTxCallBack)(void);
typedef uint8 (*tNetTxMsg)(const tUdsId, const uint16, const uint8 *, const tpfNetTxCallBack, const uint32);
typedef uint8 (*tNetRx)(tUdsId *, uint8 *, uint8 *);
typedef uint16 tCanTpDataLen;
typedef void (*tpfAbortTxMsg)(void);


#define DATA_LEN                (8u)

#define SF_CANFD_DATA_MAX_LEN   (62u)   /* Max CAN FD Single Frame data len */
#define SF_CAN_DATA_MAX_LEN     (7u)    /* Max CAN2.0 Single Frame data len */

#define TX_SF_DATA_MAX_LEN      (7u)    /* RX support CAN FD, TX message is not support CAN FD */

#define FF_DATA_MIN_LEN         (8u)    /* Min First Frame data len*/

#define CF_DATA_MAX_LEN         (7u)    /* Single Consecutive Frame max data len */
#define MAX_CF_DATA_LEN         (150u)  /* Max Consecutive Frame data len */

#define NORMAL_ADDRESSING (0u) /* Normal addressing */
#define MIXED_ADDRESSING  (1u) /* Mixed addressing */

typedef struct
{
    uint8 ucCalledPeriod;        /* Called CAN TP main function period */
    tUdsId xRxFunId;             /* RX FUN ID */
    tUdsId xRxPhyId;             /* RX PHY ID */
    tUdsId xTxId;                /* TX RESP ID */
    tBlockSize xBlockSize;       /* BS = block size */
    tNetTime xSTmin;             /* STmin */
    tNetTime xNAs;               /* N_As */
    tNetTime xNAr;               /* N_Ar */
    tNetTime xNBs;               /* N_Bs */
    tNetTime xNBr;               /* N_Br */
    tNetTime xNCs;               /* N_Cs < 0.9 N_Cr */
    tNetTime xNCr;               /* N_Cr */
    uint32 txBlockingMaxTimeMs;  /* TX Max blocking time(ms). > 0 mean timeout for TX. equal 0 is not waiting. */
    tNetTxMsg pfNetTxMsg;        /* Net TX message with non blocking */
    tNetRx pfNetRx;              /* Net RX */
    tpfAbortTxMsg pfAbortTXMsg;  /* Abort TX message */
} tUdsCANNetLayerCfg;


/* UDS Network layer config info */
extern const tUdsCANNetLayerCfg g_stCANUdsNetLayerCfgInfo;


tUdsId CANTP_GetConfigTxMsgID(void);

tUdsId CANTP_GetConfigRxMsgFUNID(void);

tUdsId CANTP_GetConfigRxMsgPHYID(void);

tNetTxMsg CANTP_GetConfigTxHandle(void);

tNetRx CANTP_GetConfigRxHandle(void);

boolean CANTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID);

boolean CANTP_DriverWriteDataInCANTP(const uint32 i_RxID, const uint32 i_dataLen, const uint8 *i_pDataBuf);

void CANTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg);

void CANTP_DoTxMsgSuccessfulCallBack(void);

boolean CANTP_DriverReadDataFromCANTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader);

#endif /* EN_CAN_TP*/

#endif /* CAN_TP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
