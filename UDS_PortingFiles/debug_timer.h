/*
 * @ 名称: debug_timer.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
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
