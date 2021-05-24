/*
 * @ 名称: debug_timer.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "user_config.h"

#ifdef EN_DEBUG_TIMER

#include "Cpu.h"
#include "debug_timer.h"

void DEBUG_TIMER_Init(void)
{
}

uint32 DEBUG_TIMER_GetTimerValue(void)
{
    uint32 timerValue = 0u;
    return timerValue;
}

void DEBUG_TIMER_Deinit(void)
{
}

#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
