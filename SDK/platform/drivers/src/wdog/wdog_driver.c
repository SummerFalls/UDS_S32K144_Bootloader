/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * @file wdog_driver.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable. A source of error in
 * writing dynamic code is that the stack segment may be different from the data
 * segment.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and
 * integer type.
 * The cast is required to initialize a pointer with an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
 * The cast is required to initialize a pointer with an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * The function is defined for use by application code.
 */

#include "wdog_hw_access.h"
#include "clock_manager.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for WDOG instances. */
static WDOG_Type * const s_wdogBase[] = WDOG_BASE_PTRS;

/*! @brief Table to save WDOG IRQ enum numbers defined in CMSIS header file. */
static const IRQn_Type s_wdogIrqId[] = WDOG_IRQS;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

#ifdef DEV_ERROR_DETECT
/* Gets the frequency of the specified WDOG clock source. Only used at development
 * time, when WDOG_DRV_Init checks if the needed clock sources are enabled. */
static uint32_t WDOG_DRV_GetClockSourceFreq(wdog_clk_source_t wdogClk)
{
    uint32_t freq = 0;

    switch (wdogClk)
    {
        case WDOG_BUS_CLOCK:
            (void)CLOCK_SYS_GetFreq(BUS_CLK, &freq);
            break;
        case WDOG_SIRC_CLOCK:
            (void)CLOCK_SYS_GetFreq(SIRC_CLK, &freq);
            break;
        case WDOG_SOSC_CLOCK:
            (void)CLOCK_SYS_GetFreq(SOSC_CLK, &freq);
            break;
        case WDOG_LPO_CLOCK:
            (void)CLOCK_SYS_GetFreq(SIM_LPO_CLK, &freq);
            break;
        default:
            /* Should not get here */
            break;
    }

    return freq;
}

#endif /* ifdef DEV_ERROR_DETECT */

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_Init
 * Description   : initialize the WDOG driver
 *
 * Implements    : WDOG_DRV_Init_Activity
 *END**************************************************************************/
status_t WDOG_DRV_Init(uint32_t instance,
                       const wdog_user_config_t * userConfigPtr)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    WDOG_Type * base           = s_wdogBase[instance];
    status_t status            = STATUS_SUCCESS;
    status_t statusClockSource = STATUS_SUCCESS;

#ifdef DEV_ERROR_DETECT
    uint32_t prevClockHz, crtClockHz;

    /* Check if the previous clock source and the configuration clock source
     * are enabled(if not, the counter will not be incremented) */
    prevClockHz = WDOG_DRV_GetClockSourceFreq(WDOG_GetClockSource(s_wdogBase[instance]));
    crtClockHz  = WDOG_DRV_GetClockSourceFreq(userConfigPtr->clkSource);

    if ((prevClockHz == 0U) || (crtClockHz == 0U))
    {
        statusClockSource = STATUS_ERROR;
    }
