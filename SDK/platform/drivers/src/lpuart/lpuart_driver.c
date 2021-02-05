/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
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
 * @file lpuart_driver.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, There shall be no occurrence of
 * undefined or critical unspecified behavior.
 * This is caused because the addresses of some uninitialized variables are
 * passed as parameters. Those variables are not initialized as they are used as
 * output parameters by the functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.1, identifier clash
 * The supported compilers use more than 31 significant characters for identifiers.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.2, identifier clash
 * The supported compilers use more than 31 significant characters for identifiers.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.4, identifier clash
 * The supported compilers use more than 31 significant characters for identifiers.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.5, identifier clash
 * The supported compilers use more than 31 significant characters for identifiers.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, could define variable at block scope
 * The variables are defined in the common source file to make transition to other
 * platforms easier.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.7, Composite expression with smaller
 * essential type than other operand.
 * The expression is safe as the baud rate calculation algorithm cannot overflow
 * the result.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, A conversion should not be
 * performed between a pointer to object and an integer type.
 * The cast is required as source and destination addresses for DMA transfers must
 * be written to registers as 32-bit unsigned integers.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed
 * between pointer to void and an arithmetic type.
 * The cast is required as source and destination addresses for DMA transfers must
 * be written to registers as 32-bit unsigned integers.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.4, More than one 'break' terminates loop
 * This operation is necessary because more than one error flag can stop the reception.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, Return statement before end of function.
 * The return statement before end of function is used for simpler code structure
 * and better readability.
 */

#include <stddef.h>
#include <stdbool.h>
#include "lpuart_hw_access.h"
#include "lpuart_irq.h"
#include "clock_manager.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Pointer to lpuart runtime state structure */
static lpuart_state_t * s_lpuartStatePtr[LPUART_INSTANCE_COUNT] = {NULL};

/* Table of base addresses for lpuart instances. */
static LPUART_Type * const s_lpuartBase[LPUART_INSTANCE_COUNT] = LPUART_BASE_PTRS;

/* Table to save LPUART enum numbers defined in CMSIS files. */
static const IRQn_Type s_lpuartRxTxIrqId[LPUART_INSTANCE_COUNT] = LPUART_RX_TX_IRQS;

/* Table to save LPUART clock names as defined in clock manager. */
static const clock_names_t s_lpuartClkNames[LPUART_INSTANCE_COUNT] = LPUART_CLOCK_NAMES;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static status_t LPUART_DRV_StartSendDataUsingInt(uint32_t instance,
                                                 const uint8_t * txBuff,
                                                 uint32_t txSize);
static void LPUART_DRV_CompleteSendDataUsingInt(uint32_t instance);
static status_t LPUART_DRV_StartReceiveDataUsingInt(uint32_t instance,
                                                    uint8_t * rxBuff,
                                                    uint32_t rxSize);
static void LPUART_DRV_CompleteReceiveDataUsingInt(uint32_t instance);
#if FEATURE_LPUART_HAS_DMA_ENABLE
static void LPUART_DRV_StopTxDma(uint32_t instance);
static void LPUART_DRV_StopRxDma(uint32_t instance);
static status_t LPUART_DRV_StartSendDataUsingDma(uint32_t instance,
                                                 const uint8_t * txBuff,
                                                 uint32_t txSize);
static void LPUART_DRV_TxDmaCallback(void * parameter, edma_chn_status_t status);
static status_t LPUART_DRV_StartReceiveDataUsingDma(uint32_t instance,
                                                    uint8_t * rxBuff,
                                                    uint32_t rxSize);
static void LPUART_DRV_RxDmaCallback(void * parameter, edma_chn_status_t status);
#endif
static void LPUART_DRV_PutData(uint32_t instance);
static void LPUART_DRV_GetData(uint32_t instance);
static void LPUART_DRV_RxIrqHandler(uint32_t instance);
static void LPUART_DRV_TxEmptyIrqHandler(uint32_t instance);
static void LPUART_DRV_TxCompleteIrqHandler(uint32_t instance);
static void LPUART_DRV_ErrIrqHandler(uint32_t instance);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_GetDefaultConfig
 * Description   : Initializes the LPUART configuration structure with
 *                 default values.
 *
 * Implements    : LPUART_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
