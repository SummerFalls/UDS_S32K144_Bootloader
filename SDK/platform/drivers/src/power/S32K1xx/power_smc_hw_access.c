/*
 * Copyright (c) 2013-2016, Freescale Semiconductor, Inc.
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
 * @file power_smc_hw_access.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed
 * between pointer to void and an arithmetic type.
 * The address of hardware modules is provided as integer so
 * it needs to be cast to pointer.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, A conversion should not be performed
 * between a pointer to object and an integer type.
 * The address of hardware modules is provided as integer so
 * a conversion between a pointer and an integer has to be performed
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application or driver code.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, Expression assigned to a narrower or different essential type.
 * The cast is required to perform a conversion between an unsigned integer and an enum type with many values.
 */

#include <stddef.h>
#include "power_smc_hw_access.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * INTERNAL FUNCTIONS
 ******************************************************************************/
static bool SMC_WaitForStatChange(const SMC_Type * const baseAddr,
                                  const power_mode_stat_t mode,
                                  const uint32_t timeout);

/*! Timeout used for waiting to set new mode */
#define SMC_TIMEOUT 1000U


/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : SMC_SetPowerMode
 * Description   : Configure the power mode
 * This function will configure the power mode control for any run, stop and
 * stop submode if needed. It will also configure the power options for specific
 * power mode. Application should follow the proper procedure to configure and
 * switch power mode between the different run and stop mode. Refer to reference
 * manual for the proper procedure and supported power mode that can be configured
 * and switch between each other. Refer to smc_power_mode_config_t for required
 * parameters to configure the power mode and the supported options. Other options
 * may need to configure through the hal driver individaully. Refer to hal driver
 * header for details.
 *
 *END**************************************************************************/
