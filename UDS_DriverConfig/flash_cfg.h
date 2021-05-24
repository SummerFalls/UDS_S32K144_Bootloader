/*
 * @ ����: flash_cfg.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
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
