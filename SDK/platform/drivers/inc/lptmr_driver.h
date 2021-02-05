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

#ifndef LPTMR_DRIVER_H
#define LPTMR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "device_registers.h"
/*! @file lptmr_driver.h*/

/*!
 * @addtogroup lptmr_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Pulse Counter Input selection
 *  Implements : lptmr_pinselect_t_Class
 */
typedef enum {
    LPTMR_PINSELECT_TRGMUX  = 0x00u, /*!< Count pulses from TRGMUX trigger */
#if FEATURE_LPTMR_HAS_INPUT_ALT1_SELECTION
    LPTMR_PINSELECT_ALT1    = 0x01u, /*!< Count pulses from pin alternative 1 */
#endif
    LPTMR_PINSELECT_ALT2    = 0x02u, /*!< Count pulses from pin alternative 2 */
    LPTMR_PINSELECT_ALT3    = 0x03u  /*!< Count pulses from pin alternative 3 */
} lptmr_pinselect_t;

/*! @brief Pulse Counter input polarity
 *  Implements : lptmr_pinpolarity_t_Class
 */
typedef enum {
    LPTMR_PINPOLARITY_RISING    = 0u, /*!< Count pulse on rising edge */
    LPTMR_PINPOLARITY_FALLING   = 1u  /*!< Count pulse on falling edge */
} lptmr_pinpolarity_t;

/*! @brief Work Mode
 *  Implements : lptmr_workmode_t_Class
 */
typedef enum {
    LPTMR_WORKMODE_TIMER        = 0u, /*!< Timer */
    LPTMR_WORKMODE_PULSECOUNTER = 1u  /*!< Pulse counter */
} lptmr_workmode_t;

/*! @brief Prescaler Selection
 *  Implements : lptmr_prescaler_t_Class
 */
typedef enum {
    LPTMR_PRESCALE_2                        = 0x00u, /*!< Timer mode: prescaler 2, Glitch filter mode: invalid */
    LPTMR_PRESCALE_4_GLITCHFILTER_2         = 0x01u, /*!< Timer mode: prescaler 4, Glitch filter mode: 2 clocks */
    LPTMR_PRESCALE_8_GLITCHFILTER_4         = 0x02u, /*!< Timer mode: prescaler 8, Glitch filter mode: 4 clocks */
    LPTMR_PRESCALE_16_GLITCHFILTER_8        = 0x03u, /*!< Timer mode: prescaler 16, Glitch filter mode: 8 clocks */
    LPTMR_PRESCALE_32_GLITCHFILTER_16       = 0x04u, /*!< Timer mode: prescaler 32, Glitch filter mode: 16 clocks */
    LPTMR_PRESCALE_64_GLITCHFILTER_32       = 0x05u, /*!< Timer mode: prescaler 64, Glitch filter mode: 32 clocks */
    LPTMR_PRESCALE_128_GLITCHFILTER_64      = 0x06u, /*!< Timer mode: prescaler 128, Glitch filter mode: 64 clocks */
    LPTMR_PRESCALE_256_GLITCHFILTER_128     = 0x07u, /*!< Timer mode: prescaler 256, Glitch filter mode: 128 clocks */
    LPTMR_PRESCALE_512_GLITCHFILTER_256     = 0x08u, /*!< Timer mode: prescaler 512, Glitch filter mode: 256 clocks */
    LPTMR_PRESCALE_1024_GLITCHFILTER_512    = 0x09u, /*!< Timer mode: prescaler 1024, Glitch filter mode: 512 clocks */
    LPTMR_PRESCALE_2048_GLITCHFILTER_1024   = 0x0Au, /*!< Timer mode: prescaler 2048, Glitch filter mode: 1024 clocks */
    LPTMR_PRESCALE_4096_GLITCHFILTER_2048   = 0x0Bu, /*!< Timer mode: prescaler 4096, Glitch filter mode: 2048 clocks */
    LPTMR_PRESCALE_8192_GLITCHFILTER_4096   = 0x0Cu, /*!< Timer mode: prescaler 8192, Glitch filter mode: 4096 clocks */
    LPTMR_PRESCALE_16384_GLITCHFILTER_8192  = 0x0Du, /*!< Timer mode: prescaler 16384, Glitch filter mode: 8192 clocks */
    LPTMR_PRESCALE_32768_GLITCHFILTER_16384 = 0x0Eu, /*!< Timer mode: prescaler 32768, Glitch filter mode: 16384 clocks */
    LPTMR_PRESCALE_65536_GLITCHFILTER_32768 = 0x0Fu  /*!< Timer mode: prescaler 65536, Glitch filter mode: 32768 clocks */
} lptmr_prescaler_t;

