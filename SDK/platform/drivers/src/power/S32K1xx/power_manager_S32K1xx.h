/*
 * Copyright (c) 2014-2016 Freescale Semiconductor, Inc.
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

#ifndef POWER_MANAGER_S32K1XX_H
#define POWER_MANAGER_S32K1XX_H

/*!
 * @file clock_manager_S32K1xx.h
 */

#include "device_registers.h"
#include "status.h"

/*!
 * @ingroup power_manager
 * @defgroup power_s32k1xx
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @brief Power modes enumeration.
 *
 * Defines power modes. Used in the power mode configuration structure
 * (power_manager_user_config_t). From ARM core perspective, Power modes
 * can be generally divided into run modes (High speed run, Run and
 * Very low power run), sleep (Wait and Very low power wait) and deep sleep modes
 * (all Stop modes).
 * List of power modes supported by specific chip along with requirements for entering
 * and exiting of these modes can be found in chip documentation.
 * List of all supported power modes:\n
 *  \li POWER_MANAGER_HSRUN - High speed run mode.
 *  \li POWER_MANAGER_RUN - Run mode.
 *  \li POWER_MANAGER_VLPR - Very low power run mode.
 *  \li POWER_MANAGER_WAIT - Wait mode.
 *  \li POWER_MANAGER_VLPW - Very low power wait mode.
 *  \li POWER_MANAGER_PSTOP1 - Partial stop 1 mode.
 *  \li POWER_MANAGER_PSTOP2 - Partial stop 2 mode.
 *  \li POWER_MANAGER_PSTOP1 - Stop 1 mode.
 *  \li POWER_MANAGER_PSTOP2 - Stop 2 mode.
 *  \li POWER_MANAGER_VLPS - Very low power stop mode.
 * Implements power_manager_modes_t_Class
 */
typedef enum
{
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
    POWER_MANAGER_HSRUN,            /*!< High speed run mode.  */
#endif
    POWER_MANAGER_RUN,              /*!< Run mode. */
    POWER_MANAGER_VLPR,             /*!< Very low power run mode.  */
#if FEATURE_SMC_HAS_WAIT_VLPW
    POWER_MANAGER_WAIT,             /*!< Wait mode.  */
    POWER_MANAGER_VLPW,             /*!< Very low power wait mode.  */
#endif
#if FEATURE_SMC_HAS_PSTOPO
    POWER_MANAGER_PSTOP1,           /*!< Partial stop 1 mode. */
    POWER_MANAGER_PSTOP2,           /*!< Partial stop 2 mode. */
#endif
#if FEATURE_SMC_HAS_STOPO
    POWER_MANAGER_STOP1,           /*!< Stop 1 mode. */
    POWER_MANAGER_STOP2,           /*!< Stop 2 mode. */
#endif
    POWER_MANAGER_VLPS,             /*!< Very low power stop mode.  */
    POWER_MANAGER_MAX
} power_manager_modes_t;

/*!
 * @brief Power mode user configuration structure.
 *
 * List of power mode configuration structure members depends on power options available
 * for the specific chip. Complete list contains:
 * mode - S32K power mode. List of available modes is chip-specific. See power_manager_modes_t
 * list of modes.
 * sleepOnExitOption - Controls whether the sleep-on-exit option value is used(when set to true)
 * or ignored(when set to false). See sleepOnExitValue.
 * sleepOnExitValue - When set to true, ARM core returns to sleep (S32K wait modes) or deep sleep
 * state (S32K stop modes) after interrupt service finishes. When set to false, core stays
 * woken-up.
 * Implements power_manager_user_config_t_Class
 */
typedef struct
{
    power_manager_modes_t powerMode;
    bool sleepOnExitValue;
} power_manager_user_config_t;

/*!
 * @brief Power Modes in PMSTAT
 *
 */
typedef enum
{
    STAT_RUN     = 0x01,              /*!< 0000_0001 - Current power mode is RUN*/
    STAT_STOP    = 0x02,              /*!< 0000_0010 - Current power mode is STOP*/
    STAT_VLPR    = 0x04,              /*!< 0000_0100 - Current power mode is VLPR*/
    STAT_VLPW    = 0x08,              /*!< 0000_1000 - Current power mode is VLPW*/
    STAT_VLPS    = 0x10,              /*!< 0001_0000 - Current power mode is VLPS*/
    STAT_HSRUN   = 0x80,              /*!< 1000_0000 - Current power mode is HSRUN*/
    STAT_INVALID = 0xFF               /*!< 1111_1111 - Non-existing power mode*/
} power_mode_stat_t;

/*!
 * @brief Run mode definition
 *
 */
typedef enum
{
    SMC_RUN,                                /*!< normal RUN mode*/
    SMC_RESERVED_RUN, SMC_VLPR,             /*!< Very-Low-Power RUN mode*/
    SMC_HSRUN                               /*!< High Speed Run mode (HSRUN)*/
} smc_run_mode_t;

/*!
 * @brief Stop mode definition
 *
 */