#endif
    /* If clock source disabled */
    if (statusClockSource == STATUS_SUCCESS)
    {
        /* If window mode enabled and window value greater than or equal to the timeout value. Or timeout value is 0 */
        if (((userConfigPtr->winEnable) && (userConfigPtr->windowValue >= userConfigPtr->timeoutValue)) \
            || (userConfigPtr->timeoutValue <= FEATURE_WDOG_MINIMUM_TIMEOUT_VALUE))
        {
            status = STATUS_ERROR;
        }
        else
        {
            /* Configure the WDOG module */
            status = WDOG_Config(base, userConfigPtr);
        }

        if (status == STATUS_SUCCESS)
        {
            /* enable WDOG timeout interrupt */
            INT_SYS_EnableIRQ(s_wdogIrqId[instance]);
        }
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_Deinit
 * Description   : De-initialize the WDOG driver
 *
 * Implements    : WDOG_DRV_Deinit_Activity
 *END**************************************************************************/
status_t WDOG_DRV_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];
    status_t status = STATUS_SUCCESS;

    INT_SYS_DisableIRQGlobal();

    /* If allowed reconfigures WDOG */
    if (WDOG_IsUpdateEnabled(base))
    {
        /* Disable WDOG timeout interrupt */
        INT_SYS_DisableIRQ(s_wdogIrqId[instance]);

        /* Disable WDOG */
        WDOG_Deinit(base);
    }
    else
    {
        status = STATUS_ERROR;
    }

    /* Enable global interrupt */
    INT_SYS_EnableIRQGlobal();

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_GetConfig
 * Description   : get the current configuration of the WDOG driver
 *
 * Implements    : WDOG_DRV_GetConfig_Activity
 *END**************************************************************************/
 void WDOG_DRV_GetConfig(uint32_t instance,
                         wdog_user_config_t * const config)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);
    const WDOG_Type *baseAddr = s_wdogBase[instance];

    *config = WDOG_GetConfig(baseAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_GetDefaultConfig
 * Description   : get default configuration of the WDOG driver
 *
 * Implements    : WDOG_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
void WDOG_DRV_GetDefaultConfig(wdog_user_config_t * const config)
{
    DEV_ASSERT(config != NULL);

    /* Construct CS register new value */
    config->winEnable       = false;
    config->prescalerEnable = false;
    config->intEnable       = false;
    config->updateEnable    = true;
    config->opMode.debug    = false;
    config->opMode.wait     = false;
    config->opMode.stop     = false;
    config->clkSource       = WDOG_LPO_CLOCK;
    /* Construct TOVAL register new value */
    config->timeoutValue    = FEATURE_WDOG_TO_RESET_VALUE;
    /* Construct WIN register new value */
    config->windowValue     = FEATURE_WDOG_WIN_RESET_VALUE;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_SetInt
 * Description   : enable/disable the WDOG timeout interrupt
 *
 * Implements    : WDOG_DRV_SetInt_Activity
 *END**************************************************************************/
status_t WDOG_DRV_SetInt(uint32_t instance,
                         bool enable)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];
    status_t status = STATUS_SUCCESS;

    /* If allowed reconfigures WDOG */
    if (WDOG_IsUpdateEnabled(base))
    {
        /* Disable global interrupt */
        INT_SYS_DisableIRQGlobal();
        /* Enable/disable WDOG timeout interrupt */
        WDOG_SetInt(base, enable);
        /* Enable global interrupt */
        INT_SYS_EnableIRQGlobal();
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_ClearIntFlag
 * Description   : Clear interrupt flag of the WDOG
 *
 * Implements    : WDOG_DRV_ClearIntFlag_Activity
 *END**************************************************************************/
void WDOG_DRV_ClearIntFlag(uint32_t instance)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];

    /* Clear interrupt flag of the WDOG */
    WDOG_ClearIntFlag(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_Trigger
 * Description   : Refreshes the WDOG counter
 *
 * Implements    : WDOG_DRV_Trigger_Activity
 *END**************************************************************************/
 void WDOG_DRV_Trigger(uint32_t instance)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];

    WDOG_Trigger(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_GetCounter
 * Description   : Get the value of the WDOG counter.
 *
 * Implements    : WDOG_DRV_GetCounter_Activity
 *END**************************************************************************/
uint16_t WDOG_DRV_GetCounter(uint32_t instance)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    const WDOG_Type * base = s_wdogBase[instance];

    return (uint16_t)base->CNT;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_SetWindow
 * Description   : Set window mode and window value of the WDOG.
 *
 * Implements    : WDOG_DRV_SetWindow_Activity
 *END**************************************************************************/
status_t WDOG_DRV_SetWindow(uint32_t instance,
                            bool enable,
                            uint16_t windowvalue)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];
    status_t status = STATUS_SUCCESS;

    /* If allowed reconfigures WDOG */
    if (WDOG_IsUpdateEnabled(base))
    {
        /* Set WDOG window mode */
        WDOG_SetWindowMode(base, enable);

        /* If enable window mode */
        if (enable)
        {
            /* Set window value for the WDOG */
            WDOG_SetWindowValue(base, windowvalue);
        }
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_SetMode
 * Description   : Set mode operation of the WDOG.
 *
 * Implements    : WDOG_DRV_SetMode_Activity
 *END**************************************************************************/
status_t WDOG_DRV_SetMode(uint32_t instance,
                          bool enable,
                          wdog_set_mode_t Setmode)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];
    status_t status = STATUS_SUCCESS;

    /* If allowed reconfigures WDOG */
    if (WDOG_IsUpdateEnabled(base))
    {
        switch (Setmode)
        {
            case WDOG_DEBUG_MODE:
                /* Set WDOG debug mode */
                WDOG_SetDebug(base, enable);
                break;
            case WDOG_WAIT_MODE:
                /* Set WDOG wait mode */
                WDOG_SetWait(base, enable);
                break;
            case WDOG_STOP_MODE:
                /* Set WDOG stop mode */
                WDOG_SetStop(base, enable);
                break;
            default:
                /* Do nothings */
                break;
        }
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_SetTimeout
 * Description   : Set time value of the WDOG timeout.
 *
 * Implements    : WDOG_DRV_SetTimeout_Activity
 *END**************************************************************************/
status_t WDOG_DRV_SetTimeout(uint32_t instance,
                             uint16_t timeout)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];
    status_t status = STATUS_SUCCESS;

    /* If allowed reconfigures WDOG */
    if (WDOG_IsUpdateEnabled(base))
    {
        WDOG_UNLOCK(base);

        base->TOVAL = timeout;
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_SetTestMode
 * Description   : Set test mode of the WDOG.
 *
 * Implements    : WDOG_DRV_SetTestMode_Activity
 *END**************************************************************************/
status_t WDOG_DRV_SetTestMode(uint32_t instance,
                              wdog_test_mode_t testMode)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    WDOG_Type * base = s_wdogBase[instance];
    uint32_t regValue = base->CS;
    status_t status = STATUS_SUCCESS;

    /* If allowed reconfigures WDOG */
    if (WDOG_IsUpdateEnabled(base))
    {
        regValue &= ~(WDOG_CS_TST_MASK);
        regValue |= WDOG_CS_TST(testMode);

        WDOG_UNLOCK(base);

        base->CS = regValue;
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WDOG_DRV_GetTestMode
 * Description   : Get test mode of the WDOG.
 *
 * Implements    : WDOG_DRV_GetTestMode_Activity
 *END**************************************************************************/
wdog_test_mode_t WDOG_DRV_GetTestMode(uint32_t instance)
{
    DEV_ASSERT(instance < WDOG_INSTANCE_COUNT);
    const WDOG_Type * base = s_wdogBase[instance];
    wdog_test_mode_t testMode = WDOG_TST_DISABLED;

    /* Gets test mode */
    switch (WDOG_GetTestMode(base))
    {
        case 0U:
            testMode = WDOG_TST_DISABLED;
            break;
        case 1U:
            testMode = WDOG_TST_USER;
            break;
        case 2U:
            testMode = WDOG_TST_LOW;
            break;
        case 3U:
            testMode = WDOG_TST_HIGH;
            break;
        default:
            testMode = WDOG_TST_DISABLED;
            break;
    }

    return testMode;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
