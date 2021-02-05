/*
 * Copyright (c) 2013 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2018 NXP
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
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in all flexio drivers so it must remain global.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and
 * integer type.
 * The cast is required to perform a conversion between a pointer and an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from pointer to unsigned long, Cast from unsigned long to pointer.
 * The cast is required to perform a conversion between a pointer and an unsigned long define,
 * representing an address.
 */

#include "edma_irq.h"
#include "clock_manager.h"
#include "interrupt_manager.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

#ifdef FEATURE_DMA_HWV3

/*! @brief Array of base addresses for DMA instances. */
static DMA_Type * const s_edmaBase[DMA_INSTANCE_COUNT] = DMA_BASE_PTRS;

#ifdef FEATURE_DMAMUX_AVAILABLE
/*! @brief Array of base addresses for DMAMUX instances. */
static DMAMUX_Type * const s_dmaMuxBase[DMAMUX_INSTANCE_COUNT] = DMAMUX_BASE_PTRS;
#endif /* FEATURE_DMAMUX_AVAILABLE */

/*! @brief Array of default DMA channel interrupt handlers. */
static const IRQn_Type s_edmaIrqId[FEATURE_DMA_VIRTUAL_CHANNELS_INTERRUPT_LINES] = DMA_CHN_IRQS;

#ifdef FEATURE_DMA_HAS_ERROR_IRQ
/*! @brief Array of default DMA error interrupt handlers. */
static const IRQn_Type s_edmaErrIrqId[FEATURE_DMA_VIRTUAL_ERROR_INTERRUPT_LINES] = DMA_ERROR_IRQS;
#endif /* FEATURE_DMA_HAS_ERROR_IRQ */

#else /* FEATURE_DMA_HWV3 */
    
/*! @brief Array of base addresses for DMA instances. */
static DMA_Type * const s_edmaBase[DMA_INSTANCE_COUNT] = DMA_BASE_PTRS;

#ifdef FEATURE_DMAMUX_AVAILABLE
/*! @brief Array of base addresses for DMAMUX instances. */
static DMAMUX_Type * const s_dmaMuxBase[DMAMUX_INSTANCE_COUNT] = DMAMUX_BASE_PTRS;
#endif /* FEATURE_DMAMUX_AVAILABLE */

/*! @brief Array of default DMA channel interrupt handlers. */
static const IRQn_Type s_edmaIrqId[FEATURE_DMA_VIRTUAL_CHANNELS_INTERRUPT_LINES] = DMA_CHN_IRQS;

#ifdef FEATURE_DMA_HAS_ERROR_IRQ
/*! @brief Array of default DMA error interrupt handlers. */
static const IRQn_Type s_edmaErrIrqId[FEATURE_DMA_VIRTUAL_ERROR_INTERRUPT_LINES] = DMA_ERROR_IRQS;
#endif /* FEATURE_DMA_HAS_ERROR_IRQ */

#endif /* FEATURE_DMA_HWV3 */

#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
/*! @brief Array for eDMA & DMAMUX clock sources. */
static const clock_names_t s_edmaClockNames[DMA_INSTANCE_COUNT] = FEATURE_DMA_CLOCK_NAMES;
#ifdef FEATURE_DMAMUX_AVAILABLE
static const clock_names_t s_dmamuxClockNames[DMAMUX_INSTANCE_COUNT] = FEATURE_DMAMUX_CLOCK_NAMES;
#endif /* FEATURE_DMAMUX_AVAILABLE */
#endif /* (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT) */

/*! @brief EDMA global structure to maintain eDMA state */
static edma_state_t * s_virtEdmaState;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/                                    
static void EDMA_DRV_ClearIntStatus(uint8_t virtualChannel);
static void EDMA_DRV_ClearSoftwareTCD(edma_software_tcd_t *stcd);
static void EDMA_DRV_ClearStructure(uint8_t *sructPtr, size_t size);
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
static bool EDMA_DRV_ValidTransferSize(edma_transfer_size_t size);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_Init
 * Description   : Initializes the eDMA module.
 *
 * Implements    : EDMA_DRV_Init_Activity
 *END**************************************************************************/
