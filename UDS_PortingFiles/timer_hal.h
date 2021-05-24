/*
 * @ 名称: timer_hal.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef TIMER_HAL_H_
#define TIMER_HAL_H_

#include "includes.h"

void TIMER_HAL_Init(void);

boolean TIMER_HAL_Is1msTickTimeout(void);

/* Timer 1ms period called */
void TIMER_HAL_1msPeriod(void);

/* check 100ms timeout? */
boolean TIMER_HAL_Is100msTickTimeout(void);

/* get timer tick cnt for random seed. */
uint32 TIMER_HAL_GetTimerTickCnt(void);

void TIMER_HAL_Deinit(void);

#endif /* TIMER_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
