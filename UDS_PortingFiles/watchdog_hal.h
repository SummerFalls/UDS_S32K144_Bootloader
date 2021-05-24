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

void WATCHDOG_HAL_Init(void);

void WATCHDOG_HAL_Feed(void);

void WATCHDOG_HAL_SystemReset(void);

void WATCHDOG_HAL_Deinit(void);

#endif /* WATCHDOG_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
