/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
 * Violates MISRA 2012 Required Rule 10.3, Expression assigned to a narrower or different essential type
 * The cast is used to convert from uint32_t to enum_type.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 10.5, Impermissible cast; cannot cast from 'essentially unsigned' to 'essentially enum<i>'
 * The cast is used to convert from uint32_t to enum_type.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 */

#include "lptmr_driver.h"
#include "lptmr_hw_access.h"
#include "clock_manager.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Takes into consideration that LPTMR compare events take place
 * when "the CNR equals the value of the CMR and increments" */
#define LPTMR_MAX_CMR_NTICKS (LPTMR_CMR_COMPARE_MASK + 1u)
#define LPTMR_MAX_PRESCALER  (1u << LPTMR_PSR_PRESCALE_WIDTH)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @cond DRIVER_INTERNAL_USE_ONLY */

/* Table of base addresses for LPTMR instances */
static LPTMR_Type* const g_lptmrBase[LPTMR_INSTANCE_COUNT] = LPTMR_BASE_PTRS;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static inline uint8_t lptmr_cfg2p(
    const lptmr_prescaler_t prescval,
    const bool bypass
    );

static inline uint64_t lptmr_us2nn(
    const uint32_t clkfreq,
    const uint32_t us
    );

static inline uint64_t lptmr_compute_nticks(
    uint64_t nn,
    uint8_t p
    );

static inline bool nticks2compare_ticks(
    uint64_t nticks,
    uint16_t* ticks
    );

static uint32_t lptmr_GetClkFreq(
    const lptmr_clocksource_t clkSrc,
    const uint32_t instance
    );

static bool lptmr_Ticks2Us(
    const uint32_t clkfreq,
    const lptmr_prescaler_t pval,
    const bool bypass,
    const uint16_t ticks,
    uint32_t* const us
    );

static bool lptmr_Us2Ticks(
    const uint32_t clkfreq,
    const lptmr_prescaler_t prescval,
    const bool bypass,
    const uint32_t us,
    uint16_t* const ticks
    );

static bool lptmr_ChooseClkConfig(
    const uint32_t clkfreq,
    const uint32_t us,
    lptmr_prescaler_t* const prescval,
    bool* const bypass,
    uint16_t* const ticks
    );

/*TIMER MODE CONFIGURATION******************************************************
 *
 * Timer Mode - Prescaler settings calculations
 * --------------------------------------------
 *
 * Timer Mode configuration takes a period (timeout) value expressed in
 * micro-seconds. To convert this to LPTMR prescaler (and compare value)
 * settings, the closest match must be found.
 * For best precision, the lowest prescaler that allows the corresponding
 * compare value to fit in the 16-bit register will be chosen.
 *
 * Algorithm for choosing prescaler and compare values:
 * =============================================================================
 * In: tper_us (period in microseconds), fclk (input clock frequency in Hertz)
 * Out: nticks (timer ticks), p (prescaler coefficient, 2^p = prescaler value)
 * ---
 * 1) Compute nn = tper_us * fclk / 1000000
 * 2) for p = 0..16
 *  2.1) nticks = nn / 2^p
 *  2.2) if nticks < 0x10000
 *      2.2.1) STOP, found nticks and p
 * 3) nticks = 0xFFFF, p = 16
 * =============================================================================
 *
 * A few names used throughout the static functions affecting Timer mode:
 *  nn - total number of timer ticks (undivided, unprescaled) that is necessary
 *      for a particular timeout.
 *      nn = (tper_us * fclk) / 1000000 = nticks * npresc
 *
 *  tper_us - a period (or timeout) expressed in microsecond units. In most
 *      functions will be denoted as 'us' for microseconds.
 *
 *  nticks - number of timer ticks that is necessary for a particular timeout,
 *      after prescaling
 *
 *  npresc - prescaler value (1, 2, 4 ... 65536)
 *
 *  p - prescaler coefficient, 2^p = npresc
 *
 *  fclk - input clock frequency, in Hertz. In most function will be denoted as
 *      'clkfreq'.
 *END**************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_cfg2p
 * Description   : Transform prescaler settings (bypass on/off, prescaler value)
 *  to prescaler coefficient value (2's power), p.
 * Return: the value of p.
 *END**************************************************************************/
