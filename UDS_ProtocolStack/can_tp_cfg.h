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
**  Description : ISO 15765-2 config file
*******************************************************/
typedef uint32 tUdsId;
typedef uint32 tUdsLen;
typedef uint16 tNetTime;
typedef uint16 tBlockSize;
typedef void (*tpfNetTxCallBack)(void);
typedef uint8 (*tNetTxMsg)(const tUdsId, const uint16, const uint8 *, const tpfNetTxCallBack, const uint32);
typedef uint8 (*tNetRx)(tUdsId *, uint8 *, uint8 *);
typedef uint16 tCanTpDataLen;

/*abort tx message*/
typedef void (*tpfAbortTxMsg)(void);


#define MAX_CF_DATA_LEN (150u) /*max first frame data len */

#define DATA_LEN (8u)

#define SF_CANFD_DATA_MAX_LEN (62u)  /*max CAN FD signle frame data len*/
#define SF_CAN_DATA_MAX_LEN (7u)    /*max CAN2.0 signle frame data len*/

#define TX_SF_DATA_MAX_LEN (7u) /*RX support CAN FD, TX message is unspport CAN FD */

#define FF_DATA_MIN_LEN (8u)  /*min fiirst frame data len*/

#define CF_DATA_MAX_LEN (7u)  /*signle conective frame max data len*/

#define CAN_DL (8u)           /*CAN message length for CAN TP*/

#define NORMAL_ADDRESSING (0u) /*normal addressing*/
#define MIXED_ADDRESSING (1u)  /*mixed addressing*/

typedef struct {
    uint8 ucCalledPeriod;/*called CAN tp main function period*/
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
} tUdsCANNetLayerCfg;


/*uds netwrok layer cfg info */
extern const tUdsCANNetLayerCfg g_stCANUdsNetLayerCfgInfo;


/*get config CAN TP tx ID*/
extern tUdsId CANTP_GetConfigTxMsgID(void);


/*get config CAN TP recevie function message ID*/
extern tUdsId CANTP_GetConfigRxMsgFUNID(void);

/*get config CAN TP recevie physical message ID*/
extern tUdsId CANTP_GetConfigRxMsgPHYID(void);

/*Get CAN TP config Tx  handle*/
extern tNetTxMsg CANTP_GetConfigTxHandle(void);

/*Get CAN TP config Rx  handle*/
extern tNetRx CANTP_GetConfigRxHandle(void);

/*Is received message valid?*/
extern boolean CANTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID);

/*write data in CAN TP*/
extern boolean CANTP_DriverWriteDataInCANTP(const uint32 i_RxID, const uint32 i_dataLen, const uint8 *i_pDataBuf);

/*register abort tx message to BUS*/
extern void CANTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg);

/*do tx message successful callback*/
extern void CANTP_DoTxMsgSuccessfulCallBack(void);

/*Driver read data from LINTP*/
extern boolean CANTP_DriverReadDataFromCANTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader);

#endif /*#ifdef EN_CAN_TP*/

#endif /* CAN_TP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
