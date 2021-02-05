/*
 * Copyright 2017 NXP
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

#ifndef CALLBACKS_H
#define CALLBACKS_H
#include <stdint.h>
/**
 * @page misra_violations MISRA-C:2012 violations
 *
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.3, A project should not contain
 * unused type declarations.
 * The header defines callback types for all PAL modules.
 */

 #include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

 /*!
 * @brief Define the enum of the events which can trigger I2C slave callback
 *
 * This enum should include the events for all platforms
 */
typedef enum
{
    I2C_SLAVE_EVENT_RX_FULL = 0x00U,
    I2C_SLAVE_EVENT_TX_EMPTY = 0x01U,
    I2C_SLAVE_EVENT_TX_REQ = 0x02U,
    I2C_SLAVE_EVENT_RX_REQ = 0x03U,
    I2C_SLAVE_EVENT_STOP = 0X04U,
}i2c_slave_event_t;

 /*!
 * @brief Define the enum of the events which can trigger I2C master callback
 *
 * This enum should include the events for all platforms
 *
 */
typedef enum
{
    I2C_MASTER_EVENT_END_TRANSFER                      = 0x4U,
}i2c_master_event_t;


/* Callback for all peripherals which supports I2C features for slave mode */
typedef void (*i2c_slave_callback_t)(i2c_slave_event_t event, void *userData);

/* Callback for all peripherals which supports I2C features for master mode */
typedef void (*i2c_master_callback_t)(i2c_master_event_t event, void *userData);

/* Define the enum of the events which can trigger SPI callback
 * This enum should include the events for all platforms
 */
typedef enum
{
    SPI_EVENT_END_TRANSFER = 0
} spi_event_t;

/* Callback for all peripherals which supports SPI features */
typedef void (*spi_callback_t)(void *driverState, spi_event_t event, void *userData);

/*!
 * @brief Define the enum of the events which can trigger UART callback
 *
 * This enum should include the events for all platforms
 *
 * Implements : uart_event_t_Class
 */
typedef enum
{
    UART_EVENT_RX_FULL      = 0x00U,    /*!< Rx buffer is full */
    UART_EVENT_TX_EMPTY     = 0x01U,    /*!< Tx buffer is empty */
    UART_EVENT_END_TRANSFER = 0x02U,    /*!< The current transfer is ending */
    UART_EVENT_ERROR        = 0x03U,    /*!< An error occured during transfer */
} uart_event_t;

/*!
 * @brief Callback for all peripherals which support UART features
 *
 * Implements : uart_callback_t_Class
 */
typedef void (*uart_callback_t)(void *driverState, uart_event_t event, void *userData);


/* Callback for all peripherals which support TIMING features */
typedef void (*timer_callback_t)(void *userData);


/*! @brief Defines a structure used to pass information to the ADC PAL callback
 *
 * Implements : adc_callback_info_t_Class
 */
typedef struct
{
    uint32_t groupIndex;         /*!< Index of the group executing the callback. */
    uint16_t resultBufferTail;   /*!< Offset of the most recent conversion result in the result buffer. */
} adc_callback_info_t;

/*! @brief Defines the callback used to be called by ADC PAL after the last conversion result in a group
 * has been copied to the result buffer.
 */
typedef void (* const adc_callback_t)(const adc_callback_info_t * const callbackInfo, void * userData);

/* I2S */
/* Define the enum of the events which can trigger i2s callback */
/* Events for all peripherals which support i2s
 *
 * Implements : i2s_event_t_Class
 */
typedef enum
{
    I2S_EVENT_RX_FULL      = 0x00U,    /*!< Rx buffer is full */
    I2S_EVENT_TX_EMPTY     = 0x01U,    /*!< Tx buffer is empty */
    I2S_EVENT_END_TRANSFER = 0x02U,    /*!< The current transfer is ending. Only FLEXIO instance uses this event. The difference between this and event TX_EMPTY is:
                                            TX_EMPTY is generated when all data has been pushed to hardware fifo, users should not call DeInit here or some last data will be lost;
                                            END_TRANSFER is generated when all data has been pushed to line, the transmission will be stopped before users can start transmit again, user can call DeInit here.
                                            For receiving case, this event is the same as RX_FULL.
                                            */
    I2S_EVENT_ERROR        = 0x03U,    /*!< An error occurred during transfer */
} i2s_event_t;

/* Callback for all peripherals which support i2s
 *
 * Implements : i2s_callback_t_Class
 */
typedef void (*i2s_callback_t)(i2s_event_t event, void *userData);

/*! @brief Define the enum of the events which can trigger CAN callback
 *  This enum should include the events for all platforms
 *  Implements : can_event_t_Class
 */
typedef enum {
    CAN_EVENT_RX_COMPLETE,     /*!< A frame was received in the configured Rx buffer. */
    CAN_EVENT_TX_COMPLETE,     /*!< A frame was sent from the configured Tx buffer. */
} can_event_t;

/*! @brief Callback for all peripherals which support CAN features
 * Implements : can_callback_t_Class
 */
typedef void (*can_callback_t)(uint32_t instance,
                               can_event_t eventType,
                               uint32_t objIdx,
                               void *driverState);

/*!
 * @brief Callback for security modules
 * Implements : security_callback_t_Class
 */
typedef void (*security_callback_t)(uint32_t completedCmd, void *callbackParam);

/* Define the enum of the events which can trigger the output compare callback */
typedef enum
{
    OC_EVENT_GENERATION_OUTPUT_COMPLETE = 0x00U    /*!< Generation output signal is completed */
} oc_event_t;

/* Callback for all peripherals which support OC feature */
typedef void (*oc_callback_t)(oc_event_t event, void *userData);
/* Define the enum of the events which can trigger the input capture callback */
typedef enum
{
    IC_EVENT_MEASUREMENT_COMPLETE = 0x00U    /*!< Capture input signal is completed */
} ic_event_t;

/* Callback for all peripherals which support IC feature */
typedef void (*ic_callback_t)(ic_event_t event, void *userData);

#endif /* CALLBACKS_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
