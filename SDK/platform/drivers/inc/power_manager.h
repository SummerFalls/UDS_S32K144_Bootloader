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

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "device_registers.h"
#include "status.h"

/*
 * Include the cpu API header files.
 */
#if ((defined(S32K14x_SERIES)) || (defined(S32MTV_SERIES)) || (defined(S32K11x_SERIES)))
/* S32K144 power management API header file */
#include "../src/power/S32K1xx/power_manager_S32K1xx.h"
#elif (defined(S32V234_SERIES))
/* S32V234 Power Management Level API header file */
#include "../src/power/S32V234/power_manager_S32V234.h"
#elif (defined(MPC574x_SERIES) || defined(S32R_SERIES))
/* MPC5748G Power Management Level API header file */
#include "../src/power/MPC57xx/power_manager_MPC57xx.h"
#else
#error "No valid CPU defined!"
#endif

/*!
 * @file power_manager.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.3, Global typedef not referenced.
 * Although all symbols from power_manager.h are referenced in power_manager.c,
 * these symbols are not referenced in other drivers;
 */

/*!
 * @addtogroup power_manager
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Power manager policies.
 *
 * Defines whether the mode switch initiated by the POWER_SYS_SetMode() is agreed upon
 * (depending on the result of notification callbacks), or forced.
 * For POWER_MANAGER_POLICY_FORCIBLE the power mode is changed
 * regardless of the callback results, while for POWER_MANAGER_POLICY_AGREEMENT policy
 * any error code returned by one of the callbacks aborts the mode change.
 * See also POWER_SYS_SetMode() description.
 * Implements power_manager_policy_t_Class
 */
typedef enum
{
    POWER_MANAGER_POLICY_AGREEMENT,      /*!< Power mode is changed if all of the callbacks return success. */
    POWER_MANAGER_POLICY_FORCIBLE        /*!< Power mode is changed regardless of the result of callbacks. */
} power_manager_policy_t;

/*!
 * @brief The PM notification type. Used to notify registered callbacks.
 * Callback notifications can be invoked in following situations:
 *  - before a power mode change (Callback return value can affect POWER_SYS_SetMode()
 *    execution. Refer to the  POWER_SYS_SetMode() and power_manager_policy_t documentation).
 *  - after a successful change of the power mode.
 *  - after an unsuccessful attempt to switch power mode, in order to recover to a working state.
 * Implements power_manager_notify_t_Class
 */
typedef enum
{
    POWER_MANAGER_NOTIFY_RECOVER = 0x00U,  /*!< Notify IP to recover to previous work state.      */
    POWER_MANAGER_NOTIFY_BEFORE  = 0x01U,  /*!< Notify IP that the system will change the power setting.  */
    POWER_MANAGER_NOTIFY_AFTER   = 0x02U   /*!< Notify IP that the system has changed to a new power setting. */
} power_manager_notify_t;

/*!
 * @brief The callback type indicates when a callback will be invoked.
 *
 * Used in the callback configuration structures (power_manager_callback_user_config_t)
 * to specify when the registered callback will be called during power mode change initiated by
 * POWER_SYS_SetMode().
 *
 * Implements power_manager_callback_type_t_Class
 */
typedef enum
{
    POWER_MANAGER_CALLBACK_BEFORE       = 0x01U, /*!< Before callback. */
    POWER_MANAGER_CALLBACK_AFTER        = 0x02U, /*!< After callback. */
    POWER_MANAGER_CALLBACK_BEFORE_AFTER = 0x03U  /*!< Before-After callback. */
} power_manager_callback_type_t;

/*!
 * @brief Callback-specific data.
 *
 * Pointer to data of this type is passed during callback registration. The pointer is
 * part of the power_manager_callback_user_config_t structure and is passed to the callback during
 * power mode change notifications.
 * Implements power_manager_callback_data_t_Class
 */
typedef void power_manager_callback_data_t;

/*!
 * @brief Power mode user configuration structure.
 *
 * This structure defines power mode with additional power options.
 * This structure is implementation-defiend. Please refer to actual definition based on the
 * underlying HAL (SMC, MC_ME etc). Applications may define multiple power modes and
 * switch between them. A list of all defined power modes is passed to the Power manager during
 * initialization as an array of references to structures of this type (see POWER_SYS_Init()).
 * Power modes can be switched by calling POWER_SYS_SetMode(), which takes as argument the index
 * of the reqested power mode in the list passed during manager initialization.
 * The power mode currently in use can be retrieved by calling POWER_SYS_GetLastMode(), which provides
 * the index of the current power mode, or by calling POWER_SYS_GetLastModeConfig(), which provides a
 * pointer to the configuration structure of the current power mode.
 * The members of the power mode configuration structure depend on power options available
 * for a specific chip, and includes at least the power mode. The available power modes are chip-specific.
 * See power_manager_modes_t defined in the underlying HAL for a list of all supported modes.
 */

/*!
 * @brief Power notification structure passed to registered callback function
 *
 * Implements power_manager_notify_struct_t_Class
 */