status_t EDMA_DRV_Init(edma_state_t *edmaState,
                       const edma_user_config_t *userConfig,
                       edma_chn_state_t * const chnStateArray[],
                       const edma_channel_config_t * const chnConfigArray[],
                       uint32_t chnCount)
{
    uint32_t index = 0U;
    DMA_Type *edmaRegBase = NULL;
    IRQn_Type irqNumber = NotAvail_IRQn;
    status_t edmaStatus = STATUS_SUCCESS;
    status_t chnInitStatus = STATUS_SUCCESS;
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    uint32_t freq = 0U;
    status_t clockManagerStatus = STATUS_SUCCESS;
#endif

    /* Check the state and configuration structure pointers are valid */
    DEV_ASSERT((edmaState != NULL) && (userConfig != NULL));

    /* Check the module has not already been initialized */
    DEV_ASSERT(s_virtEdmaState == NULL);

#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    /* Check that eDMA and DMAMUX modules are clock gated on */
    for (index = 0U; index < (uint32_t)DMA_INSTANCE_COUNT; index++)
    {
        clockManagerStatus = CLOCK_SYS_GetFreq(s_edmaClockNames[index], &freq);
        DEV_ASSERT(clockManagerStatus == STATUS_SUCCESS);
    }
#ifdef FEATURE_DMAMUX_AVAILABLE
    for (index = 0U; index < (uint32_t)DMAMUX_INSTANCE_COUNT; index++)
    {
        clockManagerStatus = CLOCK_SYS_GetFreq(s_dmamuxClockNames[index], &freq);
        DEV_ASSERT(clockManagerStatus == STATUS_SUCCESS);
    }
#endif /* FEATURE_DMAMUX_AVAILABLE */
#endif /* (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT) */

    /* Save the runtime state structure for the driver */
    s_virtEdmaState = edmaState;

    /* Clear the state structure. */
    EDMA_DRV_ClearStructure((uint8_t *)s_virtEdmaState, sizeof(edma_state_t));

    /* Init all DMA instances */
    for(index = 0U; index < (uint32_t)DMA_INSTANCE_COUNT; index++)
    {
        edmaRegBase = s_edmaBase[index];

        /* Init eDMA module on hardware level. */
        EDMA_Init(edmaRegBase);

#ifdef FEATURE_DMA_HWV3
        /* Set arbitration mode */
        EDMA_SetChannelArbitrationMode(edmaRegBase, userConfig->chnArbitration);
#else /* FEATURE_DMA_HWV3 */
        /* Set arbitration mode */
        EDMA_SetChannelArbitrationMode(edmaRegBase, userConfig->chnArbitration);
#if (FEATURE_DMA_CHANNEL_GROUP_COUNT > 0x1U)        
        EDMA_SetGroupArbitrationMode(edmaRegBase, userConfig->groupArbitration);
        EDMA_SetGroupPriority(edmaRegBase, userConfig->groupPriority);
#endif /* (FEATURE_DMA_CHANNEL_GROUP_COUNT > 0x1U) */
#endif /* FEATURE_DMA_HWV3 */
        /* Set 'Halt on error' configuration */
        EDMA_SetHaltOnErrorCmd(edmaRegBase, userConfig->haltOnError);
    }

#if defined FEATURE_DMA_HAS_ERROR_IRQ
    /* Enable the error interrupts for eDMA module. */
    for (index = 0U; index < (uint32_t)FEATURE_DMA_VIRTUAL_ERROR_INTERRUPT_LINES; index++)
    {
        /* Enable channel interrupt ID. */
        irqNumber = s_edmaErrIrqId[index];
        INT_SYS_EnableIRQ(irqNumber);
    }
#endif

    /* Register all edma channel interrupt handlers into vector table. */
    for (index = 0U; index < (uint32_t)FEATURE_DMA_VIRTUAL_CHANNELS_INTERRUPT_LINES; index++)
    {
        /* Enable channel interrupt ID. */
        irqNumber = s_edmaIrqId[index];
        INT_SYS_EnableIRQ(irqNumber);
    }
    
#ifdef FEATURE_DMAMUX_AVAILABLE
    /* Initialize all DMAMUX instances */
    for (index = 0U; index < (uint32_t)DMAMUX_INSTANCE_COUNT; index++)
    {
        DMAMUX_Init(s_dmaMuxBase[index]);
    }
#endif

    /* Initialize the channels based on configuration list */
    if ((chnStateArray != NULL) && (chnConfigArray != NULL))
    {
        for (index = 0U; index < chnCount; index++)
        {
            chnInitStatus = EDMA_DRV_ChannelInit(chnStateArray[index], chnConfigArray[index]);
            if (chnInitStatus != STATUS_SUCCESS)
            {
                edmaStatus = chnInitStatus;
            }
        }
    }

    return edmaStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_Deinit
 * Description   : Deinitialize EDMA.
 *
 * Implements    : EDMA_DRV_Deinit_Activity
 *END**************************************************************************/
status_t EDMA_DRV_Deinit(void)
{
    uint32_t index = 0U;
    IRQn_Type irqNumber = NotAvail_IRQn;
    const edma_chn_state_t *chnState = NULL;

#if defined FEATURE_DMA_HAS_ERROR_IRQ
    /* Disable the error interrupts for eDMA module. */
    for (index = 0U; index < (uint32_t)FEATURE_DMA_VIRTUAL_ERROR_INTERRUPT_LINES; index++)
    {
        /* Enable channel interrupt ID. */
        irqNumber = s_edmaErrIrqId[index];
        INT_SYS_DisableIRQ(irqNumber);
    }
#endif

    if (s_virtEdmaState != NULL)
    {
        /* Release all edma channel. */
        for (index = 0U; index < (uint32_t)FEATURE_DMA_VIRTUAL_CHANNELS; index++)
        {
            /* Release all channels. */
            chnState = s_virtEdmaState->virtChnState[index];
            if (chnState != NULL)
            {
                (void) EDMA_DRV_ReleaseChannel(chnState->virtChn);
            }
        }
        for (index = 0U; index < (uint32_t)FEATURE_DMA_VIRTUAL_CHANNELS_INTERRUPT_LINES; index++)
        {
            /* Disable channel interrupts. */
            irqNumber = s_edmaIrqId[index];
            INT_SYS_DisableIRQ(irqNumber);
        }
    }

    s_virtEdmaState = NULL;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ChannelInit
 * Description   : Initialize EDMA channel.
 *
 * Implements    : EDMA_DRV_ChannelInit_Activity
 *END**************************************************************************/
status_t EDMA_DRV_ChannelInit(edma_chn_state_t *edmaChannelState,
                              const edma_channel_config_t *edmaChannelConfig)
{
    /* Check the state and configuration structure pointers are valid */
    DEV_ASSERT((edmaChannelState != NULL) && (edmaChannelConfig != NULL));

    /* Check if the module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);
    
    /* Check the channel has not already been allocated */
    DEV_ASSERT(s_virtEdmaState->virtChnState[edmaChannelConfig->virtChnConfig] == NULL);    
    
    /* Check if the channel defined by user in the channel configuration structure is valid */
    DEV_ASSERT(edmaChannelConfig->virtChnConfig < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(edmaChannelConfig->virtChnConfig);

    /* Get DMA channel from virtual channel */
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(edmaChannelConfig->virtChnConfig);

    /* Get virtual channel value */
    uint8_t virtualChannel = edmaChannelConfig->virtChnConfig;
    
    /* Get status */
    status_t retStatus = STATUS_SUCCESS;
    
    /* Load corresponding DMA instance pointer */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
        
    /* Reset the channel state structure to default value. */
    EDMA_DRV_ClearStructure((uint8_t *)edmaChannelState, sizeof(edma_chn_state_t));

#ifdef FEATURE_DMAMUX_AVAILABLE     
    retStatus = EDMA_DRV_SetChannelRequestAndTrigger(edmaChannelConfig->virtChnConfig, (uint8_t)edmaChannelConfig->source, edmaChannelConfig->enableTrigger);
#endif    

    /* Clear the TCD registers for this channel */
    EDMA_TCDClearReg(edmaRegBase, dmaChannel);

#ifdef FEATURE_DMAMUX_AVAILABLE 
    if (retStatus == STATUS_SUCCESS)
#endif
    {
        /* Set virtual channel state */
        s_virtEdmaState->virtChnState[virtualChannel] = edmaChannelState;    
        /* Set virtual channel value */
        s_virtEdmaState->virtChnState[virtualChannel]->virtChn = virtualChannel;        
        /* Set virtual channel status to normal */
        s_virtEdmaState->virtChnState[virtualChannel]->status = EDMA_CHN_NORMAL;
        
        /* Enable error interrupt for this channel */
        EDMA_SetErrorIntCmd(edmaRegBase, dmaChannel, true);        
#ifdef FEATURE_DMA_HWV3
        /* Put the channel in a priority group, as defined in configuration */
        EDMA_SetChannelPriorityGroup(edmaRegBase, dmaChannel, edmaChannelConfig->groupPriority);
#endif

        /* Set the channel priority, as defined in the configuration, only if fixed arbitration mode is selected */
        if ((EDMA_GetChannelArbitrationMode(edmaRegBase) == EDMA_ARBITRATION_FIXED_PRIORITY) &&
            (edmaChannelConfig->channelPriority != EDMA_CHN_DEFAULT_PRIORITY))
        {
            EDMA_SetChannelPriority(edmaRegBase, dmaChannel, edmaChannelConfig->channelPriority);
        }
        /* Install the user callback */
        retStatus = EDMA_DRV_InstallCallback(edmaChannelConfig->virtChnConfig, edmaChannelConfig->callback, edmaChannelConfig->callbackParam);
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_InstallCallback
 * Description   : Register callback function and parameter.
 *
 * Implements    : EDMA_DRV_InstallCallback_Activity
 *END**************************************************************************/
status_t EDMA_DRV_InstallCallback(uint8_t virtualChannel,
                                  edma_callback_t callback,
                                  void *parameter)
{
    /* Check the channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check the channel is allocated */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    s_virtEdmaState->virtChnState[virtualChannel]->callback = callback;
    s_virtEdmaState->virtChnState[virtualChannel]->parameter = parameter;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ReleaseChannel
 * Description   : Free eDMA channel's hardware and software resource.
 *
 * Implements    : EDMA_DRV_ReleaseChannel_Activity
 *END**************************************************************************/
status_t EDMA_DRV_ReleaseChannel(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check the DMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Get pointer to channel state */
    edma_chn_state_t *chnState = s_virtEdmaState->virtChnState[virtualChannel];

    /* Check that virtual channel is initialized */
    DEV_ASSERT(chnState != NULL);

    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];

    /* Stop edma channel. */
    EDMA_SetDmaRequestCmd(edmaRegBase, dmaChannel, false);

    /* Reset the channel state structure to default value. */
    EDMA_DRV_ClearStructure((uint8_t *)chnState, sizeof(edma_chn_state_t));

    s_virtEdmaState->virtChnState[virtualChannel] = NULL;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ClearIntStatus
 * Description   : Clear done and interrupt retStatus.
 *
 *END**************************************************************************/
static void EDMA_DRV_ClearIntStatus(uint8_t virtualChannel)
{
    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_ClearDoneStatusFlag(edmaRegBase, dmaChannel);
    EDMA_ClearIntStatusFlag(edmaRegBase, dmaChannel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ClearSoftwareTCD
 * Description   : Clear the software tcd structure.
 *
 *END**************************************************************************/
static void EDMA_DRV_ClearSoftwareTCD(edma_software_tcd_t *stcd)
{
    EDMA_DRV_ClearStructure((uint8_t *)stcd, sizeof(edma_software_tcd_t));
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_IRQHandler
 * Description   : EDMA IRQ handler.
 *END**************************************************************************/
void EDMA_DRV_IRQHandler(uint8_t virtualChannel)
{
    const edma_chn_state_t *chnState = s_virtEdmaState->virtChnState[virtualChannel];

    EDMA_DRV_ClearIntStatus(virtualChannel);

    if (chnState != NULL)
    {
        if (chnState->callback != NULL)
        {
            chnState->callback(chnState->parameter, chnState->status);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ErrorIRQHandler
 * Description   : EDMA error IRQ handler
 *END**************************************************************************/
void EDMA_DRV_ErrorIRQHandler(uint8_t virtualChannel)
{
    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_SetDmaRequestCmd(edmaRegBase, dmaChannel, false);
    edma_chn_state_t *chnState = s_virtEdmaState->virtChnState[virtualChannel];
    if (chnState != NULL)
    {
        EDMA_DRV_ClearIntStatus(virtualChannel);
        EDMA_ClearErrorIntStatusFlag(edmaRegBase, dmaChannel);
        chnState->status = EDMA_CHN_ERROR;
        if (chnState->callback != NULL)
        {
            chnState->callback(chnState->parameter, chnState->status);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ConfigSingleBlockTransfer
 * Description   : Configures a DMA single block transfer.
 *
 * Implements    : EDMA_DRV_ConfigSingleBlockTransfer_Activity
 *END**************************************************************************/
status_t EDMA_DRV_ConfigSingleBlockTransfer(uint8_t virtualChannel,
                                            edma_transfer_type_t type,
                                            uint32_t srcAddr,
                                            uint32_t destAddr,
                                            edma_transfer_size_t transferSize,
                                            uint32_t dataBufferSize)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    /* Check if the value passed for 'transferSize' is valid */
    DEV_ASSERT(EDMA_DRV_ValidTransferSize(transferSize));
#endif

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    uint8_t transferOffset;
    status_t retStatus = STATUS_SUCCESS;

    /* Compute the transfer offset, based on transfer size.
     * The number of bytes transferred in each source read/destination write
     * is obtained with the following formula:
     *    source_read_size = 2^SSIZE
     *    destination_write_size = 2^DSIZE
     */
    transferOffset = (uint8_t) (1U << ((uint8_t)transferSize));

    /* The number of bytes to be transferred (buffer size) must
     * be a multiple of the source read/destination write size
     */
    if ((dataBufferSize % transferOffset) != 0U)
    {
        retStatus = STATUS_ERROR;
    }

    if (retStatus == STATUS_SUCCESS)
    {
        /* Clear transfer control descriptor for the current channel */
        EDMA_TCDClearReg(edmaRegBase, dmaChannel);  

#ifdef FEATURE_DMA_ENGINE_STALL
        /* Configure the DMA Engine to stall for a number of cycles after each R/W */
        EDMA_TCDSetEngineStall(edmaRegBase, dmaChannel, EDMA_ENGINE_STALL_4_CYCLES);
#endif

#ifdef FEATURE_DMA_HWV3
        EDMA_SetMinorLoopMappingCmd(edmaRegBase, dmaChannel, false);
#else
        EDMA_SetMinorLoopMappingCmd(edmaRegBase, true);
#endif

        /* Configure source and destination addresses */
        EDMA_TCDSetSrcAddr(edmaRegBase, dmaChannel, srcAddr);
        EDMA_TCDSetDestAddr(edmaRegBase, dmaChannel, destAddr);

        /* Set transfer size (1B/2B/4B/16B/32B) */
        EDMA_TCDSetAttribute(edmaRegBase, dmaChannel, EDMA_MODULO_OFF, EDMA_MODULO_OFF, transferSize, transferSize);

        /* Configure source/destination offset. */
        switch (type)
        {
            case EDMA_TRANSFER_PERIPH2MEM:
                EDMA_TCDSetSrcOffset(edmaRegBase, dmaChannel, 0);
                EDMA_TCDSetDestOffset(edmaRegBase, dmaChannel, (int8_t) transferOffset);
                break;
            case EDMA_TRANSFER_MEM2PERIPH:
                EDMA_TCDSetSrcOffset(edmaRegBase, dmaChannel, (int8_t) transferOffset);
                EDMA_TCDSetDestOffset(edmaRegBase, dmaChannel, 0);
                break;
            case EDMA_TRANSFER_MEM2MEM:
                EDMA_TCDSetSrcOffset(edmaRegBase, dmaChannel, (int8_t) transferOffset);
                EDMA_TCDSetDestOffset(edmaRegBase, dmaChannel, (int8_t) transferOffset);
                break;
            case EDMA_TRANSFER_PERIPH2PERIPH:
                EDMA_TCDSetSrcOffset(edmaRegBase, dmaChannel, 0);
                EDMA_TCDSetDestOffset(edmaRegBase, dmaChannel, 0);
                break;
            default:
                /* This should never be reached - all the possible values have been handled. */
                break;
        }

        /* Set the total number of bytes to be transfered */
        EDMA_TCDSetNbytes(edmaRegBase, dmaChannel, dataBufferSize);

        /* Set major iteration count to 1 (single block mode) */
        EDMA_TCDSetMajorCount(edmaRegBase, dmaChannel, 1U);

        /* Enable interrupt when the transfer completes */
        EDMA_TCDSetMajorCompleteIntCmd(edmaRegBase, dmaChannel, true);
        
        /* Set virtual channel status to normal */
        s_virtEdmaState->virtChnState[virtualChannel]->status = EDMA_CHN_NORMAL;    
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ConfigMultiBlockTransfer
 * Description   : Configures a DMA single block transfer.
 *
 * Implements    : EDMA_DRV_ConfigMultiBlockTransfer_Activity
 *END**************************************************************************/
status_t EDMA_DRV_ConfigMultiBlockTransfer(uint8_t virtualChannel,
                                           edma_transfer_type_t type,
                                           uint32_t srcAddr,
                                           uint32_t destAddr,
                                           edma_transfer_size_t transferSize,
                                           uint32_t blockSize,
                                           uint32_t blockCount,
                                           bool disableReqOnCompletion)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    status_t retStatus = STATUS_SUCCESS;

    /* Configure the transfer for one data block */
    retStatus = EDMA_DRV_ConfigSingleBlockTransfer(virtualChannel, type, srcAddr, destAddr, transferSize, blockSize);

    if (retStatus == STATUS_SUCCESS)
    {
        DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];

        /* Set the number of data blocks */
        EDMA_TCDSetMajorCount(edmaRegBase, dmaChannel, blockCount);

        /* Enable/disable requests upon completion */
        EDMA_TCDSetDisableDmaRequestAfterTCDDoneCmd(edmaRegBase, dmaChannel, disableReqOnCompletion);
    }

    return retStatus;
}
/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ConfigLoopTransfer
 * Description   : Configures the DMA transfer in a loop.
 *
 * Implements    : EDMA_DRV_ConfigLoopTransfer_Activity
 *END**************************************************************************/
status_t EDMA_DRV_ConfigLoopTransfer(uint8_t virtualChannel,
                                     const edma_transfer_config_t *transferConfig)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Check the transfer configuration structure is valid */
    DEV_ASSERT(transferConfig != NULL);

    /* Check the minor/major loop properties are defined */
    DEV_ASSERT(transferConfig->loopTransferConfig != NULL);
    
    /* If the modulo feature is enabled, check alignment of addresses */
    DEV_ASSERT((transferConfig->srcModulo == EDMA_MODULO_OFF) ||
               ((transferConfig->srcAddr % (((uint32_t)1U) << (uint32_t)transferConfig->srcModulo)) == 0U));
    DEV_ASSERT((transferConfig->destModulo == EDMA_MODULO_OFF) ||
               ((transferConfig->destAddr % (((uint32_t)1U) << (uint32_t)transferConfig->destModulo)) == 0U));

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

#ifdef FEATURE_DMA_HWV3
    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);
#endif

    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];

#ifdef FEATURE_DMA_HWV3
    EDMA_SetMinorLoopMappingCmd(edmaRegBase, dmaChannel, true);
#else
    EDMA_SetMinorLoopMappingCmd(edmaRegBase, true);
#endif
    /* Write the configuration in the transfer control descriptor registers */
    EDMA_DRV_PushConfigToReg(virtualChannel, transferConfig);
    
    /* Set virtual channel status to normal */
    s_virtEdmaState->virtChnState[virtualChannel]->status = EDMA_CHN_NORMAL;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ConfigScatterGatherTransfer
 * Description   : Configure eDMA for scatter/gather operation
 *
 * Implements    : EDMA_DRV_ConfigScatterGatherTransfer_Activity
 *END**************************************************************************/
status_t EDMA_DRV_ConfigScatterGatherTransfer(uint8_t virtualChannel,
                                              edma_software_tcd_t *stcd,
                                              edma_transfer_size_t transferSize,
                                              uint32_t bytesOnEachRequest,
                                              const edma_scatter_gather_list_t *srcList,
                                              const edma_scatter_gather_list_t *destList,
                                              uint8_t tcdCount)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Check the input arrays for scatter/gather operation are valid */
    DEV_ASSERT((stcd != NULL) && (srcList != NULL) && (destList != NULL));

#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    /* Check if the value passed for 'transferSize' is valid */
    DEV_ASSERT(EDMA_DRV_ValidTransferSize(transferSize));
#endif

    uint8_t i = 0U;
    uint16_t transferOffset = 0U;
    uint32_t stcdAlignedAddr = STCD_ADDR(stcd);
    edma_software_tcd_t *edmaSwTcdAddr = (edma_software_tcd_t *)stcdAlignedAddr;
    edma_loop_transfer_config_t edmaLoopConfig;
    edma_transfer_config_t edmaTransferConfig;
    status_t retStatus = STATUS_SUCCESS;

    /* Set virtual channel status to normal */
    s_virtEdmaState->virtChnState[virtualChannel]->status = EDMA_CHN_NORMAL;    
    
    /* Compute the transfer offset, based on transfer size.
     * The number of bytes transferred in each source read/destination write
     * is obtained with the following formula:
     *    source_read_size = 2^SSIZE
     *    destination_write_size = 2^DSIZE
     */
    transferOffset = (uint16_t) (1UL << ((uint16_t)transferSize));

    /* The number of bytes to be transferred on each request must
     * be a multiple of the source read/destination write size
     */
    if ((bytesOnEachRequest % transferOffset) != 0U)
    {
        retStatus = STATUS_ERROR;
    }

    /* Clear the configuration structures before initializing them. */
    EDMA_DRV_ClearStructure((uint8_t *)(&edmaTransferConfig), sizeof(edma_transfer_config_t));
    EDMA_DRV_ClearStructure((uint8_t *)(&edmaLoopConfig), sizeof(edma_loop_transfer_config_t));

    /* Configure the transfer for scatter/gather mode. */
    edmaTransferConfig.srcLastAddrAdjust = 0;
    edmaTransferConfig.destLastAddrAdjust = 0;
    edmaTransferConfig.srcModulo = EDMA_MODULO_OFF;
    edmaTransferConfig.destModulo = EDMA_MODULO_OFF;
    edmaTransferConfig.srcTransferSize = transferSize;
    edmaTransferConfig.destTransferSize = transferSize;
    edmaTransferConfig.minorByteTransferCount = bytesOnEachRequest;
    edmaTransferConfig.interruptEnable = true;
    edmaTransferConfig.scatterGatherEnable = true;
    edmaTransferConfig.loopTransferConfig = &edmaLoopConfig;
    edmaTransferConfig.loopTransferConfig->srcOffsetEnable = false;
    edmaTransferConfig.loopTransferConfig->dstOffsetEnable = false;
    edmaTransferConfig.loopTransferConfig->minorLoopChnLinkEnable = false;
    edmaTransferConfig.loopTransferConfig->majorLoopChnLinkEnable = false;

    /* Copy scatter/gather lists to transfer configuration*/
    for (i = 0U; (i < tcdCount) && (retStatus == STATUS_SUCCESS); i++)
    {
        edmaTransferConfig.srcAddr = srcList[i].address;
        edmaTransferConfig.destAddr = destList[i].address;
        if ((srcList[i].length != destList[i].length) || (srcList[i].type != destList[i].type))
        {
            retStatus = STATUS_ERROR;
            break;
        }
        edmaTransferConfig.loopTransferConfig->majorLoopIterationCount = srcList[i].length/bytesOnEachRequest;

        switch (srcList[i].type)
        {
            case EDMA_TRANSFER_PERIPH2MEM:
                /* Configure Source Read. */
                edmaTransferConfig.srcOffset = 0;
                /* Configure Dest Write. */
                edmaTransferConfig.destOffset = (int16_t) transferOffset;
                break;
            case EDMA_TRANSFER_MEM2PERIPH:
                /* Configure Source Read. */
                edmaTransferConfig.srcOffset = (int16_t) transferOffset;
                /* Configure Dest Write. */
                edmaTransferConfig.destOffset = 0;
                break;
            case EDMA_TRANSFER_MEM2MEM:
                /* Configure Source Read. */
                edmaTransferConfig.srcOffset = (int16_t) transferOffset;
                /* Configure Dest Write. */
                edmaTransferConfig.destOffset = (int16_t) transferOffset;
                break;
            case EDMA_TRANSFER_PERIPH2PERIPH:
                /* Configure Source Read. */
                edmaTransferConfig.srcOffset = 0;
                /* Configure Dest Write. */
                edmaTransferConfig.destOffset = 0;
                break;
            default:
                /* This should never be reached - all the possible values have been handled. */
                break;
        }

        /* Configure the pointer to next software TCD structure; for the last one, this address should be 0 */
        if (i == ((uint8_t)(tcdCount - 1U)))
        {
            edmaTransferConfig.scatterGatherNextDescAddr = 0U;
        }
        else
        {
            edma_software_tcd_t * ptNextAddr = &edmaSwTcdAddr[i];
            edmaTransferConfig.scatterGatherNextDescAddr = ((uint32_t) ptNextAddr);
        }

        if (i == 0U)
        {
            /* Push the configuration for the first descriptor to registers */
            EDMA_DRV_PushConfigToReg(virtualChannel, &edmaTransferConfig);
        }
        else
        {
            /* Copy configuration to software TCD structure */
            EDMA_DRV_PushConfigToSTCD(&edmaTransferConfig, &edmaSwTcdAddr[i - 1U]);
        }
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_StartChannel
 * Description   : Starts an eDMA channel.
 *
 * Implements    : EDMA_DRV_StartChannel_Activity
 *END**************************************************************************/
status_t EDMA_DRV_StartChannel(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Enable requests for current channel */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_SetDmaRequestCmd(edmaRegBase, dmaChannel, true);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_StopChannel
 * Description   : Stops an eDMA channel.
 *
 * Implements    : EDMA_DRV_StopChannel_Activity
 *END**************************************************************************/
status_t EDMA_DRV_StopChannel(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Disable requests for current channel */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_SetDmaRequestCmd(edmaRegBase, dmaChannel, false);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetChannelRequestAndTrigger
 * Description   : Sets DMA channel request source in DMAMUX and controls
 *                 the DMA channel periodic triggering.
 *
 * Implements    : EDMA_DRV_SetChannelRequestAndTrigger_Activity
 *END**************************************************************************/
status_t EDMA_DRV_SetChannelRequestAndTrigger(uint8_t virtualChannel,
                                              uint8_t request,
                                              bool enableTrigger)
{
    /* Check the virtual channel number is valid */
    DEV_ASSERT(virtualChannel < (uint32_t)FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

#ifdef FEATURE_DMAMUX_AVAILABLE
    /* Retrieve the DMAMUX instance serving this request */
    uint8_t dmaMuxInstance = (uint8_t)FEATURE_DMAMUX_REQ_SRC_TO_INSTANCE(request);

    /* Get request index for the corresponding DMAMUX instance */
    uint8_t dmaMuxRequest = (uint8_t)FEATURE_DMAMUX_REQ_SRC_TO_CH(request);

    /* Get DMAMUX channel for the selected request */
    uint8_t dmaMuxChannel = (uint8_t)FEATURE_DMAMUX_DMA_CH_TO_CH(virtualChannel);

    /* Retrieve the appropriate DMAMUX instance */
    DMAMUX_Type *dmaMuxRegBase = s_dmaMuxBase[dmaMuxInstance];

    /* Set request and trigger */
	DMAMUX_SetChannelCmd(dmaMuxRegBase, dmaMuxChannel, false);
    DMAMUX_SetChannelSource(dmaMuxRegBase, dmaMuxChannel, dmaMuxRequest);
#ifdef FEATURE_DMAMUX_HAS_TRIG	
	DMAMUX_SetChannelTrigger(dmaMuxRegBase, dmaMuxChannel, enableTrigger);
#else
    (void)enableTrigger;	
#endif	
	DMAMUX_SetChannelCmd(dmaMuxRegBase, dmaMuxChannel, true);
    
    return STATUS_SUCCESS;
#else
    (void)virtualChannel;    
    (void)request;
    (void)enableTrigger;    
    return STATUS_UNSUPPORTED;
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ClearTCD
 * Description   : Clears all registers to 0 for the hardware TCD.
 *
 * Implements    : EDMA_DRV_ClearTCD_Activity
 *END**************************************************************************/
void EDMA_DRV_ClearTCD(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Clear the TCD memory */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDClearReg(edmaRegBase, dmaChannel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetSrcAddr
 * Description   : Configures the source address for the eDMA channel.
 *
 * Implements    : EDMA_DRV_SetSrcAddr_Activity
 *END**************************************************************************/
void EDMA_DRV_SetSrcAddr(uint8_t virtualChannel,
                         uint32_t address)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD source address */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetSrcAddr(edmaRegBase, dmaChannel, address);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetSrcOffset
 * Description   : Configures the source address signed offset for the eDMA channel.
 *
 * Implements    : EDMA_DRV_SetSrcOffset_Activity
 *END**************************************************************************/
void EDMA_DRV_SetSrcOffset(uint8_t virtualChannel,
                           int16_t offset)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD source offset */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetSrcOffset(edmaRegBase, dmaChannel, offset);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetSrcReadChunkSize
 * Description   : Configures the source read data chunk size (transferred in a read sequence).
 *
 * Implements    : EDMA_DRV_SetSrcReadChunkSize_Activity
 *END**************************************************************************/
void EDMA_DRV_SetSrcReadChunkSize(uint8_t virtualChannel,
                                  edma_transfer_size_t size)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD source transfer size */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetSrcTransferSize(edmaRegBase, dmaChannel, size);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetSrcLastAddrAdjustment
 * Description   : Configures the source address last adjustment.
 *
 * Implements    : EDMA_DRV_SetSrcLastAddrAdjustment_Activity
 *END**************************************************************************/
void EDMA_DRV_SetSrcLastAddrAdjustment(uint8_t virtualChannel,
                                       int32_t adjust)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD source last adjustment */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetSrcLastAdjust(edmaRegBase, dmaChannel, adjust);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetDestLastAddrAdjustment
 * Description   : Configures the source address last adjustment.
 *
 * Implements    : EDMA_DRV_SetDestLastAddrAdjustment_Activity
 *END**************************************************************************/
void EDMA_DRV_SetDestLastAddrAdjustment(uint8_t virtualChannel,
                                        int32_t adjust)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD source last adjustment */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetDestLastAdjust(edmaRegBase, dmaChannel, adjust);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetDestAddr
 * Description   : Configures the destination address for the eDMA channel.
 *
 * Implements    : EDMA_DRV_SetDestAddr_Activity
 *END**************************************************************************/
void EDMA_DRV_SetDestAddr(uint8_t virtualChannel,
                          uint32_t address)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD destination address */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetDestAddr(edmaRegBase, dmaChannel, address);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetDestOffset
 * Description   : Configures the destination address signed offset for the eDMA channel.
 *
 * Implements    : EDMA_DRV_SetDestOffset_Activity
 *END**************************************************************************/
void EDMA_DRV_SetDestOffset(uint8_t virtualChannel,
                            int16_t offset)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD destination offset */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetDestOffset(edmaRegBase, dmaChannel, offset);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetDestWriteChunkSize
 * Description   : Configures the destination data chunk size (transferred in a write sequence).
 *
 * Implements    : EDMA_DRV_SetDestWriteChunkSize_Activity
 *END**************************************************************************/
void EDMA_DRV_SetDestWriteChunkSize(uint8_t virtualChannel,
                                    edma_transfer_size_t size)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD source transfer size */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetDestTransferSize(edmaRegBase, dmaChannel, size);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetMinorLoopBlockSize
 * Description   : Configures the number of bytes to be transferred in each service request of the channel.
 *
 * Implements    : EDMA_DRV_SetMinorLoopBlockSize_Activity
 *END**************************************************************************/
void EDMA_DRV_SetMinorLoopBlockSize(uint8_t virtualChannel,
                                    uint32_t nbytes)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set channel TCD minor loop block size */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetNbytes(edmaRegBase, dmaChannel, nbytes);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetMajorLoopIterationCount
 * Description   : Configures the number of major loop iterations.
 *
 * Implements    : EDMA_DRV_SetMajorLoopIterationCount_Activity
 *END**************************************************************************/
void EDMA_DRV_SetMajorLoopIterationCount(uint8_t virtualChannel,
                                         uint32_t majorLoopCount)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Set the major loop iteration count */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetMajorCount(edmaRegBase, dmaChannel, majorLoopCount);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_GetRemainingMajorIterationsCount
 * Description   : Returns the remaining major loop iteration count.
 *
 * Implements    : EDMA_DRV_GetRemainingMajorIterationsCount_Activity
 *END**************************************************************************/
uint32_t EDMA_DRV_GetRemainingMajorIterationsCount(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Retrieve the number of minor loops yet to be triggered */
    const DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    uint32_t count = EDMA_TCDGetCurrentMajorCount(edmaRegBase, dmaChannel);

    return count;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_SetScatterGatherLink
 * Description   : Configures the memory address of the next TCD, in scatter/gather mode.
 *
 * Implements    : EDMA_DRV_SetScatterGatherLink_Activity
 *END**************************************************************************/
void EDMA_DRV_SetScatterGatherLink(uint8_t virtualChannel,
                                   uint32_t nextTCDAddr)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Configures the memory address of the next TCD */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetScatterGatherLink(edmaRegBase, dmaChannel, nextTCDAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_DisableRequestsOnTransferComplete
 * Description   : Disables/Enables the DMA request after the major loop completes for the TCD.
 *
 * Implements    : EDMA_DRV_DisableRequestsOnTransferComplete_Activity
 *END**************************************************************************/
void EDMA_DRV_DisableRequestsOnTransferComplete(uint8_t virtualChannel,
                                                bool disable)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Disables/Enables the DMA request upon TCD completion */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TCDSetDisableDmaRequestAfterTCDDoneCmd(edmaRegBase, dmaChannel, disable);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ConfigureInterrupt
 * Description   : Disables/Enables the channel interrupt requests.
 *
 * Implements    : EDMA_DRV_ConfigureInterrupt_Activity
 *END**************************************************************************/
void EDMA_DRV_ConfigureInterrupt(uint8_t virtualChannel,
                                 edma_channel_interrupt_t intSrc,
                                 bool enable)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Disables/Enables the channel interrupt requests. */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    switch (intSrc)
    {
        case EDMA_CHN_ERR_INT:
            /* Enable channel interrupt request when error conditions occur */
            EDMA_SetErrorIntCmd(edmaRegBase, dmaChannel, enable);
            break;
        case EDMA_CHN_HALF_MAJOR_LOOP_INT:
            /* Enable channel interrupt request when major iteration count reaches halfway point */
            EDMA_TCDSetMajorHalfCompleteIntCmd(edmaRegBase, dmaChannel, enable);
            break;
        case EDMA_CHN_MAJOR_LOOP_INT:
            /* Enable channel interrupt request when major iteration count reaches zero */
            EDMA_TCDSetMajorCompleteIntCmd(edmaRegBase, dmaChannel, enable);
            break;
        default:
            /* This branch should never be reached if driver API is used properly */
            break;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_CancelTransfer
 * Description   : Cancels the running transfer for this channel.
 *
 * Implements    : EDMA_DRV_CancelTransfer_Activity
 *END**************************************************************************/
void EDMA_DRV_CancelTransfer(bool error)
{
    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    uint32_t dmaInstance = 0U;

    for(dmaInstance = 0U; dmaInstance < (uint32_t)DMA_INSTANCE_COUNT; dmaInstance++)
    {
        /* Cancel the running transfer. */
        DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
        if (error)
        {
            EDMA_CancelTransferWithError(edmaRegBase);
        }
        else
        {
            EDMA_CancelTransfer(edmaRegBase);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_TriggerSwRequest
 * Description   : Triggers a sw request for the current channel.
 *
 * Implements    : EDMA_DRV_TriggerSwRequest_Activity
 *END**************************************************************************/
void EDMA_DRV_TriggerSwRequest(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    /* Trigger the channel transfer. */
    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];
    EDMA_TriggerChannelStart(edmaRegBase, dmaChannel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_PushConfigToSTCD
 * Description   : Copy the configuration to the software TCD structure.
 *
 * Implements    : EDMA_DRV_PushConfigToSTCD_Activity
 *END**************************************************************************/
void EDMA_DRV_PushConfigToSTCD(const edma_transfer_config_t *config,
                               edma_software_tcd_t *stcd)
{
    if ((config != NULL) && (stcd != NULL))
    {
        /* Clear the array of software TCDs passed by the user */
        EDMA_DRV_ClearSoftwareTCD(stcd);

        /* Set the software TCD fields */
        stcd->ATTR = (uint16_t)(DMA_TCD_ATTR_SMOD(config->srcModulo) | DMA_TCD_ATTR_SSIZE(config->srcTransferSize) |
                                DMA_TCD_ATTR_DMOD(config->destModulo) | DMA_TCD_ATTR_DSIZE(config->destTransferSize));
        stcd->SADDR = config->srcAddr;
        stcd->SOFF = config->srcOffset;
        stcd->NBYTES = config->minorByteTransferCount;
        stcd->SLAST = config->srcLastAddrAdjust;
        stcd->DADDR = config->destAddr;
        stcd->DOFF = config->destOffset;
        stcd->CITER = (uint16_t) config->loopTransferConfig->majorLoopIterationCount;
        if (config->scatterGatherEnable)
        {
            stcd->DLAST_SGA = (int32_t) config->scatterGatherNextDescAddr;
        }
        else
        {
            stcd->DLAST_SGA = config->destLastAddrAdjust;
        }
#ifdef FEATURE_DMA_HWV3
        stcd->CSR = (uint16_t) (((config->interruptEnable ? 1UL : 0UL) << DMA_TCD_CSR_INTMAJOR_SHIFT) |
                                ((config->scatterGatherEnable ? 1UL : 0UL) << DMA_TCD_CSR_ESG_SHIFT));
#else
        stcd->CSR = (uint16_t) (((config->interruptEnable ? 1UL : 0UL) << DMA_TCD_CSR_INTMAJOR_SHIFT) |
                                  ((config->scatterGatherEnable ? 1UL : 0UL) << DMA_TCD_CSR_ESG_SHIFT));
#endif
        stcd->BITER = (uint16_t) config->loopTransferConfig->majorLoopIterationCount;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_PushConfigToReg
 * Description   : Copy the configuration to the TCD registers.
 *
 * Implements    : EDMA_DRV_PushConfigToReg_Activity
 *END**************************************************************************/
void EDMA_DRV_PushConfigToReg(uint8_t virtualChannel,
                              const edma_transfer_config_t *tcd)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    /* Check the transfer configuration structure is valid */
    DEV_ASSERT(tcd != NULL);

    /* Get DMA instance from virtual channel */
    uint8_t dmaInstance = (uint8_t)FEATURE_DMA_VCH_TO_INSTANCE(virtualChannel);

    /* Get DMA channel from virtual channel*/
    uint8_t dmaChannel = (uint8_t)FEATURE_DMA_VCH_TO_CH(virtualChannel);

    DMA_Type *edmaRegBase = s_edmaBase[dmaInstance];

    /* Clear TCD registers */
    EDMA_TCDClearReg(edmaRegBase, dmaChannel);

#ifdef FEATURE_DMA_ENGINE_STALL
    /* Configure the DMA Engine to stall for a number of cycles after each R/W */
    EDMA_TCDSetEngineStall(edmaRegBase, dmaChannel, EDMA_ENGINE_STALL_4_CYCLES);
#endif

    /* Set source and destination addresses */
    EDMA_TCDSetSrcAddr(edmaRegBase, dmaChannel, tcd->srcAddr);
    EDMA_TCDSetDestAddr(edmaRegBase, dmaChannel, tcd->destAddr);
    /* Set source/destination modulo feature and transfer size */
    EDMA_TCDSetAttribute(edmaRegBase, dmaChannel, tcd->srcModulo, tcd->destModulo,
                         tcd->srcTransferSize, tcd->destTransferSize);
    /* Set source/destination offset and last adjustment; for scatter/gather operation, destination
     * last adjustment is the address of the next TCD structure to be loaded by the eDMA engine */
    EDMA_TCDSetSrcOffset(edmaRegBase, dmaChannel, tcd->srcOffset);
    EDMA_TCDSetDestOffset(edmaRegBase, dmaChannel, tcd->destOffset);
    EDMA_TCDSetSrcLastAdjust(edmaRegBase, dmaChannel, tcd->srcLastAddrAdjust);
    
    if (tcd->scatterGatherEnable)
    {
        EDMA_TCDSetScatterGatherCmd(edmaRegBase, dmaChannel, true);
        EDMA_TCDSetScatterGatherLink(edmaRegBase, dmaChannel, tcd->scatterGatherNextDescAddr);
    }
    else
    {
        EDMA_TCDSetScatterGatherCmd(edmaRegBase, dmaChannel, false);
        EDMA_TCDSetDestLastAdjust(edmaRegBase, dmaChannel, tcd->destLastAddrAdjust);
    }

    /* Configure channel interrupt */
    EDMA_TCDSetMajorCompleteIntCmd(edmaRegBase, dmaChannel, tcd->interruptEnable);

    /* If loop configuration is available, copy minor/major loop setup to registers */
    if (tcd->loopTransferConfig != NULL)
    {
        EDMA_TCDSetSrcMinorLoopOffsetCmd(edmaRegBase, dmaChannel, tcd->loopTransferConfig->srcOffsetEnable);
        EDMA_TCDSetDestMinorLoopOffsetCmd(edmaRegBase, dmaChannel, tcd->loopTransferConfig->dstOffsetEnable);
        EDMA_TCDSetMinorLoopOffset(edmaRegBase, dmaChannel, tcd->loopTransferConfig->minorLoopOffset);
        EDMA_TCDSetNbytes(edmaRegBase, dmaChannel, tcd->minorByteTransferCount);

        EDMA_TCDSetChannelMinorLink(edmaRegBase, dmaChannel, tcd->loopTransferConfig->minorLoopChnLinkNumber,
                                    tcd->loopTransferConfig->minorLoopChnLinkEnable);
        EDMA_TCDSetChannelMajorLink(edmaRegBase, dmaChannel, tcd->loopTransferConfig->majorLoopChnLinkNumber,
                                    tcd->loopTransferConfig->majorLoopChnLinkEnable);

        EDMA_TCDSetMajorCount(edmaRegBase, dmaChannel, tcd->loopTransferConfig->majorLoopIterationCount);
    }
    else
    {
        EDMA_TCDSetNbytes(edmaRegBase, dmaChannel, tcd->minorByteTransferCount);
    }
}

#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ValidTransferSize
 * Description   : Check if the transfer size value is legal (0/1/2/4/5).
 *
 *END**************************************************************************/
static bool EDMA_DRV_ValidTransferSize(edma_transfer_size_t size)
{
    bool isValid;
    switch (size)
    {
        case EDMA_TRANSFER_SIZE_1B:
        case EDMA_TRANSFER_SIZE_2B:
        case EDMA_TRANSFER_SIZE_4B:
#ifdef FEATURE_DMA_TRANSFER_SIZE_8B
        case EDMA_TRANSFER_SIZE_8B:
#endif
#ifdef FEATURE_DMA_TRANSFER_SIZE_16B
        case EDMA_TRANSFER_SIZE_16B:
#endif
#ifdef FEATURE_DMA_TRANSFER_SIZE_32B
        case EDMA_TRANSFER_SIZE_32B:
#endif
#ifdef FEATURE_DMA_TRANSFER_SIZE_64B
        case EDMA_TRANSFER_SIZE_64B:
#endif
            isValid = true;
            break;
        default:
            isValid = false;
            break;
    }
    return isValid;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_GetChannelStatus
 * Description   : Returns the eDMA channel retStatus.
 *
 * Implements    : EDMA_DRV_GetChannelStatus_Activity
 *END**************************************************************************/
edma_chn_status_t EDMA_DRV_GetChannelStatus(uint8_t virtualChannel)
{
    /* Check that virtual channel number is valid */
    DEV_ASSERT(virtualChannel < FEATURE_DMA_VIRTUAL_CHANNELS);

    /* Check that eDMA module is initialized */
    DEV_ASSERT(s_virtEdmaState != NULL);

    /* Check that virtual channel is initialized */
    DEV_ASSERT(s_virtEdmaState->virtChnState[virtualChannel] != NULL);

    return s_virtEdmaState->virtChnState[virtualChannel]->status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_GetDmaRegBaseAddr
 * Description   : Returns the DMA register base address.
 *
 * Implements    : EDMA_DRV_GetDmaRegBaseAddr
 *END**************************************************************************/
DMA_Type * EDMA_DRV_GetDmaRegBaseAddr(uint32_t instance)
{
    /* Check that instance is valid */
    DEV_ASSERT(instance < DMA_INSTANCE_COUNT);
    
    return s_edmaBase[instance];
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EDMA_DRV_ClearStructure
 * Description   : Clears all bytes at the passed structure pointer.
 *
 *END**************************************************************************/
static void EDMA_DRV_ClearStructure(uint8_t *sructPtr, size_t size)
{
    while (size > 0U)
    {
        *sructPtr = 0;
        sructPtr ++;
        size --;
    }
}

/*******************************************************************************
 * EOF
 ******************************************************************************/

