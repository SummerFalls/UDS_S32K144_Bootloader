/*
 * Copyright (c) 2013-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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

#ifndef POWER_SMC_HW_ACCESS_H
#define POWER_SMC_HW_ACCESS_H

#include "status.h"
#include "device_registers.h"
#include "power_manager_S32K1xx.h"

/*!
 * @file power_smc_hw_access.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.3, Global typedef not referenced.
 * User configuration structure is defined in Hal and is referenced from Driver.
 */

/*!
 * power_smc_hw_access
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*! @name System mode controller APIs*/
/*@{*/

/*!
 * @brief Configures the power mode.
 *
 * This function configures the power mode control for both run, stop, and
 * stop sub mode if needed. Also it configures the power options for a specific
 * power mode. An application should follow the proper procedure to configure and
 * switch power modes between  different run and stop modes. For proper procedures
 * and supported power modes, see an appropriate chip reference
 * manual. See the smc_power_mode_config_t for required
 * parameters to configure the power mode and the supported options. Other options
 * may need to be individually configured through the HAL driver. See the HAL driver
 * header file for details.
 *
 * @param baseAddr  Base address for current SMC instance.
 * @param powerModeConfig Power mode configuration structure smc_power_mode_config_t
 * @return errorCode SMC error code
 */
status_t SMC_SetPowerMode(SMC_Type * const baseAddr,
                          const smc_power_mode_config_t * const powerModeConfig);

/*!
 * @brief Configures all power mode protection settings.
 *
 * This function configures the power mode protection settings for
 * supported power modes in the specified chip family. The available power modes
 * are defined in the smc_power_mode_protection_config_t. An application should provide
 * the protect settings for all supported power modes on the chip. This
 * should be done at an early system level initialization stage. See the reference manual
 * for details. This register can only write once after the power reset. If the user has
 * only a single option to set,
 * either use this function or use the individual set function.
 *
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] protectConfig Configurations for the supported power mode protect settings
 *                      - See smc_power_mode_protection_config_t for details.
 */
void SMC_SetProtectionMode(SMC_Type * const baseAddr,
                           const smc_power_mode_protection_config_t * const protectConfig);

/*!
 * @brief Configures the the RUN mode control setting.
 *
 * This function sets the run mode settings, for example, normal run mode,
 * very lower power run mode, etc. See the smc_run_mode_t for supported run
 * mode on the chip family and the reference manual for details about the
 * run mode.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] runMode   Run mode setting defined in smc_run_mode_t
 */
static inline void SMC_SetRunModeControl(SMC_Type * const baseAddr,
                                         const smc_run_mode_t runMode)
{
    uint32_t regValue = baseAddr->PMCTRL;
    regValue &= ~(SMC_PMCTRL_RUNM_MASK);
    regValue |= SMC_PMCTRL_RUNM(runMode);
    baseAddr->PMCTRL = regValue;
}


/*!
 * @brief Enable the BIASEN bit.
 *
* This bit enables source and well biasing for the core logic in low power mode
 *
 * @param[in] baseAddr  Base address for current PMC instance.
 */
static inline void PMC_EnableBiasen(PMC_Type * const baseAddr )
{
    uint8_t regValue = baseAddr->REGSC;
    regValue &= ~(PMC_REGSC_BIASEN_MASK);

    regValue |= ((uint8_t)(PMC_REGSC_BIASEN(1U)));
    baseAddr->REGSC = regValue;
}

/*!
 * @brief Disable the BIASEN bit.
 *
 * Biasing disabled, core logic can run in full performance
 *
 * @param[in] baseAddr  Base address for current PMC instance.
 */
static inline void PMC_DisableBiasen(PMC_Type * const baseAddr )
{
    uint8_t regValue = baseAddr->REGSC;
    regValue &= ~(PMC_REGSC_BIASEN_MASK);
    baseAddr->REGSC = regValue;
}


/*!
 * @brief Configures  the STOP mode control setting.
 *
 * This function sets the stop mode settings, for example, normal stop mode,
 * very lower power stop mode, etc. See the smc_stop_mode_t for supported stop
 * mode on the chip family and the reference manual for details about the
 * stop mode.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] stopMode  Stop mode defined in smc_stop_mode_t
 */
static inline void SMC_SetStopModeControl(SMC_Type * const baseAddr,
                                          const smc_stop_mode_t stopMode)
{
    uint32_t regValue = baseAddr->PMCTRL;
    regValue &= ~(SMC_PMCTRL_STOPM_MASK);
    regValue |= SMC_PMCTRL_STOPM(stopMode);
    baseAddr->PMCTRL = regValue;
}

#if FEATURE_SMC_HAS_STOPO
/*!
 * @brief Configures the STOPO (Stop Option).
 *
 * It controls the type of the stop operation when STOPM=STOP. When entering Stop mode
 * from RUN mode, the PMC, SCG and flash remain fully powered, allowing the device to
 * wakeup almost instantaneously at the expense of higher power consumption. In STOP2,
 * only system clocks are gated allowing peripherals running on bus clock to remain fully
 * functional. In STOP1, both system and bus clocks are gated.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] option STOPO option setting defined in smc_stop_option_t
 */
static inline void SMC_SetStopOption(SMC_Type * const baseAddr,
                                     const smc_stop_option_t option)
{
    uint32_t regValue = baseAddr->STOPCTRL;
    regValue &= ~(SMC_STOPCTRL_STOPO_MASK);
    regValue |= SMC_STOPCTRL_STOPO(option);
    baseAddr->STOPCTRL = regValue;
}

#endif /* if FEATURE_SMC_HAS_STOPO */

#if FEATURE_SMC_HAS_PSTOPO

#error "Unimplemented"

#endif

/*!
 * @brief Gets the current power mode stat.
 *
 * This function returns the current power mode stat. Once application
 * switches the power mode, it should always check the stat to check whether it
 * runs into the specified mode or not. An application should check
 * this mode before switching to a different mode. The system requires that
 * only certain modes can switch to other specific modes. See the
 * reference manual for details and the power_mode_stat for information about
 * the power stat.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @return stat  Current power mode stat
 */
static inline power_mode_stat_t SMC_GetPowerModeStatus(const SMC_Type * const baseAddr)
{
    power_mode_stat_t retValue;
    uint32_t regValue = baseAddr->PMSTAT;
    regValue = (regValue & SMC_PMSTAT_PMSTAT_MASK) >> SMC_PMSTAT_PMSTAT_SHIFT;

    switch (regValue)
    {
        case 1UL:
            retValue = STAT_RUN;
            break;
        case 2UL:
            retValue = STAT_STOP;
            break;
        case 4UL:
            retValue = STAT_VLPR;
            break;
#if FEATURE_SMC_HAS_WAIT_VLPW
        case 8UL:
            retValue = STAT_VLPW;
            break;
#endif
        case 16UL:
            retValue = STAT_VLPS;
            break;
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
        case 128UL:
            retValue = STAT_HSRUN;
            break;
#endif
        case 255UL:
        default:
            retValue = STAT_INVALID;
            break;
    }

    return retValue;
}

/*@}*/

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*! @}*/

#endif /* POWER_SMC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