void LPUART_DRV_GetDefaultConfig(lpuart_user_config_t * lpuartUserConfig)
{
    DEV_ASSERT(lpuartUserConfig != NULL);

    lpuartUserConfig->transferType = LPUART_USING_INTERRUPTS;
    lpuartUserConfig->baudRate = 9600U;
    lpuartUserConfig->parityMode = LPUART_PARITY_DISABLED;
    lpuartUserConfig->stopBitCount = LPUART_ONE_STOP_BIT;
    lpuartUserConfig->bitCountPerChar = LPUART_8_BITS_PER_CHAR;
    lpuartUserConfig->rxDMAChannel = 0U;
    lpuartUserConfig->txDMAChannel = 0U;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_Init
 * Description   : This function initializes a LPUART instance for operation.
 * This function will initialize the run-time state structure to keep track of
 * the on-going transfers, ungate the clock to the LPUART module, initialize the
 * module to user defined settings and default settings, configure the IRQ state
 * structure and enable the module-level interrupt to the core, and enable the
 * LPUART module transmitter and receiver.
 * The following is an example of how to set up the lpuart_state_t and the
 * lpuart_user_config_t parameters and how to call the LPUART_DRV_Init function
 * by passing in these parameters:
 *    lpuart_user_config_t lpuartConfig;
 *    lpuartConfig.baudRate = 9600;
 *    lpuartConfig.bitCountPerChar = LPUART_8_BITS_PER_CHAR;
 *    lpuartConfig.parityMode = LPUART_PARITY_DISABLED;
 *    lpuartConfig.stopBitCount = LPUART_ONE_STOP_BIT;
 *    lpuartConfig.transferType = LPUART_USING_INTERRUPTS;
 *    lpuart_state_t lpuartState;
 *    LPUART_DRV_Init(instance, &lpuartState, &lpuartConfig);
 *
 * Implements    : LPUART_DRV_Init_Activity
 *END**************************************************************************/
status_t LPUART_DRV_Init(uint32_t instance, lpuart_state_t * lpuartStatePtr,
                         const lpuart_user_config_t * lpuartUserConfig)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(lpuartStatePtr != NULL);
    DEV_ASSERT(lpuartUserConfig != NULL);

    status_t osStatusRxSem;
    status_t osStatusTxSem;
    LPUART_Type * base = s_lpuartBase[instance];
    uint32_t idx;
    uint32_t lpuartSourceClock;
    clock_names_t instanceClkName = s_lpuartClkNames[instance];

    /* Get the LPUART clock as configured in the clock manager */
    (void)CLOCK_SYS_GetFreq(instanceClkName, &lpuartSourceClock);

    /* Check if current instance is clock gated off. */
    DEV_ASSERT(lpuartSourceClock > 0U);

    /* Check if current instance is already initialized. */
    DEV_ASSERT(s_lpuartStatePtr[instance] == NULL);

#if FEATURE_LPUART_HAS_DMA_ENABLE
    /* In DMA mode, only 8-bits chars are supported */
    DEV_ASSERT((lpuartUserConfig->transferType != LPUART_USING_DMA) ||
               (lpuartUserConfig->bitCountPerChar == LPUART_8_BITS_PER_CHAR));
#endif

    /* For 10 bits per char, parity bit cannot be enabled */
    DEV_ASSERT((lpuartUserConfig->bitCountPerChar != LPUART_10_BITS_PER_CHAR) ||
               (lpuartUserConfig->parityMode == LPUART_PARITY_DISABLED));

    /* Clear the state struct for this instance. */
    uint8_t *clearStructPtr = (uint8_t *)lpuartStatePtr;
    for (idx = 0; idx < sizeof(lpuart_state_t); idx++)
    {
        clearStructPtr[idx] = 0;
    }

    /* Save runtime structure pointer.*/
    s_lpuartStatePtr[instance] = lpuartStatePtr;

    /* Save the transfer information for runtime retrieval */
    lpuartStatePtr->transferType = lpuartUserConfig->transferType;
    lpuartStatePtr->bitCountPerChar = lpuartUserConfig->bitCountPerChar;
#if FEATURE_LPUART_HAS_DMA_ENABLE
    lpuartStatePtr->rxDMAChannel = lpuartUserConfig->rxDMAChannel;
    lpuartStatePtr->txDMAChannel = lpuartUserConfig->txDMAChannel;
#endif

    /* initialize the LPUART instance */
    LPUART_Init(base);

    /* initialize the parameters of the LPUART config structure with desired data */
    (void)LPUART_DRV_SetBaudRate(instance, lpuartUserConfig->baudRate);

    if (lpuartUserConfig->parityMode != LPUART_PARITY_DISABLED)
    {
        LPUART_SetBitCountPerChar(base, lpuartUserConfig->bitCountPerChar, true);
    }
    else
    {
        LPUART_SetBitCountPerChar(base, lpuartUserConfig->bitCountPerChar, false);
    }
    LPUART_SetParityMode(base, lpuartUserConfig->parityMode);
    LPUART_SetStopBitCount(base, lpuartUserConfig->stopBitCount);

    /* initialize last driver operation status */
    lpuartStatePtr->transmitStatus = STATUS_SUCCESS;
    lpuartStatePtr->receiveStatus = STATUS_SUCCESS;

    /* Create the synchronization objects */
    osStatusRxSem = OSIF_SemaCreate(&lpuartStatePtr->rxComplete, 0);
    osStatusTxSem = OSIF_SemaCreate(&lpuartStatePtr->txComplete, 0);
    if ((osStatusRxSem == STATUS_ERROR) || (osStatusTxSem == STATUS_ERROR))
    {
        return STATUS_ERROR;
    }

    /* Install LPUART irq handler */
    INT_SYS_InstallHandler(s_lpuartRxTxIrqId[instance], g_lpuartIsr[instance], (isr_t*) 0);

    /* Enable LPUART interrupt. */
    INT_SYS_EnableIRQ(s_lpuartRxTxIrqId[instance]);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_Deinit
 * Description   : This function shuts down the UART by disabling interrupts and
 *                 transmitter/receiver.
 *
 * Implements    : LPUART_DRV_Deinit_Activity
 *END**************************************************************************/
status_t LPUART_DRV_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    clock_names_t instanceClkName = s_lpuartClkNames[instance];
    uint32_t lpuartSourceClock;
    const LPUART_Type * base = s_lpuartBase[instance];
    const lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    (void)CLOCK_SYS_GetFreq(instanceClkName, &lpuartSourceClock);

    /* Check if current instance is already de-initialized or is gated.*/
    DEV_ASSERT(s_lpuartStatePtr[instance] != NULL);
    DEV_ASSERT(lpuartSourceClock > 0U);

    /* Wait until the data is completely shifted out of shift register */
    while (!LPUART_GetStatusFlag(base, LPUART_TX_COMPLETE)) {}

    /* Destroy the synchronization objects */
    (void)OSIF_SemaDestroy(&lpuartState->rxComplete);
    (void)OSIF_SemaDestroy(&lpuartState->txComplete);

    /* Disable LPUART interrupt. */
    INT_SYS_DisableIRQ(s_lpuartRxTxIrqId[instance]);

    /* Restore default handler. */
    INT_SYS_InstallHandler(s_lpuartRxTxIrqId[instance], DefaultISR, (isr_t*) 0);

    /* Clear our saved pointer to the state structure */
    s_lpuartStatePtr[instance] = NULL;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_InstallRxCallback
 * Description   : Install receive data callback function.
 *
 * Implements    : LPUART_DRV_InstallRxCallback_Activity
 *END**************************************************************************/
uart_callback_t LPUART_DRV_InstallRxCallback(uint32_t instance,
                                             uart_callback_t function,
                                             void * callbackParam)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    uart_callback_t currentCallback = lpuartState->rxCallback;
    lpuartState->rxCallback = function;
    lpuartState->rxCallbackParam = callbackParam;

    return currentCallback;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_InstallTxCallback
 * Description   : Install transmit data callback function, pass in NULL pointer
 * as callback will uninstall.
 *
 * Implements    : LPUART_DRV_InstallTxCallback_Activity
 *END**************************************************************************/
uart_callback_t LPUART_DRV_InstallTxCallback(uint32_t instance,
                                             uart_callback_t function,
                                             void * callbackParam)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    uart_callback_t currentCallback = lpuartState->txCallback;
    lpuartState->txCallback = function;
    lpuartState->txCallbackParam = callbackParam;

    return currentCallback;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_SendDataBlocking
 * Description   : This function sends data out through the LPUART module using
 * blocking method. The function does not return until the transmit is complete.
 *
 * Implements    : LPUART_DRV_SendDataBlocking_Activity
 *END**************************************************************************/
status_t LPUART_DRV_SendDataBlocking(uint32_t instance,
                                     const uint8_t * txBuff,
                                     uint32_t txSize,
                                     uint32_t timeout)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(txBuff != NULL);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    status_t retVal = STATUS_SUCCESS;
    status_t syncStatus;

    /* Indicates this is a blocking transaction. */
    lpuartState->isTxBlocking = true;

    DEV_ASSERT((lpuartState->transferType == LPUART_USING_INTERRUPTS) ||
               (lpuartState->transferType == LPUART_USING_DMA));

    if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
    {
        /* Start the transmission process using interrupts */
        retVal = LPUART_DRV_StartSendDataUsingInt(instance, txBuff, txSize);
    }
#if FEATURE_LPUART_HAS_DMA_ENABLE
    else
    {
        /* Start the transmission process using DMA */
        retVal = LPUART_DRV_StartSendDataUsingDma(instance, txBuff, txSize);
    }
#endif

    if (retVal == STATUS_SUCCESS)
    {
        /* Wait until the transmit is complete. */
        syncStatus = OSIF_SemaWait(&lpuartState->txComplete, timeout);

        /* Finish the transmission if timeout expired */
        if (syncStatus == STATUS_TIMEOUT)
        {
            lpuartState->isTxBlocking = false;
            lpuartState->transmitStatus = STATUS_TIMEOUT;

            if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
            {
                LPUART_DRV_CompleteSendDataUsingInt(instance);
            }
#if FEATURE_LPUART_HAS_DMA_ENABLE
            else
            {
                LPUART_DRV_StopTxDma(instance);
            }
#endif
        }
    }

    return lpuartState->transmitStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_SendDataPolling
 * Description   : Send out multiple bytes of data using polling method.
 *
 * Implements    : LPUART_DRV_SendDataPolling_Activity
 *END**************************************************************************/
status_t LPUART_DRV_SendDataPolling(uint32_t instance,
                                const uint8_t *txBuff,
                                uint32_t txSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(txBuff != NULL);

    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check the validity of the parameters */
    DEV_ASSERT(txSize > 0U);
    DEV_ASSERT((lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR) ||
               ((txSize & 1U) == 0U));

    /* Check driver is not busy transmitting data from a previous asynchronous call */
    if (lpuartState->isTxBusy)
    {
        return STATUS_BUSY;
    }

    /* Enable the LPUART transmitter */
    LPUART_SetTransmitterCmd(base, true);

    while (txSize > 0U)
    {
        while (!LPUART_GetStatusFlag(base, LPUART_TX_DATA_REG_EMPTY))
        {}

        lpuartState->txBuff = txBuff;
        LPUART_DRV_PutData(instance);

        if (lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR)
        {
            ++txBuff;
            --txSize;
        }
        else
        {
            ++txBuff;
            ++txBuff;
            txSize -= 2U;
        }
    }

    /* Disable the LPUART transmitter */
    LPUART_SetTransmitterCmd(base, false);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_SendData
 * Description   : This function sends data out through the LPUART module using
 * non-blocking method. The function will return immediately after calling this
 * function.
 *
 * Implements    : LPUART_DRV_SendData_Activity
 *END**************************************************************************/
status_t LPUART_DRV_SendData(uint32_t instance,
                             const uint8_t * txBuff,
                             uint32_t txSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(txBuff != NULL);

    status_t retVal = STATUS_SUCCESS;
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Indicates this is a non-blocking transaction. */
    lpuartState->isTxBlocking = false;

    DEV_ASSERT((lpuartState->transferType == LPUART_USING_INTERRUPTS) ||
               (lpuartState->transferType == LPUART_USING_DMA));

    if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
    {
        /* Start the transmission process using interrupts */
        retVal = LPUART_DRV_StartSendDataUsingInt(instance, txBuff, txSize);
    }
#if FEATURE_LPUART_HAS_DMA_ENABLE
    else
    {
        /* Start the transmission process using DMA */
        retVal = LPUART_DRV_StartSendDataUsingDma(instance, txBuff, txSize);
    }
#endif

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_GetTransmitStatus
 * Description   : This function returns whether the previous LPUART transmit has
 * finished. When performing non-blocking transmit, the user can call this
 * function to ascertain the state of the current transmission:
 * in progress (or busy) or complete (success). In addition, if the transmission
 * is still in progress, the user can obtain the number of words that have been
 * currently transferred.
 *
 * Implements    : LPUART_DRV_GetTransmitStatus_Activity
 *END**************************************************************************/
status_t LPUART_DRV_GetTransmitStatus(uint32_t instance, uint32_t * bytesRemaining)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    const lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    if (bytesRemaining != NULL)
    {
        if (lpuartState->isTxBusy)
        {
            /* Fill in the bytes not transferred yet. */
            if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
            {
                /* In interrupt-based communication, the remaining bytes are retrieved
                 * from the state structure
                 */
                *bytesRemaining = lpuartState->txSize;;
            }
#if FEATURE_LPUART_HAS_DMA_ENABLE
            else
            {
                /* In DMA-based communication, the remaining bytes are retrieved
                 * from the current DMA major loop count
                 */
                *bytesRemaining = EDMA_DRV_GetRemainingMajorIterationsCount(lpuartState->txDMAChannel);
            }
#endif
        }
        else
        {
            *bytesRemaining = 0;
        }
    }

    return lpuartState->transmitStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_AbortSendingData
 * Description   : This function terminates an non-blocking LPUART transmission
 * early. During a non-blocking LPUART transmission, the user has the option to
 * terminate the transmission early if the transmission is still in progress.
 *
 * Implements    : LPUART_DRV_AbortSendingData_Activity
 *END**************************************************************************/
status_t LPUART_DRV_AbortSendingData(uint32_t instance)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check if a transfer is running. */
    if (!lpuartState->isTxBusy)
    {
        return STATUS_SUCCESS;
    }

    /* Update the tx status */
    lpuartState->transmitStatus = STATUS_UART_ABORTED;

    /* Stop the running transfer. */
    if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
    {
        LPUART_DRV_CompleteSendDataUsingInt(instance);
    }
#if FEATURE_LPUART_HAS_DMA_ENABLE
    else
    {
        LPUART_DRV_StopTxDma(instance);
    }
#endif

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_ReceiveDataBlocking
 * Description   : This function receives data from LPUART module using blocking
 * method, the function does not return until the receive is complete.
 *
 * Implements    : LPUART_DRV_ReceiveDataBlocking_Activity
 *END**************************************************************************/
status_t LPUART_DRV_ReceiveDataBlocking(uint32_t instance,
                                        uint8_t * rxBuff,
                                        uint32_t rxSize,
                                        uint32_t timeout)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(rxBuff != NULL);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    status_t retVal = STATUS_SUCCESS;
    status_t syncStatus;

    /* Indicates this is a blocking transaction. */
    lpuartState->isRxBlocking = true;

    DEV_ASSERT((lpuartState->transferType == LPUART_USING_INTERRUPTS) ||
               (lpuartState->transferType == LPUART_USING_DMA));

    if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
    {
         /* Start the reception process using interrupts */
         retVal = LPUART_DRV_StartReceiveDataUsingInt(instance, rxBuff, rxSize);
    }
#if FEATURE_LPUART_HAS_DMA_ENABLE
    else
    {
        /* Start the reception process using DMA */
        retVal = LPUART_DRV_StartReceiveDataUsingDma(instance, rxBuff, rxSize);
    }
#endif

    if (retVal == STATUS_SUCCESS)
    {
        /* Wait until the receive is complete. */
        syncStatus = OSIF_SemaWait(&lpuartState->rxComplete, timeout);

        /* Finish the reception if timeout expired */
        if (syncStatus == STATUS_TIMEOUT)
        {
            lpuartState->isRxBlocking = false;
            lpuartState->receiveStatus = STATUS_TIMEOUT;

            if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
            {
                LPUART_DRV_CompleteReceiveDataUsingInt(instance);
            }
#if FEATURE_LPUART_HAS_DMA_ENABLE
            else
            {
                LPUART_DRV_StopRxDma(instance);
            }
#endif
        }
    }

    return lpuartState->receiveStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_ReceiveDataPolling
 * Description   : Receive multiple bytes of data using polling method.
 *
 * Implements    : LPUART_DRV_ReceiveDataPolling_Activity
 *END**************************************************************************/
status_t LPUART_DRV_ReceiveDataPolling(uint32_t instance,
                                       uint8_t *rxBuff,
                                       uint32_t rxSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(rxBuff != NULL);

    uint8_t tmpByte;
    status_t retVal = STATUS_SUCCESS;
    status_t tmpState = STATUS_SUCCESS;
    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check the validity of the parameters */
    DEV_ASSERT(rxSize > 0U);
    DEV_ASSERT((lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR) ||
               ((rxSize & 1U) == 0U));

    /* Check driver is not busy receiving data from a previous asynchronous call */
    if (lpuartState->isRxBusy)
    {
        return STATUS_BUSY;
    }

    /* Enable the LPUART receiver */
    LPUART_SetReceiverCmd((LPUART_Type *)base, true);

    while (rxSize > 0U)
    {
        while (!LPUART_GetStatusFlag(base, LPUART_RX_DATA_REG_FULL))
        {}

        lpuartState->rxBuff = rxBuff;
        LPUART_DRV_GetData(instance);

        if (lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR)
        {
            ++rxBuff;
            --rxSize;
        }
        else
        {
            ++rxBuff;
            ++rxBuff;
            rxSize -= 2U;
        }

        /* Check for errors on received data */
        if (LPUART_GetStatusFlag(base, LPUART_FRAME_ERR))
        {
            tmpState = STATUS_UART_FRAMING_ERROR;
            /* Disable the LPUART receiver */
            LPUART_SetReceiverCmd((LPUART_Type *)base, false);
            /* Clear the flag */
            (void)LPUART_ClearStatusFlag(base, LPUART_FRAME_ERR);
            break;
        }
        else if (LPUART_GetStatusFlag(base, LPUART_NOISE_DETECT))
        {
            tmpState = STATUS_UART_NOISE_ERROR;
            /* Disable the LPUART receiver */
            LPUART_SetReceiverCmd((LPUART_Type *)base, false);
            /* Clear the flag */
            (void)LPUART_ClearStatusFlag(base, LPUART_NOISE_DETECT);
            break;
        }
        else if (LPUART_GetStatusFlag(base, LPUART_PARITY_ERR))
        {
            tmpState = STATUS_UART_PARITY_ERROR;
            /* Disable the LPUART receiver */
            LPUART_SetReceiverCmd((LPUART_Type *)base, false);
            /* Clear the flag */
            (void)LPUART_ClearStatusFlag(base, LPUART_PARITY_ERR);
            break;
        }
        else if (LPUART_GetStatusFlag(base, LPUART_RX_OVERRUN))
        {
            tmpState = STATUS_UART_RX_OVERRUN;
            /* Disable the LPUART receiver */
            LPUART_SetReceiverCmd((LPUART_Type *)base, false);
            /* Clear the flag */
            (void)LPUART_ClearStatusFlag(base, LPUART_RX_OVERRUN);
            break;
        }
        else
        {
            /* No error condition - avoid MISRA violation */
        }
    }

    /* Update received status */
    if ((rxSize == 0U) && (tmpState == STATUS_UART_RX_OVERRUN))
    {
        retVal = STATUS_SUCCESS;
    }
    else
    {
        retVal = tmpState;
    }

    if (retVal == STATUS_SUCCESS)
    {
        /* Disable the LPUART receiver */
        LPUART_SetReceiverCmd((LPUART_Type *)base, false);
    }

    /* Read dummy to clear RDRF flag */
    LPUART_Getchar(base, &tmpByte);

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_ReceiveData
 * Description   : This function receives data from LPUART module using
 * non-blocking method.  This function returns immediately after initiating the
 * receive function. The application has to get the receive status to see when
 * the receive is complete. In other words, after calling non-blocking get
 * function, the application must get the receive status to check if receive
 * is completed or not.
 *
 * Implements    : LPUART_DRV_ReceiveData_Activity
 *END**************************************************************************/
status_t LPUART_DRV_ReceiveData(uint32_t instance,
                                uint8_t * rxBuff,
                                uint32_t rxSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(rxBuff != NULL);

    status_t retVal = STATUS_SUCCESS;
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Indicates this is a non-blocking transaction. */
    lpuartState->isRxBlocking = false;

    DEV_ASSERT((lpuartState->transferType == LPUART_USING_INTERRUPTS) ||
               (lpuartState->transferType == LPUART_USING_DMA));

    if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
    {
        /* Start the reception process using interrupts */
        retVal = LPUART_DRV_StartReceiveDataUsingInt(instance, rxBuff, rxSize);
    }
#if FEATURE_LPUART_HAS_DMA_ENABLE
    else
    {
        /* Start the reception process using DMA */
        retVal = LPUART_DRV_StartReceiveDataUsingDma(instance, rxBuff, rxSize);
    }
#endif

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_GetReceiveStatus
 * Description   : This function returns whether the previous LPUART receive is
 * complete. When performing a non-blocking receive, the user can call this
 * function to ascertain the state of the current receive progress: in progress
 * or complete. In addition, if the receive is still in progress, the user can
 * obtain the number of words that have been currently received.
 *
 * Implements    : LPUART_DRV_GetReceiveStatus_Activity
 *END**************************************************************************/
status_t LPUART_DRV_GetReceiveStatus(uint32_t instance,
                                     uint32_t * bytesRemaining)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    const lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    if (bytesRemaining != NULL)
    {
        if (lpuartState->isRxBusy)
        {
            /* Fill in the bytes transferred. */
            if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
            {
                /* In interrupt-based communication, the remaining bytes are retrieved
                 * from the state structure
                 */
                *bytesRemaining = lpuartState->rxSize;
            }
#if FEATURE_LPUART_HAS_DMA_ENABLE
            else
            {
                /* In DMA-based communication, the remaining bytes are retrieved
                 * from the current DMA major loop count
                 */
                *bytesRemaining = EDMA_DRV_GetRemainingMajorIterationsCount(lpuartState->rxDMAChannel);
            }
#endif
        }
        else
        {
            *bytesRemaining = 0;
        }
    }

    return lpuartState->receiveStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_AbortReceivingData
 * Description   : Terminates a non-blocking receive early.
 *
 * Implements    : LPUART_DRV_AbortReceivingData_Activity
 *END**************************************************************************/
status_t LPUART_DRV_AbortReceivingData(uint32_t instance)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check if a transfer is running. */
    if (!lpuartState->isRxBusy)
    {
        return STATUS_SUCCESS;
    }

    /* Update the rx status */
    lpuartState->receiveStatus = STATUS_UART_ABORTED;

    /* Stop the running transfer. */
    if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
    {
        LPUART_DRV_CompleteReceiveDataUsingInt(instance);
    }
#if FEATURE_LPUART_HAS_DMA_ENABLE
    else
    {
        LPUART_DRV_StopRxDma(instance);
    }
#endif

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_SetBaudRate
 * Description   : Configures the LPUART baud rate.
 * In some LPUART instances the user must disable the transmitter/receiver
 * before calling this function.
 * Generally, this may be applied to all LPUARTs to ensure safe operation.
 *
 * Implements    : LPUART_DRV_SetBaudRate_Activity
 *END**************************************************************************/
status_t LPUART_DRV_SetBaudRate(uint32_t instance, uint32_t desiredBaudRate)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    uint16_t sbr, sbrTemp, i;
    uint32_t osr, tempDiff, calculatedBaud, baudDiff, maxOsr;
    uint32_t lpuartSourceClock;
    clock_names_t instanceClkName = s_lpuartClkNames[instance];
    LPUART_Type * base = s_lpuartBase[instance];
    const lpuart_state_t * lpuartState;
    lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    if (lpuartState != NULL)
    {
        /* Check if there is an ongoing transfer */
        if (lpuartState->isTxBusy == true)
        {
            return STATUS_BUSY;
        }
        if (lpuartState->isRxBusy == true)
        {
            return STATUS_BUSY;
        }
    }

    /* Get the LPUART clock as configured in the clock manager */
    (void)CLOCK_SYS_GetFreq(instanceClkName, &lpuartSourceClock);

    /* Check if current instance is clock gated off. */
    DEV_ASSERT(lpuartSourceClock > 0U);
    /* Check if the desired baud rate can be configured with the current protocol clock. */
    DEV_ASSERT(lpuartSourceClock >= (desiredBaudRate * 4U));

    /* This lpuart instantiation uses a slightly different baud rate calculation
     * The idea is to use the best OSR (over-sampling rate) possible
     * Note, osr is typically hard-set to 16 in other lpuart instantiations
     * First calculate the baud rate using the minimum OSR possible (4) */
    osr = 4;
    sbr = (uint16_t)(lpuartSourceClock / (desiredBaudRate * osr));
    calculatedBaud = (lpuartSourceClock / (osr * sbr));
    if (calculatedBaud > desiredBaudRate)
    {
        baudDiff = calculatedBaud - desiredBaudRate;
    }
    else
    {
        baudDiff = desiredBaudRate - calculatedBaud;
    }
    /* find maximum osr */
    maxOsr = lpuartSourceClock / desiredBaudRate;
    if (maxOsr > 32U)
    {
        maxOsr = 32U;
    }
    /* loop to find the best osr value possible, one that generates minimum baudDiff
     * iterate through the rest of the supported values of osr */
    if (maxOsr >= 5U)
    {
        for (i = 5U; i <= maxOsr; i++)
        {
            /* calculate the temporary sbr value   */
            sbrTemp = (uint16_t)(lpuartSourceClock / (desiredBaudRate * i));
            /* calculate the baud rate based on the temporary osr and sbr values */
            calculatedBaud = (lpuartSourceClock / (i * sbrTemp));

            if (calculatedBaud > desiredBaudRate)
            {
                tempDiff = calculatedBaud - desiredBaudRate;
            }
            else
            {
                tempDiff = desiredBaudRate - calculatedBaud;
            }

            if (tempDiff <= baudDiff)
            {
                baudDiff = tempDiff;
                osr = i;  /* update and store the best osr value calculated */
                sbr = sbrTemp;  /* update store the best sbr value calculated */
            }
        }
    }
    /* Check if osr is between 4x and 7x oversampling.
     * If so, then "BOTHEDGE" sampling must be turned on */
    if (osr < 8U)
    {
    	LPUART_EnableBothEdgeSamplingCmd(base);
    }

    /* program the osr value (bit value is one less than actual value) */
    LPUART_SetOversamplingRatio(base, (osr - 1U));

    /* write the sbr value to the BAUD registers */
    LPUART_SetBaudRateDivisor(base, sbr);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_GetBaudRate
 * Description   : Returns the LPUART configured baud rate.
 *
 * Implements    : LPUART_DRV_GetBaudRate_Activity
 *END**************************************************************************/
void LPUART_DRV_GetBaudRate(uint32_t instance, uint32_t * configuredBaudRate)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(configuredBaudRate != NULL);

    uint8_t osr;
    uint16_t sbr;
    uint32_t lpuartSourceClock;
    clock_names_t instanceClkName = s_lpuartClkNames[instance];
    const LPUART_Type * base = s_lpuartBase[instance];

    /* Get the LPUART clock as configured in the clock manager */
    (void)CLOCK_SYS_GetFreq(instanceClkName, &lpuartSourceClock);

    osr = LPUART_GetOversamplingRatio(base);
    sbr = LPUART_GetBaudRateDivisor(base);

    *configuredBaudRate = (lpuartSourceClock / ((osr + 1UL) * sbr));
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_SetTxBuffer
 * Description   : Sets the driver internal reference to the tx buffer.
 *                 Can be called from the tx callback to provide a different
 *                 buffer for continuous transmission.
 *
 * Implements    : LPUART_DRV_SetTxBuffer_Activity
 *END**************************************************************************/
status_t LPUART_DRV_SetTxBuffer(uint32_t instance,
                                const uint8_t * txBuff,
                                uint32_t txSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(txBuff != NULL);
    DEV_ASSERT(txSize > 0U);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    lpuartState->txBuff = txBuff;
    lpuartState->txSize = txSize;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_SetRxBuffer
 * Description   : Sets the driver internal reference to the rx buffer.
 *                 Can be called from the rx callback to provide a different
 *                 buffer for continuous reception.
 *
 * Implements    : LPUART_DRV_SetRxBuffer_Activity
 *END**************************************************************************/
status_t LPUART_DRV_SetRxBuffer(uint32_t instance,
                                uint8_t * rxBuff,
                                uint32_t rxSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(rxBuff != NULL);
    DEV_ASSERT(rxSize > 0U);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    lpuartState->rxBuff = rxBuff;
    lpuartState->rxSize = rxSize;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_IRQHandler
 * Description   : Interrupt handler for LPUART.
 * This handler uses the buffers stored in the lpuart_state_t structs to transfer
 * data. This is not a public API as it is called by IRQ whenever an interrupt
 * occurs.
 *
 *END**************************************************************************/
void LPUART_DRV_IRQHandler(uint32_t instance)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    const LPUART_Type * base = s_lpuartBase[instance];

    LPUART_DRV_ErrIrqHandler(instance);

    /* Handle receive data full interrupt */
    if (LPUART_GetIntMode(base, LPUART_INT_RX_DATA_REG_FULL))
    {
        if (LPUART_GetStatusFlag(base, LPUART_RX_DATA_REG_FULL))
        {
            LPUART_DRV_RxIrqHandler(instance);
        }
    }

    /* Handle transmitter data register empty interrupt */
    if (LPUART_GetIntMode(base, LPUART_INT_TX_DATA_REG_EMPTY))
    {
        if (LPUART_GetStatusFlag(base, LPUART_TX_DATA_REG_EMPTY))
        {
            LPUART_DRV_TxEmptyIrqHandler(instance);
        }
    }

    /* Handle transmission complete interrupt */
    if (LPUART_GetIntMode(base, LPUART_INT_TX_COMPLETE))
    {
        if (LPUART_GetStatusFlag(base, LPUART_TX_COMPLETE))
        {
            LPUART_DRV_TxCompleteIrqHandler(instance);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_RxIrqHandler
 * Description   : Rx Interrupt handler for LPUART.
 * This function treats the rx full interrupt.
 *
 *END**************************************************************************/
static void LPUART_DRV_RxIrqHandler(uint32_t instance)
{
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Get data and put in receive buffer  */
    LPUART_DRV_GetData(instance);

    /* Update the internal state */
    if (lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR)
    {
        ++lpuartState->rxBuff;
        --lpuartState->rxSize;
    }
    else
    {
        lpuartState->rxBuff = &lpuartState->rxBuff[2];
        lpuartState->rxSize -= 2U;
    }

    /* Check if this was the last byte in the current buffer */
    if (lpuartState->rxSize == 0U)
    {
        /* Invoke callback if there is one (callback may reset the rx buffer for continuous reception) */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_RX_FULL, lpuartState->rxCallbackParam);
        }
    }

    /* Finish reception if this was the last byte received */
    if (lpuartState->rxSize == 0U)
    {
        /* Complete transfer (disable rx logic) */
        LPUART_DRV_CompleteReceiveDataUsingInt(instance);

        /* Invoke callback if there is one */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_END_TRANSFER, lpuartState->rxCallbackParam);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_TxEmptyIrqHandler
 * Description   : Tx Empty Interrupt handler for LPUART.
 * This function treats the tx empty interrupt.
 *
 *END**************************************************************************/
static void LPUART_DRV_TxEmptyIrqHandler(uint32_t instance)
{
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];

    /* Check if there are any more bytes to send */
    if (lpuartState->txSize > 0U)
    {
        /* Transmit the data */
        LPUART_DRV_PutData(instance);

        /* Update the internal state */
        if (lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR)
        {
            ++lpuartState->txBuff;
            --lpuartState->txSize;
        }
        else
        {
            lpuartState->txBuff = &lpuartState->txBuff[2];
            lpuartState->txSize -= 2U;
        }

        /* Check if this was the last byte in the current buffer */
        if (lpuartState->txSize == 0U)
        {
            /* Invoke callback if there is one (callback may reset the tx buffer for continuous transmission)*/
            if (lpuartState->txCallback != NULL)
            {
                lpuartState->txCallback(lpuartState, UART_EVENT_TX_EMPTY, lpuartState->txCallbackParam);
            }

            /* If there's no new data, disable tx empty interrupt and enable transmission complete interrupt */
            if (lpuartState->txSize == 0U)
            {
                LPUART_SetIntMode(base, LPUART_INT_TX_DATA_REG_EMPTY, false);
                LPUART_SetIntMode(base, LPUART_INT_TX_COMPLETE, true);
            }
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_TxCompleteIrqHandler
 * Description   : Tx Complete Interrupt handler for LPUART.
 * This function treats the tx complete interrupt.
 *
 *END**************************************************************************/
static void LPUART_DRV_TxCompleteIrqHandler(uint32_t instance)
{
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    if (lpuartState->txSize == 0U)
    {
        if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
        {
            /* Complete the interrupt based transfer */
            LPUART_DRV_CompleteSendDataUsingInt(instance);
        }
#if FEATURE_LPUART_HAS_DMA_ENABLE
        else
        {
            /* Complete the DMA based transfer */
            LPUART_DRV_StopTxDma(instance);
        }
#endif
        /* Invoke callback if there is one */
        if (lpuartState->txCallback != NULL)
        {
            lpuartState->txCallback(lpuartState, UART_EVENT_END_TRANSFER, lpuartState->txCallbackParam);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_ErrIrqHandler
 * Description   : Error Interrupt handler for LPUART.
 * This function treats the error interrupts.
 *
 *END**************************************************************************/
static void LPUART_DRV_ErrIrqHandler(uint32_t instance)
{
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];

    /* Handle receive overrun interrupt */
    if (LPUART_GetStatusFlag(base, LPUART_RX_OVERRUN))
    {
        /* Update the internal status */
        lpuartState->receiveStatus = STATUS_UART_RX_OVERRUN;
        if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
        {
            /* Complete the transfer (disable rx logic) */
            LPUART_DRV_CompleteReceiveDataUsingInt(instance);
        }
#if FEATURE_LPUART_HAS_DMA_ENABLE
        else
        {
            /* Complete the transfer (stop DMA channel) */
            LPUART_DRV_StopRxDma(instance);
        }
#endif
        /* Invoke callback if there is one */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_ERROR, lpuartState->rxCallbackParam);
        }

        /* Clear the flags */
        LPUART_DRV_ClearErrorFlags(base);
    }
    /* Handle framing error interrupt */
    else if (LPUART_GetStatusFlag(base, LPUART_FRAME_ERR))
    {
        /* Update the internal status */
        lpuartState->receiveStatus = STATUS_UART_FRAMING_ERROR;
        if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
        {
            /* Complete the transfer (disable rx logic) */
            LPUART_DRV_CompleteReceiveDataUsingInt(instance);
        }
#if FEATURE_LPUART_HAS_DMA_ENABLE
        else
        {
            /* Complete the transfer (stop DMA channel) */
            LPUART_DRV_StopRxDma(instance);
        }
#endif
        /* Invoke callback if there is one */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_ERROR, lpuartState->rxCallbackParam);
        }

        /* Clear the flags */
        LPUART_DRV_ClearErrorFlags(base);
    }
    /* Handle parity error interrupt */
    else if (LPUART_GetStatusFlag(base, LPUART_PARITY_ERR))
    {
        /* Update the internal status */
        lpuartState->receiveStatus = STATUS_UART_PARITY_ERROR;
        if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
        {
            /* Complete the transfer (disable rx logic) */
            LPUART_DRV_CompleteReceiveDataUsingInt(instance);
        }
#if FEATURE_LPUART_HAS_DMA_ENABLE
        else
        {
            /* Complete the transfer (stop DMA channel) */
            LPUART_DRV_StopRxDma(instance);
        }
#endif
        /* Invoke callback if there is one */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_ERROR, lpuartState->rxCallbackParam);
        }

        /* Clear the flags */
        LPUART_DRV_ClearErrorFlags(base);
    }
    /* Handle noise error interrupt */
    else if (LPUART_GetStatusFlag(base, LPUART_NOISE_DETECT))
    {
        /* Update the internal status */
        lpuartState->receiveStatus = STATUS_UART_NOISE_ERROR;
        if (lpuartState->transferType == LPUART_USING_INTERRUPTS)
        {
            /* Complete transfer (disable rx logic) */
            LPUART_DRV_CompleteReceiveDataUsingInt(instance);
        }
#if FEATURE_LPUART_HAS_DMA_ENABLE
        else
        {
            /* Complete the transfer (stop DMA channel) */
            LPUART_DRV_StopRxDma(instance);
        }
#endif
        /* Invoke callback if there is one */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_ERROR, lpuartState->rxCallbackParam);
        }

        /* Clear the flags */
        LPUART_DRV_ClearErrorFlags(base);
    }
    else
    {
        /* Clear all the error/interrupt flags */
        base->STAT = FEATURE_LPUART_STAT_REG_FLAGS_MASK;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_StartSendDataUsingInt
 * Description   : Initiate (start) a transmit by beginning the process of
 * sending data and enabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t LPUART_DRV_StartSendDataUsingInt(uint32_t instance,
                                                 const uint8_t * txBuff,
                                                 uint32_t txSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(txBuff != NULL);

    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check it's not busy transmitting data from a previous function call */
    if (lpuartState->isTxBusy)
    {
        return STATUS_BUSY;
    }

    /* Check the validity of the parameters */
    DEV_ASSERT(txSize > 0U);
    DEV_ASSERT((lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR) ||
               ((txSize & 1U) == 0U));

    /* initialize the module driver state structure */
    lpuartState->txBuff = txBuff;
    lpuartState->txSize = txSize;
    lpuartState->isTxBusy = true;
    lpuartState->transmitStatus = STATUS_BUSY;

    /* Enable the LPUART transmitter */
    LPUART_SetTransmitterCmd(base, true);

    /* Enable tx empty interrupt */
    LPUART_SetIntMode(base, LPUART_INT_TX_DATA_REG_EMPTY, true);

    return STATUS_SUCCESS;
}

#if FEATURE_LPUART_HAS_DMA_ENABLE
/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_StartSendDataUsingDma
 * Description   : Initiate (start) a transmit by beginning the process of
 * sending data using DMA transfers.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t LPUART_DRV_StartSendDataUsingDma(uint32_t instance,
                                                 const uint8_t * txBuff,
                                                 uint32_t txSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(txBuff != NULL);

    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check it's not busy transmitting data from a previous function call */
    if (lpuartState->isTxBusy)
    {
        return STATUS_BUSY;
    }

    DEV_ASSERT(txSize > 0U);

    /* Update state structure */
    lpuartState->txBuff = txBuff;
    lpuartState->txSize = 0U;
    lpuartState->isTxBusy = true;
    lpuartState->transmitStatus = STATUS_BUSY;

    /* Configure the transfer control descriptor for the previously allocated channel */
    (void)EDMA_DRV_ConfigMultiBlockTransfer(lpuartState->txDMAChannel, EDMA_TRANSFER_MEM2PERIPH, (uint32_t)txBuff,
                                             (uint32_t)(&(base->DATA)), EDMA_TRANSFER_SIZE_1B, 1U, txSize, true);

    /* Call driver function to end the transmission when the DMA transfer is done */
    (void)EDMA_DRV_InstallCallback(lpuartState->txDMAChannel,
                                   (edma_callback_t)(LPUART_DRV_TxDmaCallback),
                                   (void*)(instance));

    /* Start the DMA channel */
    (void)EDMA_DRV_StartChannel(lpuartState->txDMAChannel);

    /* Enable the LPUART transmitter */
    LPUART_SetTransmitterCmd(base, true);

    /* Enable tx DMA requests for the current instance */
    LPUART_SetTxDmaCmd(base, true);

    return STATUS_SUCCESS;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_CompleteSendDataUsingInt
 * Description   : Finish up a transmit by completing the process of sending
 * data and disabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_CompleteSendDataUsingInt(uint32_t instance)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    if (lpuartState->transmitStatus == STATUS_BUSY)
    {
        /* If the transfer is completed, update the transmit status */
        lpuartState->transmitStatus = STATUS_SUCCESS;
    }
    else
    {
        /* If the transfer is aborted or timed out, disable tx empty interrupt */
        LPUART_SetIntMode(base, LPUART_INT_TX_DATA_REG_EMPTY, false);
    }

    /* Disable transmission complete interrupt */
    LPUART_SetIntMode(base, LPUART_INT_TX_COMPLETE, false);

    /* Disable transmitter */
    LPUART_SetTransmitterCmd(base, false);

    /* Update the internal busy flag */
    lpuartState->isTxBusy = false;

    /* Signal the synchronous completion object. */
    if (lpuartState->isTxBlocking)
    {
        (void)OSIF_SemaPost(&lpuartState->txComplete);
    }
}

#if FEATURE_LPUART_HAS_DMA_ENABLE
/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_TxDmaCallback
 * Description   : Finish up a transmit by completing the process of sending
 * data and disabling the DMA requests. This is a callback for DMA major loop
 * completion, so it must match the DMA callback signature.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_TxDmaCallback(void * parameter, edma_chn_status_t status)
{
    uint32_t instance = ((uint32_t)parameter);
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];

    /* Check if the DMA transfer completed with errors */
    if (status == EDMA_CHN_ERROR)
    {
        /* Update the status */
        lpuartState->transmitStatus = STATUS_ERROR;
        /* Stop the transfer */
        LPUART_DRV_StopTxDma(instance);
        /* Notify the application that an error occurred */
        if (lpuartState->txCallback != NULL)
        {
            lpuartState->txCallback(lpuartState, UART_EVENT_ERROR, lpuartState->txCallbackParam);
        }
    }
    else
    {
        /* Invoke callback if there is one */
        if (lpuartState->txCallback != NULL)
        {
            /* Allow the user to provide a new buffer, for continuous transmission */
            lpuartState->txCallback(lpuartState, UART_EVENT_TX_EMPTY, lpuartState->txCallbackParam);
        }

        /* If the callback has updated the tx buffer, update the DMA descriptor to continue the transfer;
         * otherwise, stop the current transfer.
         */
        if (lpuartState->txSize > 0U)
        {
            /* Set the source address and the number of minor loops (bytes to be transfered) */
            EDMA_DRV_SetSrcAddr(lpuartState->txDMAChannel, (uint32_t)(lpuartState->txBuff));
            EDMA_DRV_SetMajorLoopIterationCount(lpuartState->txDMAChannel, lpuartState->txSize);

            /* Now that this tx is set up, clear remaining bytes count */
            lpuartState->txSize = 0U;

            /* Re-start the channel */
            (void)EDMA_DRV_StartChannel(lpuartState->txDMAChannel);
        }
        else
        {
            /* Enable transmission complete interrupt */
            LPUART_SetIntMode(base, LPUART_INT_TX_COMPLETE, true);
        }
    }
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_StartReceiveDataUsingInt
 * Description   : Initiate (start) a receive by beginning the process of
 * receiving data and enabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t LPUART_DRV_StartReceiveDataUsingInt(uint32_t instance,
                                                    uint8_t * rxBuff,
                                                    uint32_t rxSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(rxBuff != NULL);

    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];

    /* Check it's not busy receiving data from a previous function call */
    if (lpuartState->isRxBusy)
    {
        return STATUS_BUSY;
    }

    /* Check the validity of the parameters */
    DEV_ASSERT(rxSize > 0U);
    DEV_ASSERT((lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR) ||
               ((rxSize & 1U) == 0U));

    /* Initialize the module driver state struct to indicate transfer in progress
     * and with the buffer and byte count data. */
    lpuartState->isRxBusy = true;
    lpuartState->rxBuff = rxBuff;
    lpuartState->rxSize = rxSize;
    lpuartState->receiveStatus = STATUS_BUSY;

    /* Enable the receiver */
    LPUART_SetReceiverCmd(base, true);

    /* Enable error interrupts */
    LPUART_SetErrorInterrupts(base, true);

    /* Enable receive data full interrupt */
    LPUART_SetIntMode(base, LPUART_INT_RX_DATA_REG_FULL, true);

    return STATUS_SUCCESS;
}

#if FEATURE_LPUART_HAS_DMA_ENABLE
/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_StartReceiveDataUsingDma
 * Description   : Initiate (start) a receive by beginning the process of
 * receiving data using DMA transfers.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t LPUART_DRV_StartReceiveDataUsingDma(uint32_t instance,
                                                    uint8_t * rxBuff,
                                                    uint32_t rxSize)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);
    DEV_ASSERT(rxBuff != NULL);

    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Check it's not busy transmitting data from a previous function call */
    if (lpuartState->isRxBusy)
    {
        return STATUS_BUSY;
    }

    DEV_ASSERT(rxSize > 0U);

    /* Configure the transfer control descriptor for the previously allocated channel */
    (void)EDMA_DRV_ConfigMultiBlockTransfer(lpuartState->rxDMAChannel, EDMA_TRANSFER_PERIPH2MEM,
                                            (uint32_t)(&(base->DATA)), (uint32_t)rxBuff, EDMA_TRANSFER_SIZE_1B,
                                            1U, rxSize, true);

    /* Call driver function to end the reception when the DMA transfer is done */
    (void)EDMA_DRV_InstallCallback(lpuartState->rxDMAChannel,
                                   (edma_callback_t)(LPUART_DRV_RxDmaCallback),
                                   (void*)(instance));

    /* Start the DMA channel */
    (void)EDMA_DRV_StartChannel(lpuartState->rxDMAChannel);

    /* Update the state structure */
    lpuartState->rxBuff = rxBuff;
    lpuartState->rxSize = 0U;
    lpuartState->isRxBusy = true;
    lpuartState->receiveStatus = STATUS_BUSY;

    /* Enable the receiver */
    LPUART_SetReceiverCmd(base, true);

    /* Enable error interrupts */
    LPUART_SetErrorInterrupts(base, true);

    /* Enable rx DMA requests for the current instance */
    LPUART_SetRxDmaCmd(base, true);

    return STATUS_SUCCESS;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_CompleteReceiveDataUsingInt
 * Description   : Finish up a receive by completing the process of receiving data
 * and disabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_CompleteReceiveDataUsingInt(uint32_t instance)
{
    DEV_ASSERT(instance < LPUART_INSTANCE_COUNT);

    uint8_t tmpByte;
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];

    /* Disable receiver */
    LPUART_SetReceiverCmd(base, false);

    /* Disable error interrupts */
    LPUART_SetErrorInterrupts(base, false);

    /* Read dummy to clear RDRF flag */
    LPUART_Getchar(base, &tmpByte);

    /* Disable receive data full and rx overrun interrupt. */
    LPUART_SetIntMode(base, LPUART_INT_RX_DATA_REG_FULL, false);

    /* Signal the synchronous completion object. */
    if (lpuartState->isRxBlocking)
    {
        (void)OSIF_SemaPost(&lpuartState->rxComplete);
        lpuartState->isRxBlocking = false;
    }

    /* Update the information of the module driver state */
    lpuartState->isRxBusy = false;
    if (lpuartState->receiveStatus == STATUS_BUSY)
    {
        lpuartState->receiveStatus = STATUS_SUCCESS;
    }
}

#if FEATURE_LPUART_HAS_DMA_ENABLE
/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_RxDmaCallback
 * Description   : Finish up a receive by completing the process of receiving data
 * and disabling the DMA requests. This is a callback for DMA major loop
 * completion, so it must match the DMA callback signature.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_RxDmaCallback(void * parameter, edma_chn_status_t status)
{
    uint32_t instance = ((uint32_t)parameter);
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];

    if (status == EDMA_CHN_ERROR)
    {
        /* Update the status */
        lpuartState->receiveStatus = STATUS_ERROR;
        /* Stop the transfer */
        LPUART_DRV_StopRxDma(instance);
        /* Notify the application that an error occurred */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_ERROR, lpuartState->rxCallbackParam);
        }

        /* Clear the flags */
        LPUART_DRV_ClearErrorFlags(base);
    }

    /* Return if an error occurred; error cases are treated by the interrupt handler */
    if (lpuartState->receiveStatus != STATUS_BUSY)
    {
        return;
    }

    /* Invoke callback if there is one */
    if (lpuartState->rxCallback != NULL)
    {
        /* Allow the user to provide a new buffer inside the callback, to continue the reception */
        lpuartState->rxCallback(lpuartState, UART_EVENT_RX_FULL, lpuartState->rxCallbackParam);
    }

    /* If the callback has updated the rx buffer, update the DMA descriptor to continue the transfer;
     * otherwise, stop the current transfer.
     */
    if (lpuartState->rxSize > 0U)
    {
        /* Set the source address and the number of minor loops (bytes to be transfered) */
        EDMA_DRV_SetDestAddr(lpuartState->rxDMAChannel, (uint32_t)(lpuartState->rxBuff));
        EDMA_DRV_SetMajorLoopIterationCount(lpuartState->rxDMAChannel, lpuartState->rxSize);

        /* Now that this rx is set up, clear remaining bytes count */
        lpuartState->rxSize = 0U;

        /* Re-start the channel */
        (void)EDMA_DRV_StartChannel(lpuartState->rxDMAChannel);
    }
    else
    {
        /* Stop the reception */
        LPUART_DRV_StopRxDma(instance);

        /* Invoke the callback to notify the end of the transfer */
        if (lpuartState->rxCallback != NULL)
        {
            lpuartState->rxCallback(lpuartState, UART_EVENT_END_TRANSFER, lpuartState->rxCallbackParam);
        }

        /* Clear the flags */
        LPUART_DRV_ClearErrorFlags(base);
    }
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_PutData
 * Description   : Write data to the buffer register, according to configured
 * word length.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_PutData(uint32_t instance)
{
    const lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    LPUART_Type * base = s_lpuartBase[instance];
    uint16_t data;
    const uint8_t *txBuff = lpuartState->txBuff;

    if (lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR)
    {
        /* Transmit the data */
        LPUART_Putchar(base, *txBuff);
    }
    else if (lpuartState->bitCountPerChar == LPUART_9_BITS_PER_CHAR)
    {
        /* Create a 16-bits integer from two bytes */
        data = (uint16_t)(*txBuff);
        ++txBuff;
        data |= (uint16_t)(((uint16_t)(*txBuff)) << 8U);

        /* Transmit the data */
        LPUART_Putchar9(base, data);
    }
    else
    {
        /* Create a 16-bits integer from two bytes */
        data = (uint16_t)(*txBuff);
        ++txBuff;
        data |= (uint16_t)(((uint16_t)(*txBuff)) << 8U);

        /* Transmit the data */
        LPUART_Putchar10(base, data);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_GetData
 * Description   : Read data from the buffer register, according to configured
 * word length.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_GetData(uint32_t instance)
{
    const lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    const LPUART_Type * base = s_lpuartBase[instance];
    uint16_t data;
    uint8_t *rxBuff = lpuartState->rxBuff;

    if (lpuartState->bitCountPerChar == LPUART_8_BITS_PER_CHAR)
    {
        /* Receive the data */
        LPUART_Getchar(base, rxBuff);
    }
    else if (lpuartState->bitCountPerChar == LPUART_9_BITS_PER_CHAR)
    {
        /* Receive the data */
        LPUART_Getchar9(base, &data);

        /* Write the least significant bits to the receive buffer */
        *rxBuff = (uint8_t)(data & 0xFFU);
        ++rxBuff;
        /* Write the ninth bit to the subsequent byte in the rx buffer */
        *rxBuff = (uint8_t)(data >> 8U);
    }
    else
    {
        /* Receive the data */
        LPUART_Getchar10(base, &data);

        /* Write the least significant bits to the receive buffer */
        *rxBuff = (uint8_t)(data & 0xFFU);
        ++rxBuff;
        /* Write the ninth and tenth bits to the subsequent byte in the rx buffer */
        *rxBuff = (uint8_t)(data >> 8U);
    }
}

#if FEATURE_LPUART_HAS_DMA_ENABLE
/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_StopTxDma
 * Description   : Finish up a DMA transmission by disabling the DMA requests,
 * transmission complete interrupt and tx logic. This function also resets the
 * internal driver state (busy flag/tx semaphore).
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_StopTxDma(uint32_t instance)
{
    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];

    /* Disable tx DMA requests for the current instance */
    LPUART_SetTxDmaCmd(base, false);

    /* Stop the dma channel */
    (void)EDMA_DRV_StopChannel(lpuartState->txDMAChannel);

    /* Disable transmission complete interrupt */
    LPUART_SetIntMode(base, LPUART_INT_TX_COMPLETE, false);

    /* Disable transmitter */
    LPUART_SetTransmitterCmd(base, false);

    /* Signal the synchronous completion object. */
    if (lpuartState->isTxBlocking)
    {
        (void)OSIF_SemaPost(&lpuartState->txComplete);
    }

    if (lpuartState->transmitStatus == STATUS_BUSY)
    {
        /* If the transfer is completed, update the transmit status */
        lpuartState->transmitStatus = STATUS_SUCCESS;
    }

    /* Update the internal busy flag */
    lpuartState->isTxBusy = false;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_DRV_StopRxDma
 * Description   : Finish up a DMA reception by disabling the DMA requests,
 * error interrupts and rx logic. This function also resets the internal driver
 * state (busy flag/rx semaphore).
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void LPUART_DRV_StopRxDma(uint32_t instance)
{
    LPUART_Type * base = s_lpuartBase[instance];
    lpuart_state_t * lpuartState = (lpuart_state_t *)s_lpuartStatePtr[instance];
    uint8_t tmpByte;

    /* Disable receiver */
    LPUART_SetReceiverCmd(base, false);

    /* Disable error interrupts */
    LPUART_SetErrorInterrupts(base, false);

    /* Disable rx DMA requests for the current instance */
    LPUART_SetRxDmaCmd(base, false);

    /* Read dummy to clear RDRF flag */
    LPUART_Getchar(base, &tmpByte);

    /* Stop the DMA channel */
    (void)EDMA_DRV_StopChannel(lpuartState->rxDMAChannel);

    /* Signal the synchronous completion object. */
    if (lpuartState->isRxBlocking)
    {
        (void)OSIF_SemaPost(&lpuartState->rxComplete);
        lpuartState->isRxBlocking = false;
    }

    /* Update the internal driver status */
    if (lpuartState->receiveStatus == STATUS_BUSY)
    {
        lpuartState->receiveStatus = STATUS_SUCCESS;
    }

    /* Update the information of the module driver state */
    lpuartState->isRxBusy = false;
}
#endif

/*******************************************************************************
 * EOF
 ******************************************************************************/
