/*
 * @ ����: watchdog_hal.c
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "watchdog_hal.h"

void WATCHDOG_HAL_Init(void)
{
    WDOG_DRV_Init(INST_WATCHDOG1, &watchdog1_Config0);
}

void WATCHDOG_HAL_Feed(void)
{
    WDOG_DRV_Trigger(INST_WATCHDOG1);
}

void WATCHDOG_HAL_SystemReset(void)
{
    WDOG_DRV_SetTimeout(INST_WATCHDOG1, 0u);
}

void WATCHDOG_HAL_Deinit(void)
{
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