status_t SMC_SetPowerMode(SMC_Type * const baseAddr,
                          const smc_power_mode_config_t * const powerModeConfig)
{
    status_t retCode;
    smc_stop_mode_t stopMode;
    power_manager_modes_t powerModeName = powerModeConfig->powerModeName;
    /* Branch based on power mode name*/
    switch (powerModeName)
    {
        case POWER_MANAGER_RUN:
            /* Biasing disabled, core logic can run in full performance */
            PMC_DisableBiasen(PMC);
            /* Set to RUN mode. */
            SMC_SetRunModeControl(baseAddr, SMC_RUN);
            /* Wait for stat change */
            if (!SMC_WaitForStatChange(baseAddr, STAT_RUN, SMC_TIMEOUT))
            {
                /* Timeout for power mode change expired. */
                retCode = STATUS_MCU_TRANSITION_FAILED;
            }
            else
            {
                retCode = STATUS_SUCCESS;
            }
            break;

        case POWER_MANAGER_VLPR:
            /* Biasing enable before entering VLP* mode to educe MCU power consumption in low power mode*/
            PMC_EnableBiasen(PMC);

            /* Set power mode to VLPR*/
            SMC_SetRunModeControl(baseAddr, SMC_VLPR);
            /* Wait for stat change */
            if (!SMC_WaitForStatChange(baseAddr, STAT_VLPR, SMC_TIMEOUT))
            {
                /* Timeout for power mode change expired. */
                retCode = STATUS_MCU_TRANSITION_FAILED;
            }
            else
            {
                retCode = STATUS_SUCCESS;
            }
            break;

#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
        case POWER_MANAGER_HSRUN:
            /* Biasing disabled, core logic can run in full performance */
            PMC_DisableBiasen(PMC);
            /* Set power mode to HSRUN */
            SMC_SetRunModeControl(baseAddr, SMC_HSRUN);
            /* Wait for stat change */
            if (!SMC_WaitForStatChange(baseAddr, STAT_HSRUN, SMC_TIMEOUT))
            {
                /* Timeout for power mode change expired. */
                retCode = STATUS_MCU_TRANSITION_FAILED;
            }
            else
            {
                retCode = STATUS_SUCCESS;
            }

            break;
#endif /* if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */
#if FEATURE_SMC_HAS_WAIT_VLPW
        case POWER_MANAGER_WAIT:
        /* Fall-through */
        case POWER_MANAGER_VLPW:
            /* Clear the SLEEPDEEP bit to disable deep sleep mode - WAIT */
            S32_SCB->SCR &= ~S32_SCB_SCR_SLEEPDEEP_MASK;

            /* Cpu is going into sleep state */
            STANDBY();

            retCode = STATUS_SUCCESS;
            break;
#endif /* if FEATURE_SMC_HAS_WAIT_VLPW */
        case POWER_MANAGER_STOP1:
        /* Fall-through */
        case POWER_MANAGER_STOP2:
        /* Fall-through */
        case POWER_MANAGER_VLPS:
            if ((powerModeName == POWER_MANAGER_STOP1) || (powerModeName == POWER_MANAGER_STOP2))
            {
                stopMode = SMC_STOP;
#if FEATURE_SMC_HAS_STOPO
                SMC_SetStopOption(baseAddr, powerModeConfig->stopOptionValue);
#endif
#if FEATURE_SMC_HAS_PSTOPO
                SMC_SetPStopOption(baseAddr, powerModeConfig->pStopOptionValue);
#endif
            }
            else
            {
                /* Biasing enable before entering VLP* mode to educe MCU power consumption in low power mode*/
                PMC_EnableBiasen(PMC);
                stopMode = SMC_VLPS;
            }

            /* Set power mode to specified STOP mode*/
            SMC_SetStopModeControl(baseAddr, stopMode);

            /* Set the SLEEPDEEP bit to enable deep sleep mode (STOP)*/
            S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;

            /* Cpu is going into deep sleep state */
            STANDBY();

            /* check the current mode to control bias bit */
            if (SMC_GetPowerModeStatus(baseAddr) == STAT_RUN)
            {
                PMC_DisableBiasen(PMC);
            }
            else
            {
                PMC_EnableBiasen(PMC);
            }

            retCode = STATUS_SUCCESS;
            break;
        default:
            retCode = STATUS_UNSUPPORTED;
            break;
    }

    return retCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SMC_SetProtectionMode
 * Description   : Configure all power mode protection settings
 * This function will configure the power mode protection settings for
 * supported power mode on the specified chip family. The available power modes
 * are defined in smc_power_mode_protection_config_t. Application should provide
 * the protect settings for all supported power mode on the chip and also this
 * should be done at early system level initialize stage. Refer to reference manual
 * for details. This register can only write once after power reset. So either
 * use this function or use the individual set function if you only have single
 * option to set.
 *
 *END**************************************************************************/
void SMC_SetProtectionMode(SMC_Type * const baseAddr,
                           const smc_power_mode_protection_config_t * const protectConfig)
{
    /* Initialize the setting */
    uint32_t regValue = 0U;

    /* Check configurations for each mode and combine the setting together */
    if (protectConfig->vlpProt)
    {
        regValue |= SMC_PMPROT_AVLP(1);
    }

#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
    if (protectConfig->hsrunProt)
    {
        regValue |= SMC_PMPROT_AHSRUN(1);
    }

#endif

    /* Write once into PMPROT register*/
    baseAddr->PMPROT = regValue;
}

/*FUNCTION**********************************************************************
 * Function Name : SMC_WaitForStatChange
 * Description   : Internal function used by SMC_SetPowerMode function
 * to wait until the state is changed or timeout expires
 *
 * return power mode status change
 *                - true: power mode has been changed successfully
 *                - false: timeout expired, power mode has not been changed
 *END**************************************************************************/
static bool SMC_WaitForStatChange(const SMC_Type * const baseAddr,
                                  const power_mode_stat_t mode,
                                  const uint32_t timeout)
{
    uint32_t i;
    bool retValue;

    /* Waiting for register read access. It's no longer apply when bus clock has very low frequency in HSRUN mode */
    if (mode == STAT_HSRUN)
    {
        for (i = 0U; i < 100U; i++)
        {
            /* Do nothing */
        }
    }

    for (i = 0U; i < timeout; i++)
    {
        if (mode == SMC_GetPowerModeStatus(baseAddr))
        {
            /* Power mode has been changed successfully */
            break;
        }
    }

    /* If i greater or equal to timeout, then timeout expired(the power mode has not been changed)*/
    retValue = (i < timeout);

    return retValue;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
