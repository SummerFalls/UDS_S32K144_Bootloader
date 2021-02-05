/*
 * @ 名称: watchdog_hal.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef WATCHDOG_HAL_H_
#define WATCHDOG_HAL_H_

#include "includes.h"

extern void WATCHDOG_HAL_Init(void);

extern void WATCHDOG_HAL_Fed(void);

extern void WATCHDOG_HAL_SystemRest(void);

extern void WATCHDOG_HAL_Deinit(void);

#endif /* WATCHDOG_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
