/*
 * @ 名称: can_cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef CAN_CFG_H_
#define CAN_CFG_H_

#include "cpu.h"
#include "user_config.h"

#ifdef EN_CAN_TP

//#define CAN_DRIVER_DEBUG

#ifdef CAN_DRIVER_DEBUG
#include "bootloader_debug.h"
#endif

#define CAN_ERRO_INTERRUPUT     (0u)
#define CAN_ERRO_POLLING        (1u)

#define CAN_WAKE_UP_INTERRUPT   (0u)
#define CAN_WAKE_UP_POLLING     (1u)

#define USE_CAN_ERRO            (CAN_ERRO_POLLING)
#define USE_CAN_WAKE_UP         (CAN_WAKE_UP_POLLING)

/* CAN RX and TX ID type, ID mask configuration */
#if defined (USE_CAN_STD_ID)
#define RX_FUN_ADDR_ID_TYPE  FLEXCAN_MSG_ID_STD
#define RX_FUN_ADDR_ID_MASK  0xFFFFFFFFu

#define RX_PHY_ADDR_ID_TYPE  FLEXCAN_MSG_ID_STD
#define RX_PHY_ADDR_ID_MASK  0xFFFFFFFFu

#define TX_RESP_ADDR_ID_TYPE FLEXCAN_MSG_ID_STD
#elif defined (USE_CAN_EXT_ID)
#define RX_FUN_ADDR_ID_TYPE  FLEXCAN_MSG_ID_EXT
#define RX_FUN_ADDR_ID_MASK  0xFFFFFFFFu

#define RX_PHY_ADDR_ID_TYPE  FLEXCAN_MSG_ID_EXT
#define RX_PHY_ADDR_ID_MASK  0xFFFFFFFFu

#define TX_RESP_ADDR_ID_TYPE FLEXCAN_MSG_ID_EXT
#endif

/* RX and TX mailbox number configuration */
#define RX_FUN_ADDR_ID_MAILBOX  (1u)
#define RX_PHY_ADDR_ID_MAILBOX  (2u)
#define TX_RESP_ADDR_ID_MAILBOX (3u)

#ifdef CAN_DRIVER_DEBUG
#define CANDebugPrintf DebugPrintf
#else
#define CANDebugPrintf(...)
#endif


typedef void (*tpfTxSuccesfullCallBack)(void);

#if 0
/* CAN hardware configuration */
typedef struct
{
    unsigned long ulCANClock;     /* CAN clock unit MHz */
    unsigned short usCANBaudRate; /* CAN baud rate unit KHz */
} tCANHardwareConfig;
#endif

typedef struct
{
    uint32_t usTxID;                        /* TX CAN ID */
    uint8_t ucTxMailBox;                    /* used TX mailbox */
    flexcan_msgbuff_id_type_t TxID_Type;    /* RX mask ID type: Standard ID or Extended ID */
    tpfTxSuccesfullCallBack pfCallBack;
} tTxMsgConfig;

typedef struct
{
    uint8_t usRxMailBox;                    /* used RX mailbox */
    uint32_t usRxID;                        /* RX CAN ID */
    uint32_t usRxMask;                      /* RX mask */
    flexcan_msgbuff_id_type_t RxID_Type;    /* RX mask ID type: Standard ID or Extended ID */
} tRxMsgConfig;

/* TODO Bootloader: #03 use SDK2.0 CAN PAL or FlexCAN Driver */
//#define IsUse_CAN_Pal_Driver

#ifdef IsUse_CAN_Pal_Driver

extern can_buff_config_t buff_RxTx_Cfg;
extern can_message_t recvMsg;

#else

extern flexcan_data_info_t buff_RxTx_Cfg;
extern flexcan_msgbuff_t recvMsg;

#endif /* IsUse_CAN_Pal_Driver */

#if 0
extern const tCANHardwareConfig g_stCANHardWareConfig;
#endif

extern const tRxMsgConfig g_astRxMsgConfig[];
extern const uint8_t g_ucRxCANMsgIDNum;
extern tTxMsgConfig g_stTxMsgConfig;

#endif /* EN_CAN_TP */

#endif /* CAN_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
