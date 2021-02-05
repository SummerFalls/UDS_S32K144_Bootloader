/*
 * @ 名称: can_cfg.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "can_cfg.h"
#include "user_config.h"

#ifdef IsUse_CAN_Pal_Driver
#include "can_pal1.h"
#endif  //end of IsUse_CAN_Pal_Driver

#ifdef IsUse_CAN_Pal_Driver
/* Set information about the data to be receive and sent
     *  - Standard message ID
     *  - Bit rate switch enabled to use a different bitrate for the data segment
     *  - Flexible data rate enabled
     *  - Use zeros for FD padding
     */
can_buff_config_t buff_RxTx_Cfg =
{
    .enableFD = false,
    .enableBRS = true,
    .fdPadding = 0U,
    .idType = RX_FUN_ID_TYPE,
    .isRemote = false
};

/* Define receive buffer */
can_message_t recvMsg;

#else
/* Set information about the data to be receive and sent
             *  - Standard message ID
             *  - Bit rate switch enabled to use a different bitrate for the data segment
             *  - Flexible data rate enabled
             *  - Use zeros for FD padding
             */
flexcan_data_info_t buff_RxTx_Cfg =
{
    .msg_id_type = RX_FUN_ID_TYPE,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0u,
    .enable_brs = true,
    .is_remote = false,
};

/* Define receive buffer */
flexcan_msgbuff_t recvMsg;

#endif  //end of IsUse_CAN_Pal_Driver

#if 0
/*can hardware config*/
const tCANHardwareConfig g_stCANHardWareConfig =
{
    16000000,   /*can clock*/
    500u        /*can baud rate*/
};
#endif

/*RX can message config*/
const tRxMsgConfig g_astRxMsgConfig[] =
{
    {RX_MAILBOX_FUN_ID, RX_FUN_ID, RX_FUN_ID_MASK, RX_FUN_ID_TYPE}, /*UDS Rx function ID*/
    {RX_MAILBOX_PHY_ID, RX_PHY_ID, RX_PHY_ID_MASK, RX_PHY_ID_TYPE}  /*UDS RX physical ID*/
};

/*rx can message num*/
const unsigned char g_ucRxCANMsgIDNum = sizeof(g_astRxMsgConfig) / sizeof(g_astRxMsgConfig[0u]);

/*TX can message config*/
tTxMsgConfig g_stTxMsgConfig =
{
    TX_ID,     /*UDS tx can message id*/
    TX_MAILBOX_0, /*UDS tx can message mail box*/
    TX_ID_TYPE, /* UDS tx can message id type */
    NULL
};

/* -------------------------------------------- END OF FILE -------------------------------------------- */
