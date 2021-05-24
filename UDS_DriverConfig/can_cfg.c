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

#ifdef EN_CAN_TP

#ifdef IsUse_CAN_Pal_Driver
#include "can_pal1.h"

can_buff_config_t buff_RxTx_Cfg =
{
    .enableFD = false,
    .enableBRS = false,
    .fdPadding = 0U,
    .idType = RX_FUN_ADDR_ID_TYPE,
    .isRemote = false
};

can_message_t recvMsg;

#else

flexcan_data_info_t buff_RxTx_Cfg =
{
    .msg_id_type = RX_FUN_ADDR_ID_TYPE,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0u,
    .enable_brs = false,
    .is_remote = false,
};

flexcan_msgbuff_t recvMsg;

#endif /* IsUse_CAN_Pal_Driver */

#if 0
const tCANHardwareConfig g_stCANHardWareConfig =
{
    16000000,   /* CAN clock */
    500u        /* CAN baud rate */
};
#endif

const tRxMsgConfig g_astRxMsgConfig[] =
{
    {RX_FUN_ADDR_ID_MAILBOX, RX_FUN_ADDR_ID, RX_FUN_ADDR_ID_MASK, RX_FUN_ADDR_ID_TYPE},
    {RX_PHY_ADDR_ID_MAILBOX, RX_PHY_ADDR_ID, RX_PHY_ADDR_ID_MASK, RX_PHY_ADDR_ID_TYPE}
};

const unsigned char g_ucRxCANMsgIDNum = sizeof(g_astRxMsgConfig) / sizeof(g_astRxMsgConfig[0u]);

tTxMsgConfig g_stTxMsgConfig =
{
    TX_RESP_ADDR_ID,
    TX_RESP_ADDR_ID_MAILBOX,
    TX_RESP_ADDR_ID_TYPE,
    NULL
};

#endif /* EN_CAN_TP */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