typedef struct
{
    power_manager_user_config_t * targetPowerConfigPtr; /*!< Pointer to target power configuration */
    uint8_t targetPowerConfigIndex;    /*!< Target power configuration index. */
    power_manager_policy_t policy;     /*!< Power mode transition policy.          */
    power_manager_notify_t notifyType; /*!< Power mode notification type.          */
} power_manager_notify_struct_t;

/*!
 * @brief Callback prototype.
 *
 * Declaration of callback. It is common for all registered callbacks.
 * Function pointer of this type is part of power_manager_callback_user_config_t callback
 * configuration structure.
 * Depending on the callback type, the callback function is invoked during power mode change
 * (see POWER_SYS_SetMode()) before the mode change, after it, or in both cases to notify about
 * the change progress (see power_manager_callback_type_t). When called, the type of the notification
 * is passed as parameter along with a pointer to power mode configuration structure
 * (see power_manager_notify_struct_t) and any data passed during the callback registration (see
 * power_manager_callback_data_t).
 * When notified before a mode change, depending on the power mode change policy (see
 * power_manager_policy_t) the callback may deny the mode change by returning any error code other
 * than STATUS_SUCCESS (see POWER_SYS_SetMode()).
 * @param notify Notification structure.
 * @param dataPtr Callback data. Pointer to the data passed during callback registration. Intended to
 *  pass any driver or application data such as internal state information.
 * @return An error code or STATUS_SUCCESS.
 * Implements power_manager_callback_t_Class
 */
typedef status_t (* power_manager_callback_t)(power_manager_notify_struct_t * notify,
                                              power_manager_callback_data_t * dataPtr);

/*!
 * @brief callback configuration structure
 *
 * This structure holds configuration of callbacks passed
 * to the Power manager during its initialization.
 * Structures of this type are expected to be statically
 * allocated.
 * This structure contains following application-defined data:
 *  callback - pointer to the callback function
 *  callbackType - specifies when the callback is called
 *  callbackData - pointer to the data passed to the callback
 * Implements power_manager_callback_user_config_t_Class
 */
typedef struct
{
    power_manager_callback_t callbackFunction;
    power_manager_callback_type_t callbackType;
    power_manager_callback_data_t * callbackData;
} power_manager_callback_user_config_t;

/*!
 * @brief Power manager internal state structure.
 *
 * Power manager internal structure. Contains data necessary for Power manager proper
 * functionality. Stores references to registered power mode configurations,
 * callbacks, and other internal data.
 * This structure is statically allocated and initialized by POWER_SYS_Init().
 * Implements power_manager_state_t_Class
 */
typedef struct
{
    power_manager_user_config_t * (*configs)[];             /*!< Pointer to power configure table.*/
    uint8_t configsNumber;                                  /*!< Number of power configurations */
    power_manager_callback_user_config_t * (*staticCallbacks)[]; /*!< Pointer to callback table. */
    uint8_t staticCallbacksNumber;                          /*!< Max. number of callback configurations */
    uint8_t errorCallbackIndex;                             /*!< Index of callback returns error. */
    uint8_t currentConfig;                                  /*!< Index of current configuration.  */
} power_manager_state_t;

/*! @brief Power manager internal structure. */
extern power_manager_state_t gPowerManagerState;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Power manager initialization for operation.
 *
 * This function initializes the Power manager and its run-time state structure.
 * Pointer to an array of Power mode configuration structures needs to be passed
 * as a parameter along with a parameter specifying its size. At least one power mode
 * configuration is required. Optionally, pointer to the array of predefined
 * callbacks can be passed with its corresponding size parameter.
 * For details about callbacks, refer to the power_manager_callback_user_config_t.
 * As Power manager stores only pointers to arrays of these structures, they need
 * to exist and be valid for the entire life cycle of Power manager.
 *
 * @param[in] powerConfigsPtr A pointer to an array of pointers to all power
 *  configurations which will be handled by Power manager.
 * @param[in] configsNumber Number of power configurations. Size of powerConfigsPtr
 *  array.
 * @param[in] callbacksPtr A pointer to an array of pointers to callback configurations.
 *  If there are no callbacks to register during Power manager initialization, use NULL value.
 * @param[in] callbacksNumber Number of registered callbacks. Size of callbacksPtr
 *  array.
 * @return An error code or STATUS_SUCCESS.
 */
status_t POWER_SYS_Init(power_manager_user_config_t * (*powerConfigsPtr)[],
                        uint8_t configsNumber,
                        power_manager_callback_user_config_t * (*callbacksPtr)[],
                        uint8_t callbacksNumber);

/*!
 * @brief This function deinitializes the Power manager.
 *
 * @return An error code or STATUS_SUCCESS.
 */
status_t POWER_SYS_Deinit(void);