typedef enum
{
    SMC_STOP            = 0U,    /*!< Normal STOP mode*/
    SMC_RESERVED_STOP1  = 1U,    /*!< Reserved*/
    SMC_VLPS            = 2U     /*!< Very-Low-Power STOP mode*/
} smc_stop_mode_t;

/*!
 * @brief STOP option
 *
 */
typedef enum
{
    SMC_STOP_RESERVED = 0x00,               /*!< Reserved stop mode */
    SMC_STOP1         = 0x01,               /*!< Stop with both system and bus clocks disabled */
    SMC_STOP2         = 0x02                /*!< Stop with system clock disabled and bus clock enabled */
} smc_stop_option_t;

/*!
 * @brief Power mode protection configuration
 *
 */
typedef struct
{
    bool vlpProt;            /*!< VLP protect*/
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
    bool hsrunProt;          /*!< HSRUN protect */
#endif
} smc_power_mode_protection_config_t;

/*!
 * @brief Power mode control configuration used for calling the SMC_SYS_SetPowerMode API
 *
 */
typedef struct
{
    power_manager_modes_t powerModeName;    /*!< Power mode(enum), see power_manager_modes_t */
#if FEATURE_SMC_HAS_STOPO
    smc_stop_option_t stopOptionValue;      /*!< STOPO option(enum), see smc_stop_option_t */
#endif
#if FEATURE_SMC_HAS_PSTOPO
    bool pStopOption;                       /*!< If PSTOPO option is needed */
    smc_stop_option_t pStopOptionValue;     /*!< PSTOPO option(enum), see smc_stop_option_t */
#endif
} smc_power_mode_config_t;

/*!
 * @brief System Reset Source Name definitions
 * Implements rcm_source_names_t_Class
 */
typedef enum
{
    RCM_LOW_VOLT_DETECT      = 1U,             /*!< Low voltage detect reset */
    RCM_LOSS_OF_CLK          = 2U,       /*!< Loss of clock reset */
    RCM_LOSS_OF_LOCK         = 3U,       /*!< Loss of lock reset */
#if FEATURE_RCM_HAS_CMU_LOSS_OF_CLOCK
    RCM_CMU_LOC              = 4U,        /*!< CMU Loss of lock reset */
#endif
    RCM_WATCH_DOG            = 5U,        /*!< Watch dog reset */
    RCM_EXTERNAL_PIN         = 6U,       /*!< External pin reset */
    RCM_POWER_ON             = 7U,       /*!< Power on reset */
    RCM_SJTAG                = 8U,       /*!< JTAG generated reset */
    RCM_CORE_LOCKUP          = 9U,       /*!< core lockup reset */
    RCM_SOFTWARE             = 10U,       /*!< Software reset */
    RCM_SMDM_AP              = 11U,       /*!< MDM-AP system reset */
    RCM_STOP_MODE_ACK_ERR    = 13U,       /*!< Stop mode ack error reset */
    RCM_SRC_NAME_MAX
} rcm_source_names_t;
/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief This function implementation-specific configuration of power modes.
 *
 * This function performs the actual implementation-specific initialization based on the provided power mode configurations.
 * In addition, This function get all clock source were enabled. This one was used for update init clock when CPU 
    jump from very low power mode to run or high speed run mode.
 *
 * @return Operation status
 *        - STATUS_SUCCESS: Operation was successful.
 *        - STATUS_ERROR: Operation failed.
 */
status_t POWER_SYS_DoInit(void);

/*!
 * @brief This function implementation-specific de-initialization of power manager.
 *
 * This function performs the actual implementation-specific de-initialization.
 *
 * @return Operation status
 *        - STATUS_SUCCESS: Operation was successful.
 *        - STATUS_ERROR: Operation failed.
 */
status_t POWER_SYS_DoDeinit(void);

/*!
 * @brief This function configures the power mode.
 *
 * This function performs the actual implementation-specific logic to switch to one of the defined power modes.
 *
 * @param configPtr: Pointer to user configuration structure
 * @return Operation status
 *        - STATUS_SUCCESS: Operation was successful.
 *        - STATUS_MCU_TRANSITION_FAILED: Operation failed.
 */
status_t POWER_SYS_DoSetMode(const power_manager_user_config_t * const configPtr);

/*!
 * @brief Gets the reset source status
 *
 * This function gets the current reset source status for a specified source.
 *
 * @param[in] baseAddr     Register base address of RCM
 * @param[in] srcName      reset source name
 * @return status          True or false for specified reset source
 */
bool POWER_SYS_GetResetSrcStatusCmd(const RCM_Type * const baseAddr , const rcm_source_names_t srcName);

/*!
 * @brief Gets the default power_manager configuration structure.
 *
 * This function gets the power_manager configuration structure of the default power mode.
 *
 * @param[out] defaultConfig : Pointer to power mode configuration structure of the default power mode.
 */
static inline void POWER_SYS_DoGetDefaultConfig(power_manager_user_config_t * const defaultConfig)
{
    defaultConfig->powerMode = POWER_MANAGER_RUN;    /*!< Power manager mode  */
    defaultConfig->sleepOnExitValue = false;         /*!< Sleep on exit value */
}

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* POWER_MANAGER_S32K1XX_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
