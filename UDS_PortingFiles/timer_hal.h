/*
 * @ ����: timer_hal.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
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
