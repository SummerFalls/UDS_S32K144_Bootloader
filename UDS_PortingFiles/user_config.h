/*
 * @ 名称: user_config.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

/****************************core define ***********************/
/*the macro must >=1*/
#define CORE_NO (1u)
#if (defined CORE_NO) && (CORE_NO < 1)
#undef CORE_NO
#define CORE_NO (1u)
#elif (!defined CORE_NO)
#define CORE_NO (1u)
#endif
/************************************************************/

/****************************ASSERT and DEBUG IO/TIMER**********/
/*Enable debug IO*/
#define EN_DEBUG_IO

/*Enable debug timer*/
//#define EN_DEBUG_TIMER

/*define assert*/
#define EN_ASSERT

/*enable debug flash module*/
#define EN_DEBUG_FLS_MODULE
#define EN_UDS_DEBUG
#define EN_TP_DEBUG
#define EN_APP_DEBUG

/*used CRC or not*/
//#define DebugBootloader_NOTCRC

/*enable debug FIFO*/
//#define EN_DEBUG_FIFO

/*enable CAN FD or not.*/
//#define EN_CAN_FD

#if (defined EN_DEBUG_FLS_MODULE) || (defined EN_UDS_DEBUG) || (defined EN_TP_DEBUG) || (defined EN_APP_DEBUG) || (defined EN_DEBUG_FIFO)
#ifndef EN_DEBUG_PRINT
/*Enable print*/
#define EN_DEBUG_PRINT
#endif
#else
/*Enable print or not*/
//#define EN_DEBUG_PRINT
#endif
/************************************************************/

/************************UDS algthorim access config***************/
/*enable algthorim access with software*/
#define EN_ALG_SW

/*enable algthorim access with hardware*/
#define EN_ALG_HW

/*AES seed length*/
#define AES_SEED_LEN (16u)

/************************************************************/

/******************TP enable and define message ID****************/
/*only one TP can enable, if multi TP enable will trigger error!*/
#define EN_CAN_TP
//#define EN_LIN_TP
//#define EN_ETHERNET_TP
//#define EN_OTHERS_TP

#ifdef EN_CAN_TP
/* TODO Bootloader: #01 CAN Rx and Tx message ID configure */
#if 1
#define RX_FUN_ID       (0x7DFu)    /* FuncReq  - CAN TP RX function ID */
#define RX_PHY_ID       (0x74Cu)    /* PhysReq  - CAN TP RX physical ID */
#define TX_ID           (0x75Cu)    /* PhysResp - CAN TP TX physical ID */
#else
#define RX_FUN_ID       (0x18da5536u)    /* FuncReq  - CAN TP RX function ID */
#define RX_PHY_ID       (0x18da5535u)    /* PhysReq  - CAN TP RX physical ID */
#define TX_ID           (0x18da3555u)    /* PhysResp - CAN TP TX physical ID */
#endif
#endif  //end of EN_CAN_TP

#ifdef EN_LIN_TP
#define RX_BOARD_ID (0x7Fu) /*LIN TP rx board ID -- all messages, response unexpected, but supported*/
#define RX_FUN_ID (0x7Eu)   /*LIN tp rx function ID -- don't need  response/only support SF*/
#define RX_PHY_ID (0x55u)   /*LIN tp rx phy ID*/
#define TX_ID (0x35u)       /*LIN tp tx ID (master NAD ID)*/
#endif
/***********************************************************/

/***********************CRC config****************************/
/*enable CRC module with hardware*/
//#define EN_CRC_HARDWARE

/*enable CRC module with software*/
#define EN_CRC_SOFTWARE

/*FLASH address continue or not*/
#define FALSH_ADDRESS_CONTINUE (FALSE)
/***********************************************************/

/********************FIFO define*******************************/
/*RX message from BUS FIFO ID*/
#define RX_BUS_FIFO        ('r')  /*RX bus fifo*/

#ifdef EN_CAN_TP
#define RX_BUS_FIFO_LEN (300u)     /*RX BUS FIFO length*/
#elif (defined EN_LIN_TP)
#define RX_BUS_FIFO_LEN (50)      /*RX BUS FIFO length*/
#else
#define RX_BUS_FIFO_LEN (50u)     /*RX BUS FIFO length*/
#endif

#ifdef EN_CAN_TP
/*TX message to BUS FIFO ID*/
#define TX_BUS_FIFO        ('t')  /*RX bus fifo*/
#define TX_BUS_FIFO_LEN (100u)     /*RX BUS FIFO length*/
#elif (defined EN_LIN_TP)
/*TX message to BUS FIFO ID*/
#define TX_BUS_FIFO        ('t')  /*RX bus fifo*/
#define TX_BUS_FIFO_LEN (50u)     /*RX BUS FIFO length*/
#else

#endif
/***********************************************************/

/**********************FOTA A/B config************************/
//#define EN_SUPPORT_APP_B
typedef enum
{
    APP_A_TYPE = 0u,         /*APP A type*/

#ifdef EN_SUPPORT_APP_B
    APP_B_TYPE = 1u,         /*APP B type*/
#endif

    APP_INVLID_TYPE = 0xFFu, /*APP invalid type*/
}tAPPType;

#ifdef EN_SUPPORT_APP_B
/*enable,  newest is invalid, jump to old app*/
#define EN_NEWEST_APP_INVALID_JUMP_OLD_APP
#endif
/***********************************************************/

/*******************Global interrupt define************************/
/*disable all interrupts*/
#define DisableAllInterrupts() INT_SYS_DisableIRQGlobal()

/*enable all interrupts*/
#define EnableAllInterrupts() INT_SYS_EnableIRQGlobal()
/***********************************************************/

/*******************MCU type for flash erase a sector time***********/
/*MCU type for erase flash time*/
#define MCU_S12Z (1)
#define MCU_S32K14x (2)

#define MCU_TYPE (MCU_S32K14x)
/***********************************************************/

/******Jump to APP delay time when have not received uds message*******/
#define EN_DELAY_TIME
#define DELAY_MAX_TIME_MS (5000u)

/***********************************************************/

#endif /* USER_CONFIG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
