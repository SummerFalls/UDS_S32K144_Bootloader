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

#ifdef USE_FLASH_DRIVER
#define FALSH_DRIVER_START (0x12u)
#define FALSH_DRIVER_END   (0xABu)
#endif

/* slave application data address offset base */
#define SLAVE_APP_OFFSET_BASE (0x10000000u)

//#define FLASH_API_DEBUG
//#define FLASH_SDK_USING

#endif /* FLASH_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
