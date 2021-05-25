/*
 * @ ����: user_config.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

/*
 * @NOTE 1:
 * ������Ҫ��ӡ���ʱ��ֻ���ڴ�����ע�ͼ��ɣ�����Դ�ļ��еĺ�����������ע��
 *
 * @NOTE 2:
 * ���ø����ӡ����������;��
 * 1.ʹ��Newlib Standard
 * ���� -> C/C++ Build -> Settings -> Tool Settings -> General -> Runtime Library -> Newlib Standard
 * 2.ʹ��Newlib-nano�����-u _printf_float
 * ���� -> C/C++ Build -> Settings -> Tool Settings -> C Linker -> Miscellaneous -> Other options ����� -u _printf_float
 *
 * �Ƽ���2�֣���С��RAM��FLASHռ��
 *
 * @NOTE 3:
 * RTT_PRINTF_FLT       ͨ��printf��ӡ�����֧�ָ����������ض���RTT�����ǻ��õ����ڴ棬����һЩ
 * RTT_PRINTF           ͨ��RTTԭ��������ӡ�������֧�ָ���������ʹ�ö��ڴ棬�Ͽ�
 * RTT_SET_TERMINAL(A)  ����Ҫ��ӡ������ն˱�ţ��˺������RTT_PRINTF_FLT��RTT_PRINTF�Ⱥ���ʹ��
 * RTT_TERMINAL_OUT     ����Ҫ����RTT_SET_TERMINAL(A)����ֱ�������ָ����ŵ��ն�
 */
#if 0
#include <stdio.h>
#include "SEGGER_RTT.h"
#define RTT_INIT            SEGGER_RTT_Init
#define RTT_CFG_UP_BUFFER   SEGGER_RTT_ConfigUpBuffer
#define RTT_PRINTF_FLT      printf
#define RTT_PRINTF          SEGGER_RTT_printf
#define RTT_SET_TERMINAL(A) SEGGER_RTT_SetTerminal(A)
#define RTT_TERMINAL_OUT    SEGGER_RTT_TerminalOut
#else
#define RTT_INIT(...)
#define RTT_CFG_UP_BUFFER(...)
#define RTT_PRINTF_FLT(...)
#define RTT_PRINTF(...)
#define RTT_SET_TERMINAL(...)
#define RTT_TERMINAL_OUT(...)
#endif

/* -------------------- Core Define -------------------- */
/* This macro must >= 1 */
#define CORE_NO (1u)
#if (defined CORE_NO) && (CORE_NO < 1)
#undef CORE_NO
#define CORE_NO (1u)
#elif (!defined CORE_NO)
#define CORE_NO (1u)
#endif

/* -------------------- ASSERT and DEBUG IO/TIMER -------------------- */
#define EN_DEBUG_IO
//#define EN_DEBUG_TIMER
//#define EN_ASSERT

/* -------------------- Enable debug module -------------------- */
//#define EN_DEBUG_FLS_MODULE
//#define EN_UDS_DEBUG
//#define EN_TP_DEBUG
//#define EN_APP_DEBUG

/* -------------------- Enable debug FIFO -------------------- */
//#define EN_DEBUG_FIFO

#if (defined EN_DEBUG_FLS_MODULE) || (defined EN_UDS_DEBUG) || (defined EN_TP_DEBUG) || (defined EN_APP_DEBUG) || (defined EN_DEBUG_FIFO)
#ifndef EN_DEBUG_PRINT
/* Enable print */
#define EN_DEBUG_PRINT
#endif
#endif

/* -------------------- Enable CAN FD or not. -------------------- */
//#define EN_CAN_FD

/* -------------------- UDS Security Access Algorithm Configuration -------------------- */
#define EN_ZLG_SA_ALGORITHM         /* ʹ�������� ZCANPRO ר�ð�ȫ�����㷨 */
//#define EN_AES_SA_ALGORITHM_SW      /* Enable AES Security Access Algorithm with software */
//#define EN_AES_SA_ALGORITHM_HW      /* Enable AES Security Access Algorithm with hardware */

#define SA_ALGORITHM_SEED_LEN (16u) /* Seed Length */

/* -------------------- TP enable and define message ID -------------------- */
/* Only one TP can be enabled */
#define EN_CAN_TP
//#define EN_LIN_TP
//#define EN_ETHERNET_TP
//#define EN_OTHERS_TP

#ifdef EN_CAN_TP

#define USE_CAN_STD_ID
//#define USE_CAN_EXT_ID

/* TODO Bootloader: #01 CAN RX and TX message ID Configuration */
#if defined (USE_CAN_STD_ID)
#define RX_FUN_ADDR_ID       (0x7DFu)    /* FuncReq  - CAN TP RX function ID */
#define RX_PHY_ADDR_ID       (0x74Cu)    /* PhysReq  - CAN TP RX physical ID */
#define TX_RESP_ADDR_ID      (0x75Cu)    /* PhysResp - CAN TP TX physical ID */
#elif defined (USE_CAN_EXT_ID)
#define RX_FUN_ADDR_ID       (0x18DA5536u)    /* FuncReq  - CAN TP RX function ID */
#define RX_PHY_ADDR_ID       (0x18DA5535u)    /* PhysReq  - CAN TP RX physical ID */
#define TX_RESP_ADDR_ID      (0x18DA3555u)    /* PhysResp - CAN TP TX physical ID */
#else
#error "��ѡ���ʵ��� CAN ID ����"
#endif
#endif