/*!
 * @brief This function configures the power mode.
 *
 * This function switches to one of the defined power modes. Requested mode number is passed
 * as an input parameter. This function notifies all registered callback functions before
 * the mode change (using  POWER_MANAGER_CALLBACK_BEFORE set as callback type parameter),
 * sets specific power options defined in the power mode configuration and enters the specified
 * mode. In case of run modes (for example, Run, Very low power run, or High speed run), this function
 * also invokes all registered callbacks after the mode change (using POWER_MANAGER_CALLBACK_AFTER).
 * In case of sleep or deep sleep modes, if the requested mode is not exited through
 * a reset, these notifications are sent after the core wakes up.
 * Callbacks are invoked in the following order: All registered callbacks are notified
 * ordered by index in the callbacks array (see callbacksPtr parameter of POWER_SYS_Init()).
 * The same order is used for before and after switch notifications.
 * The notifications before the power mode switch can be used to obtain confirmation about
 * the change from registered callbacks. If any registered callback denies the power
 * mode change, further execution of this function depends on mode change policy: the mode
 * change is either forced(POWER_MANAGER_POLICY_FORCIBLE) or aborted(POWER_MANAGER_POLICY_AGREEMENT).
 * When mode change is forced, the results of the before switch notifications are ignored. If
 * agreement is requested, in case any callback returns an error code then further
 * before switch notifications are cancelled and all already notified callbacks are re-invoked
 * with POWER_MANAGER_CALLBACK_AFTER set as callback type parameter. The index of the callback
 * which returned error code during pre-switch notifications is stored and can be obtained by using
 * POWER_SYS_GetErrorCallback(). Any error codes during callbacks re-invocation (recover phase) are ignored.
 * POWER_SYS_SetMode() returns an error code denoting the phase in which a callback failed.
 * It is possible to enter any mode supported by the processor. Refer to the chip reference manual
 * for the list of available power modes. If it is necessary to switch into an intermediate power mode prior to
 * entering the requested mode (for example, when switching from Run into Very low power wait through Very low
 * power run mode), then the intermediate mode is entered without invoking the callback mechanism.
 *
 * @param[in] powerModeIndex Requested power mode represented as an index into
 * array of user-defined power mode configurations passed to the POWER_SYS_Init().
 * @param[in] policy Transaction policy
 * @return An error code or STATUS_SUCCESS.
 */
status_t POWER_SYS_SetMode(uint8_t powerModeIndex,
                           power_manager_policy_t policy);

/*!
 * @brief This function returns the last successfully set power mode.
 *
 * This function returns index of power mode which was last set using POWER_SYS_SetMode().
 * If the power mode was entered even though some of the registered callbacks denied the mode change,
 * or if any of the callbacks invoked after the entering/restoring run mode failed, then the return
 * code of this function has STATUS_ERROR value.
 *
 * @param[out] powerModeIndexPtr Power mode which has been set represented as an index into array of power mode
 * configurations passed to the POWER_SYS_Init().
 * @return An error code or STATUS_SUCCESS.
 */
status_t POWER_SYS_GetLastMode(uint8_t * powerModeIndexPtr);

/*!
 * @brief This function returns the user configuration structure of the last successfully set power mode.
 *
 * This function returns a pointer to configuration structure which was last set using POWER_SYS_SetMode().
 * If the current power mode was entered even though some of the registered callbacks denied
 * the mode change, or if any of the callbacks invoked after the entering/restoring run mode failed, then
 * the return code of this function has STATUS_ERROR value.
 *
 * @param[out] powerModePtr Pointer to power mode configuration structure of the last set power mode.
 * @return An error code or STATUS_SUCCESS.
 */
status_t POWER_SYS_GetLastModeConfig(power_manager_user_config_t ** powerModePtr);

/*!
 * @brief This function returns currently running power mode.
 *
 * This function reads hardware settings and returns currently running power mode.
 *
 * @return Currently used run power mode.
 */
power_manager_modes_t POWER_SYS_GetCurrentMode(void);

/*!
 * @brief This function returns the last failed notification callback.
 *
 * This function returns the index of the last callback that failed during the power mode switch when
 * POWER_SYS_SetMode() was called. The returned value represents the index in the array of registered callbacks.
 * If the last POWER_SYS_SetMode() call ended successfully, a value equal to the number of registered callbacks
 * is returned.
 *
 * @return Callback index of last failed callback or value equal to callbacks count.
 */
uint8_t POWER_SYS_GetErrorCallbackIndex(void);

/*!
 * @brief This function returns the callback configuration structure for the last failed notification.
 *
 * This function returns a pointer to configuration structure of the last callback that failed during
 * the power mode switch when POWER_SYS_SetMode() was called.
 * If the last POWER_SYS_SetMode() call ended successfully, a NULL value is returned.
 *
 * @return Pointer to the callback configuration which returns error.
 */
power_manager_callback_user_config_t * POWER_SYS_GetErrorCallback(void);

/*!
 * @brief This function returns the default power_manager configuration structure.
 *
 * This function returns a pointer of the power_manager configuration structure.
 * All structure members have default value when CPU is default power mode.
 *
 */
void POWER_SYS_GetDefaultConfig(power_manager_user_config_t * const config);

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* POWER_MANAGER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