/*! @brief Clock Source selection
 *  Implements : lptmr_clocksource_t_Class
 */
typedef enum {
    LPTMR_CLOCKSOURCE_SIRCDIV2  = 0x00u, /*!< SIRC clock */
    LPTMR_CLOCKSOURCE_1KHZ_LPO  = 0x01u, /*!< 1kHz LPO clock */
    LPTMR_CLOCKSOURCE_RTC       = 0x02u, /*!< RTC clock */
    LPTMR_CLOCKSOURCE_PCC       = 0x03u  /*!< PCC configured clock */
} lptmr_clocksource_t;

/*!
 * @brief Defines the LPTMR counter units available for configuring or reading the timer compare value.
 *
 * Implements : lptmr_counter_units_t_Class
 */
typedef enum
{
    LPTMR_COUNTER_UNITS_TICKS           = 0x00U,
    LPTMR_COUNTER_UNITS_MICROSECONDS    = 0x01U
} lptmr_counter_units_t;

/*!
 * @brief Defines the configuration structure for LPTMR.
 *
 * Implements : lptmr_config_t_Class
 */
typedef struct
{
    /* General parameters */
    bool dmaRequest;                    /*!< Enable/Disable DMA requests */
    bool interruptEnable;               /*!< Enable/Disable Interrupt */
    bool freeRun;                       /*!< Enable/Disable Free Running Mode */
    lptmr_workmode_t workMode;          /*!< Time/Pulse Counter Mode */
    /* Counter parameters */
    lptmr_clocksource_t clockSelect;    /*!< Clock selection for Timer/Glitch filter */
    lptmr_prescaler_t prescaler;        /*!< Prescaler Selection */
    bool bypassPrescaler;               /*!< Enable/Disable prescaler bypass */
    uint32_t compareValue;              /*!< Compare value */
    lptmr_counter_units_t counterUnits; /*!< Compare value units */
    /* Pulse Counter specific parameters */
    lptmr_pinselect_t pinSelect;        /*!< Pin selection for Pulse-Counter */
    lptmr_pinpolarity_t pinPolarity;    /*!< Pin Polarity for Pulse-Counter */
} lptmr_config_t;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
/*!
 * @name LPTMR Driver Functions
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize a configuration structure with default values.
 *
 * @param[out] config - Pointer to the configuration structure to be initialized
 */
void LPTMR_DRV_InitConfigStruct(lptmr_config_t * const config);

/*!
 * @brief Initialize a LPTMR instance with values from an input configuration structure.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS) the function will
 * automatically configure the timer for the input compareValue in microseconds.
 * The input parameters for 'prescaler' and 'bypassPrescaler' will be ignored - their values
 * will be adapted by the function, to best fit the input compareValue
 * (in microseconds) for the operating clock frequency.
 *
 * LPTMR_COUNTER_UNITS_MICROSECONDS may only be used for LPTMR_WORKMODE_TIMER mode.
 * Otherwise the function shall not convert 'compareValue' in ticks
 * and this is likely to cause erroneous behavior.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_TICKS) the function will use the
 * 'prescaler' and 'bypassPrescaler' provided in the input configuration structure.
 *
 * @param[in] instance     - LPTMR instance number
 * @param[in] config       - Pointer to the input configuration structure
 * @param[in] startCounter - Flag for starting the counter immediately after configuration
 */
void LPTMR_DRV_Init(const uint32_t instance,
                    const lptmr_config_t * const config,
                    const bool startCounter);

/*!
 * @brief Configure a LPTMR instance.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_MICROSECONDS) the function will
 * automatically configure the timer for the input compareValue in microseconds.
 * The input parameters for 'prescaler' and 'bypassPrescaler' will be ignored - their values
 * will be adapted by the function, to best fit the input compareValue
 * (in microseconds) for the operating clock frequency.
 *
 * LPTMR_COUNTER_UNITS_MICROSECONDS may only be used for LPTMR_WORKMODE_TIMER mode.
 * Otherwise the function shall not convert 'compareValue' in ticks
 * and this is likely to cause erroneous behavior.
 *
 * When (counterUnits == LPTMR_COUNTER_UNITS_TICKS) the function will use the
 * 'prescaler' and 'bypassPrescaler' provided in the input configuration structure.
 *
 * @param[in] instance - LPTMR instance number
 * @param[in] config   - Pointer to the input configuration structure
 */
void LPTMR_DRV_SetConfig(const uint32_t instance,
                         const lptmr_config_t * const config);