#ifdef EN_LIN_TP
#define RX_BOARD_ID          (0x7Fu) /* LIN TP RX board ID -- all messages, response unexpected, but supported */
#define RX_FUN_ADDR_ID       (0x7Eu) /* LIN TP RX function ID -- don't need response/only support SF */
#define RX_PHY_ADDR_ID       (0x55u) /* LIN TP RX physical ID */
#define TX_RESP_ADDR_ID      (0x35u) /* LIN TP TX ID (master NAD ID) */
#endif

/* -------------------- CRC module selection -------------------- */
//#define DebugBootloader_NOTCRC /* Enable CRC or not */

//#define EN_CRC_HARDWARE /* Enable CRC module with hardware */
#define EN_CRC_SOFTWARE /* Enable CRC module with software */

/* -------------------- FLASH address continue or not -------------------- */
#define FALSH_ADDRESS_CONTINUE (FALSE)

/* -------------------- FIFO Configuration -------------------- */
/* RX message from BUS FIFO ID */
#define RX_BUS_FIFO         ('r')       /* RX bus FIFO */

#ifdef EN_CAN_TP
#define RX_BUS_FIFO_LEN     (300u)      /* RX BUS FIFO length */
#elif defined (EN_LIN_TP)
#define RX_BUS_FIFO_LEN     (50)        /* RX BUS FIFO length */
#else
#define RX_BUS_FIFO_LEN     (50u)       /* RX BUS FIFO length */
#endif

#ifdef EN_CAN_TP
/* TX message to BUS FIFO ID */
#define TX_BUS_FIFO         ('t')       /* RX bus FIFO */
#define TX_BUS_FIFO_LEN     (100u)      /* RX BUS FIFO length */
#elif defined (EN_LIN_TP)
/* TX message to BUS FIFO ID */
#define TX_BUS_FIFO         ('t')       /* RX bus FIFO */
#define TX_BUS_FIFO_LEN     (50u)       /* RX BUS FIFO length */
#else

#endif

/* -------------------- FOTA A/B Configuration -------------------- */
//#define EN_SUPPORT_APP_B
typedef enum
{
    APP_A_TYPE = 0u,         /* APP A type */

#ifdef EN_SUPPORT_APP_B
    APP_B_TYPE = 1u,         /* APP B type */
#endif

    APP_INVLID_TYPE = 0xFFu, /* APP invalid type */
} tAPPType;

#ifdef EN_SUPPORT_APP_B
/* Enable if newest is invalid, jump to old APP */
#define EN_NEWEST_APP_INVALID_JUMP_OLD_APP
#endif

/* -------------------- Global interrupt define -------------------- */
#define DisableAllInterrupts() INT_SYS_DisableIRQGlobal()
#define EnableAllInterrupts() INT_SYS_EnableIRQGlobal()

/* -------------------- MCU type for flash erase a sector time -------------------- */
/* MCU type for erase flash time */
#define MCU_S12Z    (1)
#define MCU_S32K14x (2)
#define MCU_S32K11x (3)

#if (defined S32K144_SERIES)
#define MCU_TYPE (MCU_S32K14x)
#elif (defined S32K118_SERIES)
#define MCU_TYPE (MCU_S32K11x)
#endif

#if (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K14x)
#define INFO_START_ADDR                 0x20006FF0u
#define REQUEST_ENTER_BOOTLOADER_ADDR   0x20006FF1u
#define DOWNLOAD_APP_SUCCESSFUL_ADDR    0x20006FF0u

#define FLASH_DRV_START_ADDR            0x1FFF8000u
#define FLASH_DRV_END_ADDR              0x1FFF8800u

#define APP_A_START_ADDR                0x00014000u
#define APP_A_END_ADDR                  0x00080000u

#define APP_B_START_ADDR                0x00080000u
#define APP_B_END_ADDR                  0x000EF000u
#endif

#if (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K11x)
#define INFO_START_ADDR                 0x200057F0u
#define REQUEST_ENTER_BOOTLOADER_ADDR   0x200057F1u
#define DOWNLOAD_APP_SUCCESSFUL_ADDR    0x200057F0u

#define FLASH_DRV_START_ADDR            0x20004FF0u
#define FLASH_DRV_END_ADDR              0x200057F0u

#define APP_A_START_ADDR                0x00014000u
#define APP_A_END_ADDR                  0x00040000u

#define APP_B_START_ADDR                0x00080000u
#define APP_B_END_ADDR                  0x000EF000u
#endif

/* -------------------- Jump to APP delay time when have not received UDS message -------------------- */
#define EN_DELAY_TIME
#define DELAY_MAX_TIME_MS (2000u)

#endif /* USER_CONFIG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
