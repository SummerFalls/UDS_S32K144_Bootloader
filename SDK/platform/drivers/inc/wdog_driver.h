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
 * @file wdog_driver.h
 */

#ifndef WDOG_DRIVER_H
#define WDOG_DRIVER_H

#include "status.h"
#include "interrupt_manager.h"

/*!
 * @defgroup wdog_drv WDOG Driver
 * @ingroup wdog
 * @brief Watchdog Timer Peripheral Driver.
 * @addtogroup wdog_drv
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Enumerations.
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @brief Clock sources for the WDOG.
 * Implements : wdog_clk_source_t_Class
 */
typedef enum
{
    WDOG_BUS_CLOCK                        = 0x00U, /*!< Bus clock */
    WDOG_LPO_CLOCK                        = 0x01U, /*!< LPO clock */
    WDOG_SOSC_CLOCK                       = 0x02U, /*!< SOSC clock */
    WDOG_SIRC_CLOCK                       = 0x03U  /*!< SIRC clock */
} wdog_clk_source_t;

/*!
 * @brief Test modes for the WDOG.
 * Implements : wdog_test_mode_t_Class
 */
typedef enum
{
    WDOG_TST_DISABLED                     = 0x00U, /*!< Test mode disabled */
    WDOG_TST_USER                         = 0x01U, /*!< User mode enabled. (Test mode disabled.) */
    WDOG_TST_LOW                          = 0x02U, /*!< Test mode enabled, only the low byte is used. */
    WDOG_TST_HIGH                         = 0x03U  /*!< Test mode enabled, only the high byte is used. */
} wdog_test_mode_t;

/*!
 * @brief set modes for the WDOG.
 * Implements : wdog_set_mode_t_Class
 */
typedef enum
{
    WDOG_DEBUG_MODE                        = 0x00U, /*!< Debug mode */
    WDOG_WAIT_MODE                         = 0x01U, /*!< Wait mode */
    WDOG_STOP_MODE                         = 0x02U  /*!< Stop mode */
} wdog_set_mode_t;

/*!
 * @brief WDOG option mode configuration structure
 * Implements : wdog_op_mode_t_Class
 */
typedef struct
{
    bool wait;  /*!< Wait mode */
    bool stop;  /*!< Stop mode */
    bool debug; /*!< Debug mode */
} wdog_op_mode_t;

/*!
 * @brief WDOG user configuration structure
 * Implements : wdog_user_config_t_Class
 */
typedef struct
{
    wdog_clk_source_t    clkSource;       /*!< The clock source of the WDOG */
    wdog_op_mode_t       opMode;          /*!< The modes in which the WDOG is functional */
    bool                 updateEnable;    /*!< If true, further updates of the WDOG are enabled */
    bool                 intEnable;       /*!< If true, an interrupt request is generated before reset */
    bool                 winEnable;       /*!< If true, window mode is enabled */
    uint16_t             windowValue;     /*!< The window value */
    uint16_t             timeoutValue;    /*!< The timeout value */
    bool                 prescalerEnable; /*!< If true, a fixed 256 prescaling of the counter reference clock is enabled */
} wdog_user_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @name WDOG Driver API
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the WDOG driver.
 *
 * @param[in] instance WDOG peripheral instance number
 * @param[in] userConfigPtr pointer to the WDOG user configuration structure
 * @return operation status
 *        - STATUS_SUCCESS: Operation was successful.
 *        - STATUS_ERROR: Operation failed. Possible causes: previous
 *        clock source or the one specified in the configuration structure is
 *        disabled; WDOG configuration updates are not allowed; WDOG instance has been initialized before;
 *        If window mode enabled and window value greater than or equal to the timeout value.
 */
status_t WDOG_DRV_Init(uint32_t instance,
                       const wdog_user_config_t * userConfigPtr);

/*!
 * @brief De-initializes the WDOG driver
 *
 * @param[in] instance  WDOG peripheral instance number
 * @return operation status
 *        - STATUS_SUCCESS: if allowed reconfigures WDOG module and de-initializes successful.
 *        - STATUS_ERROR: Operation failed. Possible causes: failed to
 *          WDOG configuration updates not allowed.
 */
status_t WDOG_DRV_Deinit(uint32_t instance);

/*!
 * @brief Gets the current configuration of the WDOG.
 *
 * @param[in] instance  WDOG peripheral instance number
 * @param[out] configures  the current configuration
 */
