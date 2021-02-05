/*
 * @ ����: debug_timer.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef DEBUG_TIMER_H_
#define DEBUG_TIMER_H_

#include "includes.h"

#ifdef EN_DEBUG_TIMER

extern void DEBUG_TIMER_Init(void);

extern  uint32 DEBUG_TIMER_GetTimerValue(void);

extern void DEBUG_TIMER_Deinit(void);

#endif /*#ifdef EN_DEBUG_TIMER*/

#endif /* DEBUG_TIMER_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