/*!
 * @brief Get the current configuration of a LPTMR instance.
 *
 * @param[in] instance - LPTMR instance number
 * @param[out] config  - Pointer to the output configuration structure
 */
void LPTMR_DRV_GetConfig(const uint32_t instance,
                         lptmr_config_t * const config);

/*!
 * @brief De-initialize a LPTMR instance
 *
 * @param[in] instance - LPTMR instance number
 */
void LPTMR_DRV_Deinit(const uint32_t instance);

/*!
 * @brief Set the compare value in counter tick units, for a LPTMR instance.
 *
 * @param[in] instance            - LPTMR instance number
 * @param[in] compareValueByCount - The compare value in counter ticks, to be written
 * @return Operation status:
 * - STATUS_SUCCESS: completed successfully
 * - STATUS_ERROR: cannot reconfigure compare value (TCF not set)
 * - STATUS_TIMEOUT: compare value greater then current counter value
 */
status_t LPTMR_DRV_SetCompareValueByCount(const uint32_t instance,
                                          const uint16_t compareValueByCount);

/*!
 * @brief Get the compare value in counter tick units, of a LPTMR instance.
 *
 * @param[in] instance             - LPTMR instance number
 * @param[out] compareValueByCount - Pointer to current compare value, in counter ticks
 */
void LPTMR_DRV_GetCompareValueByCount(const uint32_t instance,
                                      uint16_t * const compareValueByCount);

/*!
 * @brief Set the compare value for Timer Mode in microseconds, for a LPTMR instance.
 *
 * @param[in] instance       - LPTMR peripheral instance number
 * @param[in] compareValueUs - Compare value in microseconds
 * @return Operation status:
 * - STATUS_SUCCESS: completed successfully
 * - STATUS_ERROR: cannot reconfigure compare value
 * - STATUS_TIMEOUT: compare value greater then current counter value
 */
status_t LPTMR_DRV_SetCompareValueByUs(const uint32_t instance,
                                       const uint32_t compareValueUs);

/*!
 * @brief Get the compare value in microseconds, of a LPTMR instance.
 *
 * @param[in] instance        - LPTMR instance number
 * @param[out] compareValueUs - Pointer to current compare value, in microseconds
 */
void LPTMR_DRV_GetCompareValueByUs(const uint32_t instance,
                                   uint32_t * const compareValueUs);

/*!
 * @brief Get the current state of the Compare Flag of a LPTMR instance.
 *
 * @param[in] instance - LPTMR instance number
 * @return The state of the Compare Flag
 */
bool LPTMR_DRV_GetCompareFlag(const uint32_t instance);

/*!
 * @brief Clear the Compare Flag of a LPTMR instance.
 *
 * @param[in] instance - LPTMR instance number
 */
void LPTMR_DRV_ClearCompareFlag(const uint32_t instance);

/*!
 * @brief Get the run state of a LPTMR instance.
 *
 * @param[in] instance - LPTMR instance number
 * @return The run state of the LPTMR instance:
 *  - true: Timer/Counter started
 *  - false: Timer/Counter stopped
 */
bool LPTMR_DRV_IsRunning(const uint32_t instance);

/*!
 * @brief Enable/disable the LPTMR interrupt.
 *
 * @param[in] instance        - LPTMR instance number
 * @param[in] enableInterrupt - The new state of the LPTMR interrupt enable flag.
 */
void LPTMR_DRV_SetInterrupt(const uint32_t instance,
                            const bool enableInterrupt);

/*!
 * @brief Get the current counter value in counter tick units.
 *
 * @param[in] instance - LPTMR instance number
 * @return The current counter value
 */
uint16_t LPTMR_DRV_GetCounterValueByCount(const uint32_t instance);

/*!
 * @brief Enable the LPTMR / Start the counter
 *
 * @param[in] instance - LPTMR instance number
 */
void LPTMR_DRV_StartCounter(const uint32_t instance);

/*!
 * @brief Disable the LPTMR / Stop the counter
 *
 * @param[in] instance - LPTMR instance number
 */
void LPTMR_DRV_StopCounter(const uint32_t instance);

/*!
 * @brief Set the Input Pin configuration for Pulse Counter mode
 *
 * @param[in] instance    - LPTMR instance number
 * @param[in] pinSelect   - LPTMR pin selection
 * @param[in] pinPolarity - Polarity on which to increment counter (rising/falling edge)
 */
void LPTMR_DRV_SetPinConfiguration(const uint32_t instance,
                                   const lptmr_pinselect_t pinSelect,
                                   const lptmr_pinpolarity_t pinPolarity);

#if defined(__cplusplus)
}
#endif

/*! @}*/

/*! @}*/

#endif /* LPTMR_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