void WDOG_DRV_GetConfig(uint32_t instance,
                        wdog_user_config_t * const config);

/*!
 * @brief Gets default configuration of the WDOG.
 *
 * @param[out] configures the default configuration
 */
void WDOG_DRV_GetDefaultConfig(wdog_user_config_t * const config);

/*!
 * @brief  Enables/Disables the WDOG timeout interrupt and sets a function to be
 * called when a timeout interrupt is received, before reset.
 *
 * @param[in] instance WDOG peripheral instance number
 * @param[in] enable enable/disable interrupt
 * @return operation status
 *        - STATUS_SUCCESS: if allowed reconfigures WDOG timeout interrupt.
 *        - STATUS_ERROR: Operation failed. Possible causes: failed to
 *          WDOG configuration updates not allowed.
 */
status_t WDOG_DRV_SetInt(uint32_t instance,
                         bool enable);

/*!
 * @brief Clear interrupt flag of the WDOG.
 *
 * @param[in] instance WDOG peripheral instance number
 */
void WDOG_DRV_ClearIntFlag(uint32_t instance);

/*!
 * @brief Refreshes the WDOG counter.
 *
 * @param[in] instance WDOG peripheral instance number
 */
void WDOG_DRV_Trigger(uint32_t instance);

/*!
 * @brief Gets the value of the WDOG  counter.
 *
 * @param[in] instance WDOG peripheral instance number.
 * @return the value of the WDOG counter.
 */
uint16_t WDOG_DRV_GetCounter(uint32_t instance);

/*!
 * @brief Set window mode and window value of the WDOG.
 *
 * This function set window mode, window value is set when window mode enabled.
 *
 * @param[in] instance WDOG peripheral instance number.
 * @param[in] enable enable/disable window mode and window value.
 * @param[in] windowvalue the value of the WDOG window.
 * @return operation status
 *        - STATUS_SUCCESS: if allowed reconfigures window value success.
 *        - STATUS_ERROR: Operation failed. Possible causes: failed to
 *          WDOG configuration updates not allowed.
 */
status_t WDOG_DRV_SetWindow(uint32_t instance,
                            bool enable,
                            uint16_t windowvalue);

/*!
 * @brief Sets the mode operation of the WDOG.
 *
 * This function changes the mode operation of the WDOG.
 *
 * @param[in] instance WDOG peripheral instance number.
 * @param[in] enable enable/disable mode of the WDOG.
 * @param[in] Setmode select mode of the WDOG.
 * @return operation status
 *        - STATUS_SUCCESS: if allowed reconfigures mode operation of the WDOG.
 *        - STATUS_ERROR: Operation failed. Possible causes: failed to
 *          WDOG configuration updates not allowed.
 */
status_t WDOG_DRV_SetMode(uint32_t instance,
                          bool enable,
                          wdog_set_mode_t Setmode);

/*!
 * @brief Sets the value of the WDOG timeout.
 *
 * This function sets the value of the WDOG timeout.
 *
 * @param[in] instance WDOG peripheral instance number.
 * @param[in] timeout the value of the WDOG timeout.
 * @return operation status
 *        - STATUS_SUCCESS: if allowed reconfigures WDOG timeout.
 *        - STATUS_ERROR: Operation failed. Possible causes: failed to
 *          WDOG configuration updates not allowed.
 */
status_t WDOG_DRV_SetTimeout(uint32_t instance,
                             uint16_t timeout);

/*!
 * @brief Changes the WDOG test mode.
 *
 * This function changes the test mode of the WDOG. If the WDOG is tested in
 * mode, software should set this field to 0x01U in order to indicate that the
 * WDOG is functioning normally.
 *
 * @param[in] instance WDOG peripheral instance number
 * @param[in] testMode Test modes for the WDOG.
 * @return operation status
 *        - STATUS_SUCCESS: if allowed reconfigures WDOG test mode.
 *        - STATUS_ERROR: Operation failed. Possible causes: failed to
 *          WDOG configuration updates not allowed.
 */
status_t WDOG_DRV_SetTestMode(uint32_t instance,
                              wdog_test_mode_t testMode);

/*!
 * @brief Gets the WDOG test mode.
 *
 * This function verifies the test mode of the WDOG.
 *
 * @param[in] instance: WDOG peripheral instance number
 * @return Test modes for the WDOG
 */
wdog_test_mode_t WDOG_DRV_GetTestMode(uint32_t instance);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WDOG_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
