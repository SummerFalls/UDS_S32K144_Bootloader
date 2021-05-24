/*
 * @ 名称: timer_hal.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "timer_hal.h"


static uint16 gs_1msCnt = 0u;
static uint16 gs_100msCnt = 0u;

static void LPTimerISR(void)
{
    LPTMR_DRV_ClearCompareFlag(INST_LPTMR1);
    TIMER_HAL_1msPeriod();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMER_HAL_Init
 * Description   : This function initial this module.
 *
 * Implements : TIMER_HAL_Init_Activity
 *END**************************************************************************/
void TIMER_HAL_Init(void)
{
    LPTMR_DRV_Init(INST_LPTMR1, &lpTmr1_config0, false);
    /* Install IRQ handler for LPTMR interrupt */
    INT_SYS_InstallHandler(LPTMR0_IRQn, &LPTimerISR, (isr_t *)0);
    /* Enable IRQ for LPTMR */
    INT_SYS_EnableIRQ(LPTMR0_IRQn);
    INT_SYS_EnableIRQGlobal();
    /* Start LPTMR counter */
    LPTMR_DRV_StartCounter(INST_LPTMR1);
}

/* Timer 1ms period called */
void TIMER_HAL_1msPeriod(void)
{
    uint16 cntTmp = 0u;
    /* Just for check time overflow or not? */
    cntTmp = gs_1msCnt + 1u;

    if (0u != cntTmp)
    {
        gs_1msCnt++;
    }

    cntTmp = gs_100msCnt + 1u;

    if (0u != cntTmp)
    {
        gs_100msCnt++;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMER_HAL_Is1msTickTimeout
 * Description   : This function is check timeout. If timeout return TRUE, else return FALSE.
 *
 * Implements : Is1msTickTimeout_Activity
 *END**************************************************************************/
boolean TIMER_HAL_Is1msTickTimeout(void)
{
    boolean result = FALSE;

    if (gs_1msCnt)
    {
        result = TRUE;
        gs_1msCnt--;
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TIMER_HAL_Is10msTickTimeout
 * Description   : This function is check timeout or not. If timeout return TRUE, else return FALSE.
 *
 * Implements : Is10msTickTimeout_Activity
 *END**************************************************************************/
boolean TIMER_HAL_Is100msTickTimeout(void)
{
    boolean result = FALSE;

    if (gs_100msCnt >= 100u)
    {
        result = TRUE;
        gs_100msCnt -= 100u;
    }

    return result;
}

/* Get timer tick cnt for random seed. */
uint32 TIMER_HAL_GetTimerTickCnt(void)
{
    /* This two variables not init before used, because it used for generate random */
    uint32 hardwareTimerTickCnt;
    uint32 timerTickCnt;
#if 0
    /* For S32K1xx get timer counter(LPTIMER), get timer count will trigger the period incorrect. */
    hardwareTimerTickCnt = LPTMR_DRV_GetCounterValueByCount(INST_LPTMR1);
#endif
#pragma GCC diagnostic ignored "-Wuninitialized"
    timerTickCnt = ((hardwareTimerTickCnt & 0xFFFFu)) | (timerTickCnt << 16u);
    return timerTickCnt;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : TIMER_HAL_Deinit
 * Description   : This function initial this module.
 *
 * Implements : TIMER_HAL_Deinit_Activity
 *END**************************************************************************/
void TIMER_HAL_Deinit(void)
{
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
