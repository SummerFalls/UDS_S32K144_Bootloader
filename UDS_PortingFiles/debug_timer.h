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

void DEBUG_TIMER_Init(void);

uint32 DEBUG_TIMER_GetTimerValue(void);

void DEBUG_TIMER_Deinit(void);

#endif

#endif /* DEBUG_TIMER_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