static inline uint8_t lptmr_cfg2p(
    const lptmr_prescaler_t prescval,
    const bool bypass
    )
{
    uint8_t p = 0u;

    if (!bypass)
    {
        p = (uint8_t)(((uint8_t)prescval) + 1u);
    }

    return p;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_us2nn
 * Description   : Transform microseconds to undivided (unprescaled) timer units,
 * nn.
 * Return: the value of nn.
 *END**************************************************************************/
static inline uint64_t lptmr_us2nn(
    const uint32_t clkfreq,
    const uint32_t us
    )
{
    /* Approximate the timeout in undivided (unprescaled) timer ticks.
        - us is the timeout in microseconds (1/10^6 seconds)
        - clkfreq is the frequency in Hertz
        Operation:
        nn = (us/1000000) * clkfreq
        In C:
        For better precision, first to the multiplication (us * clkfreq)
        To overcome the truncation of the div operator in C, add half of the
        denominator before the division. Hence:
        nn = (us * clkfreq + 500000) / 1000000
    */
    /* There is no risk of overflow since us is 32-bit wide and clkfreq can be
       a theoretical maximum of ~100 MHz (platform maximum), which is over the
       maximum input of the LPTMR anyway
     */
    uint64_t nn = (uint64_t)( (uint64_t)us * (uint64_t)clkfreq );
    nn = (nn + 500000u) / 1000000u;
    return nn;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_compute_nticks
 * Description   : Compute total number of divided (prescaled) timer ticks,
 * nticks.
 * Return: the value of nticks.
 *END**************************************************************************/
static inline uint64_t lptmr_compute_nticks(
    uint64_t nn,
    uint8_t p
    )
{
    uint64_t npresc = (uint64_t) 1u << p;
    DEV_ASSERT(npresc != 0u);

    /* integer division */
    uint64_t nticks = ((nn + (npresc >> 1u)) / npresc);

    return nticks;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : nticks2compare_ticks
 * Description   : Transform the value of divided (prescaled) timer ticks, nticks
 * to a 16-bit value to be written to the hardware register. Cap or underflow
 * cause an error.
 * Return: the success state.
 *  - true: no underflow or overflow detected
 *  - false: value written was capped, underflow or overflow detected
 *
 *END**************************************************************************/
static inline bool nticks2compare_ticks(
    uint64_t nticks,
    uint16_t* ticks
    )
{
    bool success = true;

    /* if nticks fits, write the value to ticks */
    if (nticks <= LPTMR_MAX_CMR_NTICKS)
    {
        if (nticks == 0u)
        {
            /* timeout period (us) too low for prescaler settings */
            *ticks = 0u;
            success = false;
        }
        else{
            /* According to RM, the LPTMR compare events take place when "the CNR equals the value of the CMR and increments".
             * The additional increment is compensated here by decrementing the calculated compare value with 1, before being written to CMR. */
            *ticks = (uint16_t)(nticks - 1u);
        }
    }
    else {
        /* timeout period (us) too high for prescaler settings */
        *ticks = LPTMR_CMR_COMPARE_MASK;
        success = false;
    }

    return success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_GetClkFreq
 * Description   : Get the clock frequency for the selected clock source. If the
 * selected clock source is not enabled, a frequency of 0 is returned.
 * Return values:
 *  - the clock frequency or 0 if the clock is invalid.
 *
 *END**************************************************************************/
static uint32_t lptmr_GetClkFreq(const lptmr_clocksource_t clkSrc,
                                 const uint32_t instance)
{
    /* LPTMR PCC clock source names, for getting the input clock frequency */
    static const clock_names_t lptmrPccClockName[LPTMR_INSTANCE_COUNT] = {LPTMR0_CLK};
    clock_names_t inputClockName = SIRC_CLK;
    uint32_t      clkFreq;
    status_t      clkStatus;

    /* Get input clock name */
    switch(clkSrc)
    {
    case LPTMR_CLOCKSOURCE_SIRCDIV2:
        inputClockName = SIRC_CLK;
        break;
    case LPTMR_CLOCKSOURCE_1KHZ_LPO:
        inputClockName = SIM_LPO_1K_CLK;
        break;
    case LPTMR_CLOCKSOURCE_RTC:
        inputClockName = SIM_RTCCLK_CLK;
        break;
    case LPTMR_CLOCKSOURCE_PCC:
        inputClockName = lptmrPccClockName[instance];
        break;
    default:
        /* Invalid clock source */
        DEV_ASSERT(false);
        break;
    }

    /* Get input clock frequency */
    if (inputClockName == SIRC_CLK)
    {
        clkStatus = CLOCK_SYS_GetFreq(SIRCDIV2_CLK, &clkFreq);
        DEV_ASSERT(clkStatus == STATUS_SUCCESS);
        (void) clkStatus;
        DEV_ASSERT(clkFreq != 0u); /* If the GetFreq functions fails, clkfreq will be 0 */
    }
    else
    {
        clkStatus = CLOCK_SYS_GetFreq(inputClockName, &clkFreq);
        DEV_ASSERT(clkStatus == STATUS_SUCCESS);
        (void) clkStatus;
        DEV_ASSERT(clkFreq != 0u); /* If the GetFreq functions fails, clkfreq will be 0 */
    }

    return clkFreq;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_Ticks2Us
 * Description   : Transform timer ticks to microseconds using the given
 * prescaler settings. Clock frequency must be valid (different from 0).
 * Possible return values:
 * - true: conversion success
 * - false: conversion failed, result did not fit in 32-bit.
 *
 *END**************************************************************************/
static bool lptmr_Ticks2Us(
    const uint32_t clkfreq,
    const lptmr_prescaler_t pval,
    const bool bypass,
    const uint16_t ticks,
    uint32_t* const us
    )
{
    bool success = true;
    uint8_t p = lptmr_cfg2p(pval, bypass);
    uint64_t nn = ( (uint64_t)ticks + 1u ) << p;
    uint64_t us_real = (nn * 1000000u) / (clkfreq);
    uint32_t us_local;

    if ( us_real <= (0xFFFFFFFFu) )
    {
        us_local = (uint32_t)us_real;
    }
    else
    {
        us_local = 0xFFFFFFFFu;
        success = false;
    }

    *us = us_local;
    return success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_Us2Ticks
 * Description   : Transform microseconds to timer ticks using the given
 * prescaler settings. Input clock frequency, clkfreq, must be greater than 0.
 * Possible return values:
 * - true: conversion completed successfully
 * - false: conversion failed, value did not fit in 16-bit.
 *
 *END**************************************************************************/
static bool lptmr_Us2Ticks(
    const uint32_t clkfreq,
    const lptmr_prescaler_t prescval,
    const bool bypass,
    const uint32_t us,
    uint16_t* const ticks
    )
{
    bool success = true;
    /* Transform prescaler configuration to prescaler coefficient p */
    uint8_t p = lptmr_cfg2p(prescval, bypass);
    /* Compute nn, the number of ticks necessary for the period in microseconds
       without any prescaler */
    uint64_t nn = lptmr_us2nn(clkfreq, us);
    /* Compute nticks, total number of ticks with prescaler */
    uint64_t nticks = lptmr_compute_nticks(nn, p);
    /* Transform nticks to value to be written to register */
    success = nticks2compare_ticks(nticks, ticks);
    return success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : lptmr_ChooseClkConfig
 * Description   : Choose clocking configuration (prescaler value, timer ticks)
 * for the desired timeout period, given in microseconds. Input clock frequency,
 * clkfreq, must be greater than 0.
 * Possible return values:
 * - true: configuration found
 * - false: configuration mismatch, desired timeout period is too small or too
 * big for the clock settings.
 *
 *END**************************************************************************/
static bool lptmr_ChooseClkConfig(
    const uint32_t clkfreq,
    const uint32_t us,
    lptmr_prescaler_t* const prescval,
    bool* const bypass,
    uint16_t* const ticks
    )
{
    uint8_t p;
    uint64_t nticks = 0ULL;
    bool success;

    uint64_t nn = lptmr_us2nn(clkfreq, us);

    /* Find the lowest prescaler value that allows the compare value in 16-bits */
    for (p = 0u; p <= LPTMR_MAX_PRESCALER; p++)
    {
        nticks = lptmr_compute_nticks(nn, p);

        if (nticks <= LPTMR_MAX_CMR_NTICKS)
        {
            /* Search finished, value will fit in the 16-bit register */
            break;
        }
    }

    success = nticks2compare_ticks(nticks, ticks);

    /* Convert p to prescaler configuration */
    if (p == 0u)
    {
        /* Prescaler value of 1 */
        *bypass = true;
        *prescval = LPTMR_PRESCALE_2;
    }
    else{
        *bypass = false;
        p--; /* Decrement to match lptmr_prescaler_t.  */
        *prescval = (lptmr_prescaler_t) p;
    }

    return success;
}


/*******************************************************************************
 * Public Functions
 ******************************************************************************/


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_InitConfigStruct
 * Description   : Initialize a configuration structure with default values.
 *
 * Implements : LPTMR_DRV_InitConfigStruct_Activity
 *END**************************************************************************/
void LPTMR_DRV_InitConfigStruct(lptmr_config_t * const config)
{
    DEV_ASSERT(config != NULL);

    /* General parameters */
    config->dmaRequest      = false;
    config->interruptEnable = false;
    config->freeRun         = false;
    config->workMode        = LPTMR_WORKMODE_TIMER;

    /* Counter parameters */
    config->clockSelect     = LPTMR_CLOCKSOURCE_SIRCDIV2;
    config->prescaler       = LPTMR_PRESCALE_2;
    config->bypassPrescaler = false;
    config->compareValue    = 0u;
    config->counterUnits    = LPTMR_COUNTER_UNITS_TICKS;

    /* Pulse Counter specific parameters */
    config->pinSelect       = LPTMR_PINSELECT_TRGMUX;
    config->pinPolarity     = LPTMR_PINPOLARITY_RISING;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_Init
 * Description   : Initialize a LPTMR instance based on the input configuration
 * structure.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS) the function will
 * automatically configure the timer for the input compareValue in microseconds.
 * The input parameters for 'prescaler' and 'bypassPrescaler' will be ignored
 * - their values will be adapted by the function, to best fit the input compareValue
 * (in microseconds) for the operating clock frequency.
 *
 * LPTMR_COUNTER_UNITS_MICROSECONDS may only be used for LPTMR_WORKMODE_TIMER mode.
 * Otherwise the function shall not convert 'compareValue' in ticks
 * and this is likely to cause erroneous behavior.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_TICKS) the function will use the
 * 'prescaler' and 'bypassPrescaler' provided in the input configuration structure.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_TICKS), 'compareValue' must be lower
 * than 0xFFFFu. Only the least significant 16bits of 'compareValue' will be used.
 * When (counterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS), 'compareValue'
 * may take any 32bits unsigned value.
 *
 * Implements : LPTMR_DRV_Init_Activity
 *END**************************************************************************/
void LPTMR_DRV_Init(const uint32_t instance,
                    const lptmr_config_t * const config,
                    const bool startCounter)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    LPTMR_Type* const base = g_lptmrBase[instance];

    LPTMR_DRV_SetConfig(instance, config);

    /* Start the counter if requested */
    if (startCounter)
    {
        LPTMR_Enable(base);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_SetConfig
 * Description   : Configure a LPTMR instance based on the input configuration
 * structure.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS) the function will
 * automatically configure the timer for the input compareValue in microseconds.
 * The input parameters for 'prescaler' and 'bypassPrescaler' will be ignored
 * - their values will be adapted by the function, to best fit the input compareValue
 * (in microseconds) for the operating clock frequency.
 *
 * LPTMR_COUNTER_UNITS_MICROSECONDS may only be used for LPTMR_WORKMODE_TIMER mode.
 * Otherwise the function shall not convert 'compareValue' in ticks
 * and this is likely to cause erroneous behavior.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_TICKS) the function will use the
 * 'prescaler' and 'bypassPrescaler' provided in the input configuration structure.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_TICKS), 'compareValue' must be lower
 * than 0xFFFFu. Only the least significant 16bits of 'compareValue' will be used.
 * When (counterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS), 'compareValue'
 * may take any 32bits unsigned value.
 *
 * Implements : LPTMR_DRV_SetConfig_Activity
 *END**************************************************************************/
void LPTMR_DRV_SetConfig(const uint32_t instance,
                         const lptmr_config_t * const config)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    LPTMR_Type* const base          = g_lptmrBase[instance];
    uint32_t configCmpValue         = config->compareValue;
    lptmr_workmode_t configWorkMode = config->workMode;
    uint16_t cmpValueTicks          = 0U;
    lptmr_prescaler_t prescVal      = config->prescaler;
    bool prescBypass                = config->bypassPrescaler;
    lptmr_counter_units_t configCounterUnits = config->counterUnits;

    if(configWorkMode == LPTMR_WORKMODE_TIMER)
    {
        /* A valid clock must be selected when used in Timer Mode. */
        uint32_t clkFreq;
        clkFreq = lptmr_GetClkFreq(config->clockSelect, instance);
        DEV_ASSERT(clkFreq != 0U); /* Clock frequency equal to '0', signals invalid value.  */

        if(configCounterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS)
        {
            bool chooseClkConfigStatus;

            /* When workmode is set to Timer Mode and compare value is provided in microseconds,
             * then the input parameters for prescale value and prescaleBypass are ignored.
             * The prescaleValue, prescaleBypass and cmpValue in ticks, are calculated to best fit
             * the input configCmpValue (in us) for the current operating clk frequency.  */
            chooseClkConfigStatus = lptmr_ChooseClkConfig(clkFreq, configCmpValue, &prescVal, &prescBypass, &cmpValueTicks);
            DEV_ASSERT(chooseClkConfigStatus == true);
            (void) chooseClkConfigStatus;
        }
        else
        {
            DEV_ASSERT(configCounterUnits == LPTMR_COUNTER_UNITS_TICKS);
            DEV_ASSERT(configCmpValue <= LPTMR_CMR_COMPARE_MASK); /* Compare Value in Tick Units must fit in CMR. */

            cmpValueTicks = (uint16_t)(configCmpValue & LPTMR_CMR_COMPARE_MASK);
        }
    }
    else
    {
        /* If configWorkMode is not LPTMR_WORKMODE_TIMER, then it must be LPTMR_WORKMODE_PULSECOUNTER. */
        DEV_ASSERT(configWorkMode == LPTMR_WORKMODE_PULSECOUNTER);

        /* Only LPTMR_COUNTER_UNITS_TICKS can be used when LPTMR is configured as Pulse Counter. */
        DEV_ASSERT(config->counterUnits == LPTMR_COUNTER_UNITS_TICKS);
        /* A valid clock must be selected when glitch filter is enabled (prescaler not bypassed). */
        DEV_ASSERT((lptmr_GetClkFreq(config->clockSelect, instance) != 0u) || prescBypass);
        /* Glitch filter does not support LPTMR_PRESCALE_2. */
        DEV_ASSERT(prescBypass || (prescVal != LPTMR_PRESCALE_2));

        DEV_ASSERT(configCmpValue <= LPTMR_CMR_COMPARE_MASK); /* Compare Value in Tick Units must fit in CMR. */

        cmpValueTicks = (uint16_t)(configCmpValue & LPTMR_CMR_COMPARE_MASK);
    }

    /* Initialize and write configuration parameters. */
    LPTMR_Init(base);

    LPTMR_SetDmaRequest   (base, config->dmaRequest);
    LPTMR_SetInterrupt    (base, config->interruptEnable);
    LPTMR_SetFreeRunning  (base, config->freeRun);
    LPTMR_SetWorkMode     (base, configWorkMode);
    LPTMR_SetPrescaler    (base, prescVal);
    LPTMR_SetBypass       (base, prescBypass);
    LPTMR_SetClockSelect  (base, config->clockSelect);
    LPTMR_SetCompareValue (base, cmpValueTicks);
    LPTMR_SetPinSelect    (base, config->pinSelect);
    LPTMR_SetPinPolarity  (base, config->pinPolarity);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_GetConfig
 * Description   : Get the current configuration of the LPTMR instance.
 * Always returns compareValue in LPTMR_COUNTER_UNITS_TICKS.
 *
 * Implements : LPTMR_DRV_GetConfig_Activity
 *END**************************************************************************/
void LPTMR_DRV_GetConfig(const uint32_t instance,
                         lptmr_config_t * const config)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    const LPTMR_Type* const base = g_lptmrBase[instance];

    /* Read current configuration */
    config->dmaRequest      = LPTMR_GetDmaRequest(base);
    config->interruptEnable = LPTMR_GetInterruptEnable(base);
    config->freeRun         = LPTMR_GetFreeRunning(base);
    config->workMode        = LPTMR_GetWorkMode(base);
    config->prescaler       = LPTMR_GetPrescaler(base);
    config->bypassPrescaler = LPTMR_GetBypass(base);
    config->clockSelect     = LPTMR_GetClockSelect(base);
    config->compareValue    = LPTMR_GetCompareValue(base);
    config->counterUnits    = LPTMR_COUNTER_UNITS_TICKS;
    config->pinSelect       = LPTMR_GetPinSelect(base);
    config->pinPolarity     = LPTMR_GetPinPolarity(base);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_Deinit
 * Description   : De-initialize the LPTMR (stop the counter and reset all registers to default value).
 *
 * Implements : LPTMR_DRV_Deinit_Activity
 *END**************************************************************************/
void LPTMR_DRV_Deinit(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];
    LPTMR_Disable(base);

    LPTMR_Init(base);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_SetCompareValueByCount
 * Description   : Set the compare value in counter tick units, for a LPTMR instance.
 * Possible return values:
 * - STATUS_SUCCESS: completed successfully
 * - STATUS_ERROR: cannot reconfigure compare value (TCF not set)
 * - STATUS_TIMEOUT: compare value is smaller than current counter value
 *
 * Implements : LPTMR_DRV_SetCompareValueByCount_Activity
 *END**************************************************************************/
status_t LPTMR_DRV_SetCompareValueByCount(const uint32_t instance,
                                          const uint16_t compareValueByCount)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base  = g_lptmrBase[instance];
    status_t statusCode     = STATUS_SUCCESS;

    bool timerEnabled = LPTMR_GetEnable(base);
    bool compareFlag  = LPTMR_GetCompareFlag(base);

    uint16_t counterVal;

    /* Check if a valid clock is selected for the timer/glitch filter */
#if (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT))
    bool bypass = LPTMR_GetBypass(base);
    lptmr_workmode_t workMode = LPTMR_GetWorkMode(base);
    (void) bypass;
    (void) workMode;
#endif /* (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT)) */
    DEV_ASSERT((lptmr_GetClkFreq(LPTMR_GetClockSelect(base), instance) != 0u) || \
               (bypass && (workMode == LPTMR_WORKMODE_PULSECOUNTER)));


    /* The compare value can only be written if counter is disabled or the compare flag is set. */
    if (timerEnabled && !compareFlag)
    {
        statusCode = STATUS_ERROR;
    }
    else
    {
        /* Check if new value is below the current counter value */
        LPTMR_SetCompareValue(base, compareValueByCount);
        counterVal = LPTMR_GetCounterValue(base);
        if (counterVal >= compareValueByCount)
        {
            statusCode = STATUS_TIMEOUT;
        }
    }

    return statusCode;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_GetCompareValueByCount
 * Description   : Get the compare value of timer in ticks units.
 *
 * Implements : LPTMR_DRV_GetCompareValueByCount_Activity
 *END**************************************************************************/
void LPTMR_DRV_GetCompareValueByCount(const uint32_t instance,
                                      uint16_t * const compareValueByCount)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    const LPTMR_Type* const base = g_lptmrBase[instance];

    *compareValueByCount = LPTMR_GetCompareValue(base);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_SetCompareValueUs
 * Description   : Set the compare value for Timer Mode in microseconds,
 * for a LPTMR instance.
 * Can be used only in Timer Mode.
 * Possible return values:
 * - STATUS_SUCCESS: completed successfully
 * - STATUS_ERROR: cannot reconfigure compare value
 * - STATUS_TIMEOUT: compare value greater then current counter value
 *
 * Implements : LPTMR_DRV_SetCompareValueByUs_Activity
 *END**************************************************************************/
status_t LPTMR_DRV_SetCompareValueByUs(const uint32_t instance,
                                       const uint32_t compareValueUs)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    status_t returnCode     = STATUS_SUCCESS;
    LPTMR_Type* const base  = g_lptmrBase[instance];
    bool timerEnabled, compareFlag;

    lptmr_clocksource_t clkSrc;
    uint32_t clkFreq;
    uint16_t cmpValTicks, currentCounterVal;
    lptmr_prescaler_t prescVal;
    bool prescBypass, conversionStatus;

    /* This function can only be used if LPTMR is configured in Timer Mode. */
    DEV_ASSERT(LPTMR_GetWorkMode(base) == LPTMR_WORKMODE_TIMER);

    timerEnabled = LPTMR_GetEnable(base);
    compareFlag  = LPTMR_GetCompareFlag(base);
    /* The compare value can only be written if counter is disabled or the compare flag is set. */
    if (timerEnabled && !compareFlag)
    {
        returnCode = STATUS_ERROR;
    }
    else
    {
        clkSrc  = LPTMR_GetClockSelect(base);
        clkFreq = lptmr_GetClkFreq(clkSrc, instance);
        DEV_ASSERT(clkFreq != 0U); /* Check the calculated clock frequency: '0' - invalid*/

        /* Get prescaler value and prescaler bypass state.*/
        prescVal    = LPTMR_GetPrescaler(base);
        prescBypass = LPTMR_GetBypass(base);
        /* Convert new compare value from microseconds to ticks. */
        conversionStatus = lptmr_Us2Ticks(clkFreq, prescVal, prescBypass, compareValueUs, &cmpValTicks);
        DEV_ASSERT(conversionStatus == true); /* Check the conversion status: compareValueUs doesn't fit for current prescaller. */
        (void) conversionStatus;

        /* Write value and check if written successfully */
        LPTMR_SetCompareValue(base, cmpValTicks);
        currentCounterVal = LPTMR_GetCounterValue(base);

        if (currentCounterVal >= cmpValTicks)
        {
            returnCode = STATUS_TIMEOUT;
        }
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_GetCompareValueByUs
 * Description   : Get the compare value in microseconds representation.
 * Can be used only in Timer Mode.
 *
 * Implements : LPTMR_DRV_GetCompareValueByUs_Activity
 *END**************************************************************************/
void LPTMR_DRV_GetCompareValueByUs(const uint32_t instance,
                                   uint32_t * const compareValueUs)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);
    DEV_ASSERT(compareValueUs != NULL);

    const LPTMR_Type* const base = g_lptmrBase[instance];
    lptmr_clocksource_t clkSrc;
    uint32_t clkFreq;
    uint16_t cmpValTicks;
    lptmr_prescaler_t prescVal;
    bool prescBypass, conversionStatus;

    /* This function can only be used if LPTMR is configured in Timer Mode. */
    DEV_ASSERT(LPTMR_GetWorkMode(base) == LPTMR_WORKMODE_TIMER);

    clkSrc  = LPTMR_GetClockSelect(base);
    clkFreq = lptmr_GetClkFreq(clkSrc, instance);
    /* The clock frequency must be valid. */
    DEV_ASSERT(clkFreq != 0U);

    /* Get prescaler value and prescaler bypass state.*/
    prescVal    = LPTMR_GetPrescaler(base);
    prescBypass = LPTMR_GetBypass(base);
    cmpValTicks = LPTMR_GetCompareValue(base);

    /* Convert current compare value from ticks to microseconds. */
    conversionStatus = lptmr_Ticks2Us(clkFreq, prescVal, prescBypass, cmpValTicks, compareValueUs);
    DEV_ASSERT(conversionStatus == true); /* Check the conversion status. */
    (void) conversionStatus;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_GetCompareFlag
 * Description   : Get the current state of the Compare Flag of a LPTMR instance
 *
 * Implements : LPTMR_DRV_GetCompareFlag_Activity
 *END**************************************************************************/
bool LPTMR_DRV_GetCompareFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    const LPTMR_Type* const base = g_lptmrBase[instance];
    bool compareFlag = LPTMR_GetCompareFlag(base);

    return compareFlag;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_ClearCompareFlag
 * Description   : Clear the Compare Flag.
 *
 * Implements : LPTMR_DRV_ClearCompareFlag_Activity
 *END**************************************************************************/
void LPTMR_DRV_ClearCompareFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];

    LPTMR_ClearCompareFlag(base);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_IsRunning
 * Description   : Get the running state of a LPTMR instance.
 * Possible return values:
 * - true: Timer/Counter started
 * - false: Timer/Counter stopped
 *
 * Implements : LPTMR_DRV_IsRunning_Activity
 *END**************************************************************************/
bool LPTMR_DRV_IsRunning(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    const LPTMR_Type* const base = g_lptmrBase[instance];

    bool runningState = LPTMR_GetEnable(base);

    return runningState;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_SetInterrupt
 * Description   : Enable/disable the LPTMR interrupt.
 *
 * Implements : LPTMR_DRV_SetInterrupt_Activity
 *END**************************************************************************/
void LPTMR_DRV_SetInterrupt(const uint32_t instance,
                            const bool enableInterrupt)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];

    LPTMR_SetInterrupt(base, enableInterrupt);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_GetCounterValueTicks
 * Description   : Get the current Counter Value in timer ticks representation.
 * Return:
 *  - the counter value.
 *
 * Implements : LPTMR_DRV_GetCounterValueByCount_Activity
 *END**************************************************************************/
uint16_t LPTMR_DRV_GetCounterValueByCount(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];

    uint16_t counterVal = LPTMR_GetCounterValue(base);

    return counterVal;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_StartCounter
 * Description   : Enable (start) the counter.
 *
 * Implements : LPTMR_DRV_StartCounter_Activity
 *END**************************************************************************/
void LPTMR_DRV_StartCounter(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];

    /* Check if a valid clock is selected for the timer/glitch filter */
#if (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT))
    bool bypass = LPTMR_GetBypass(base);
    lptmr_workmode_t workMode = LPTMR_GetWorkMode(base);
    (void) bypass;
    (void) workMode;
#endif /* (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT)) */
    DEV_ASSERT((lptmr_GetClkFreq(LPTMR_GetClockSelect(base), instance) != 0u) || \
               (bypass && (workMode == LPTMR_WORKMODE_PULSECOUNTER)));

    LPTMR_Enable(base);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_StopCounter
 * Description   : Disable (stop) the counter.
 *
 * Implements : LPTMR_DRV_StopCounter_Activity
 *END**************************************************************************/
void LPTMR_DRV_StopCounter(const uint32_t instance)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];

    LPTMR_Disable(base);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_DRV_SetPinConfiguration
 * Description   : Set the Input Pin configuration for Pulse Counter mode.
 *
 * Implements : LPTMR_DRV_SetPinConfiguration_Activity
 *END**************************************************************************/
void LPTMR_DRV_SetPinConfiguration(const uint32_t instance,
                                   const lptmr_pinselect_t pinSelect,
                                   const lptmr_pinpolarity_t pinPolarity)
{
    DEV_ASSERT(instance < LPTMR_INSTANCE_COUNT);

    LPTMR_Type* const base = g_lptmrBase[instance];

    LPTMR_SetPinSelect(base, pinSelect);
    LPTMR_SetPinPolarity(base, pinPolarity);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
