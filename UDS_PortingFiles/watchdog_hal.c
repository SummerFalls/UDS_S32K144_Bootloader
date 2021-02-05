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

/*FUNCTION**********************************************************************
 *
 * Function Name : WATCHDOG_HAL_Init
 * Description   : This function initial this module.
 *
 * Implements : WATCHDOG_HAL_Init_Activity
 *END**************************************************************************/
void WATCHDOG_HAL_Init(void)
{
    WDOG_DRV_Init(INST_WATCHDOG1, &watchdog1_Config0);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WATCHDOG_HAL_Init
 * Description   : This function is fed watchdog.
 *
 * Implements : WATCHDOG_HAL_Init_Activity
 *END**************************************************************************/
void WATCHDOG_HAL_Fed(void)
{
    WDOG_DRV_Trigger(INST_WATCHDOG1);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : WATCHDOG_HAL_Init
 * Description   : This function is trigger system reset.
 *
 * Implements : WATCHDOG_HAL_Init_Activity
 *END**************************************************************************/

void WATCHDOG_HAL_SystemRest(void)
{
    WDOG_DRV_SetTimeout(INST_WATCHDOG1, 0u);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : WATCHDOG_HAL_Deinit
 * Description   : This function initial this module.
 *
 * Implements : WATCHDOG_HAL_Deinit_Activity
 *END**************************************************************************/
void WATCHDOG_HAL_Deinit(void)
{

}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
