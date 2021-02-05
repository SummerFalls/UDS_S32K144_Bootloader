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

extern void TIMER_HAL_Init(void);

extern boolean TIMER_HAL_Is1msTickTimeout(void);

/*Timer 1ms period called*/
extern void TIMER_HAL_1msPeriod(void);

/*check 100ms timeout?*/
extern boolean TIMER_HAL_Is100msTickTimeout(void);

/*get timer tick cnt for random seed.*/
extern uint32 TIMER_HAL_GetTimerTickCnt(void);

extern void TIMER_HAL_Deinit(void);

#endif /* TIMER_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
