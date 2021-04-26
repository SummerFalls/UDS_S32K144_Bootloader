/*
 * @ 名称: flash_cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef FLASH_CFG_H_
#define FLASH_CFG_H_

#include "cpu.h"

//#define USE_FLASH_DRIVER
#define EN_DISABAL_INTERRUPT
//typedef unsigned long tLogicalAddr;
//
//typedef struct
//{
//  tLogicalAddr xBlockStartLogicalAddr; /*block start logical addr*/
//  tLogicalAddr xBlockEndLogicalAddr;   /*block end logical addr*/
//}tBlockInfo;


/*every program flash size*/
#define PROGRAM_SIZE (128u)

//#define STARTUP_ADDR (0x00000000u) /*startup address*/

/* 以下参数与链接文件相对应 */
/*appliction flash status info addr*/
//extern uint32_t __APP_FLASH_INFO_ADDR[];
//#define APP_FLASH_INFO_ADDR ((uint32_t*)__APP_FLASH_INFO_ADDR)
#define APP_FLASH_INFO_ADDR (0x00014000u)

#define FALSH_DRIVER_START (0x12u)
#define FALSH_DRIVER_END (0xABu)

/*flash driver start addr*/
//extern uint32_t __FLASH_DRIVER_START_ADDR[];
////#define FLASH_DRIVER_START_ADDR ((uint32_t*)__FLASH_DRIVER_START_ADDR)
//#define FLASH_DRIVER_START_ADDR (0x1FFF8000u)
//
///*flash driver end addr*/
//extern uint32_t __FLASH_DRIVER_END_ADDR[];
////#define FLASH_DRIVER_START_ADDR ((uint32_t*)__FLASH_DRIVER_END_ADDR)
//#define FLASH_DRIVER_END_ADDR   (0x1FFF8800u)


/*flash driver max data len*/
#define FLASH_DRIVER_LEN (FLASH_DRIVER_END_ADDR - FLASH_DRIVER_START_ADDR + 1u)

/*slave application data address offset base*/
#define SLAVE_APP_OFFSET_BASE (0x10000000u)


///*application can used space*/
//extern const tBlockInfo g_astBlockNum[];
//
///*logical num*/
//extern const unsigned char g_ucBlockNum;

//#ifdef NXF47391

//#define FLASH_API_DEBUG
//#define FLASH_SDK_USING

//#endif    //end of NXF47391

#endif /* FLASH_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
