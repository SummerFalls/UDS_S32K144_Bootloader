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
 * @file edma_hw_access.h
 */

#ifndef EDMA_HW_ACCESS_H
#define EDMA_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "edma_driver.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifdef FEATURE_DMA_ENGINE_STALL 
/*!
 * @brief Specifies the number of cycles the DMA Engine is stalled.
 */
typedef enum {
    EDMA_ENGINE_STALL_0_CYCLES = 0,
    EDMA_ENGINE_STALL_4_CYCLES = 2,
    EDMA_ENGINE_STALL_8_CYCLES = 3
} edma_engine_stall_t;
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name eDMA hw access module level functions
 * @{
 */

/*!
 * @brief Initializes eDMA module to known state.
 *
 * @param base Register base address for eDMA module.
 */
void EDMA_Init(DMA_Type * base);

/*!
 * @brief Cancels the remaining data transfer.
 *
 * This function stops the executing channel and forces the minor loop
 * to finish. The cancellation takes effect after the last write of the
 * current read/write sequence. The CX clears itself after the cancel has
 * been honored. This cancel retires the channel normally as if the minor
 * loop had completed.
 *
 * @param base Register base address for eDMA module.
 */
void EDMA_CancelTransfer(DMA_Type * base);

/*!
 * @brief Cancels the remaining data transfer and treats it as an error condition.
 *
 * This function stops the executing channel and forces the minor loop
 * to finish. The cancellation takes effect after the last write of the
 * current read/write sequence. The CX clears itself after the cancel has
 * been honoured. This cancel retires the channel normally as if the minor
 * loop had completed. Additional thing is to treat this operation as an error
 * condition.
 *
 * @param base Register base address for eDMA module.
 */
void EDMA_CancelTransferWithError(DMA_Type * base);

/*!
 * @brief Halts or does not halt the eDMA module when an error occurs.
 *
 * An error causes the HALT bit to be set. Subsequently, all service requests are ignored until the
 * HALT bit is cleared.
 *
 * @param base Register base address for eDMA module.
 * @param haltOnError Halts (true) or not halt (false) eDMA module when an error occurs.
 */
static inline void EDMA_SetHaltOnErrorCmd(DMA_Type * base, bool haltOnError)
{
    uint32_t regValTemp;
#ifdef FEATURE_DMA_HWV3
    regValTemp = base->MP_CSR;
    regValTemp &= ~(DMA_MP_CSR_HAE_MASK);
    regValTemp |= DMA_MP_CSR_HAE(haltOnError ? 1UL : 0UL);
    base->MP_CSR = regValTemp;
#else
    regValTemp = base->CR;
    regValTemp &= ~(DMA_CR_HOE_MASK);
    regValTemp |= DMA_CR_HOE(haltOnError ? 1UL : 0UL);
    base->CR = regValTemp;
#endif
}

/*! @} */

/*!
 * @name eDMA HAL driver channel priority and arbitration configuration
 * @{
 */

/*!
 * @brief Sets the eDMA channel priority.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param priority Priority of the DMA channel. Different channels should have different priority
 * setting inside a group.
 */
static inline void EDMA_SetChannelPriority(DMA_Type * base, uint8_t channel, edma_channel_priority_t priority)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif

#ifdef FEATURE_DMA_HWV3
    volatile uint32_t regValTemp;
    regValTemp = base->TCD[channel].CH_PRI;
    regValTemp &= (uint32_t)~(DMA_TCD_CH_PRI_APL_MASK);
    regValTemp |= (uint32_t)DMA_TCD_CH_PRI_APL(priority);
    base->TCD[channel].CH_PRI = regValTemp;
#else
    uint8_t regValTemp;
    uint8_t index = (uint8_t)FEATURE_DMA_CHN_TO_DCHPRI_INDEX(channel);
    regValTemp = base->DCHPRI[index];
    regValTemp &= (uint8_t)~(DMA_DCHPRI_CHPRI_MASK);
    regValTemp |= (uint8_t)DMA_DCHPRI_CHPRI(priority);
    base->DCHPRI[index] = regValTemp;
#endif
}

/*!
 * @brief Sets the channel arbitration algorithm.
 *
 * @param base Register base address for eDMA module.
 * @param channelArbitration Round-Robin way or fixed priority way.
 */
static inline void EDMA_SetChannelArbitrationMode(DMA_Type * base, edma_arbitration_algorithm_t channelArbitration)
{
    uint32_t regValTemp;
#ifdef FEATURE_DMA_HWV3
    regValTemp = base->MP_CSR;
    regValTemp &= ~(DMA_MP_CSR_ERCA_MASK);
    regValTemp |= DMA_MP_CSR_ERCA(channelArbitration);
    base->MP_CSR = regValTemp;
#else
    regValTemp = base->CR;
    regValTemp &= ~(DMA_CR_ERCA_MASK);
    regValTemp |= DMA_CR_ERCA(channelArbitration);
    base->CR = regValTemp;
#endif
}


/*!
 * @brief Gets the channel arbitration algorithm.
 *
 * @param base Register base address for eDMA module.
 * @return edma_arbitration_algorithm_t variable indicating the selected
 * channel arbitration: Round-Robin way or fixed priority way
 */
static inline edma_arbitration_algorithm_t EDMA_GetChannelArbitrationMode(const DMA_Type * base)
{
    edma_arbitration_algorithm_t retVal;
#ifdef FEATURE_DMA_HWV3
    if (((base->MP_CSR >> DMA_MP_CSR_ERCA_SHIFT) & 1U) != 0U)
#else
    if (((base->CR >> DMA_CR_ERCA_SHIFT) & 1U) != 0U)
#endif
    {
        retVal = EDMA_ARBITRATION_ROUND_ROBIN;
    }
    else
    {
        retVal = EDMA_ARBITRATION_FIXED_PRIORITY;
    }
    return retVal;
}

#if FEATURE_DMA_CHANNEL_GROUP_COUNT > 0x1U
#ifdef FEATURE_DMA_HWV3
/*!
 * @brief Sets the eDMA channel arbitration group.
 *
 * @param base Register base address for eDMA module.
 * @param channel The DMA channel.
 * @param channelGroupPriority Specify in which group the channel is.
 */
void EDMA_SetChannelPriorityGroup(DMA_Type * base, uint8_t channel, edma_group_priority_t channelPriorityGroup);
#else
/*!
 * @brief Sets the eDMA group priority.
 *
 * @param base Register base address for eDMA module.
 * @param priority Priority of the DMA groups.
 */
void EDMA_SetGroupPriority(DMA_Type * base, edma_group_priority_t priority);
#endif

/*!
 * @brief Sets the group arbitration algorithm.
 *
 * @param base Register base address for eDMA module.
 * @param groupArbitrationMode Round-Robin way or fixed priority way.
 */
#ifndef FEATURE_DMA_HWV3
static inline void EDMA_SetGroupArbitrationMode(DMA_Type * base, edma_arbitration_algorithm_t groupArbitration)
{
    uint32_t regValTemp;
    regValTemp = base->CR;
    regValTemp &= ~(DMA_CR_ERGA_MASK);
    regValTemp |= DMA_CR_ERGA(groupArbitration);
    base->CR = regValTemp;
}
#endif
#endif

/*!
 * @name eDMA HAL driver configuration and operation
 * @{
 */
/*!
 * @brief Enables/Disables the minor loop mapping.
 *
 * This function enables/disables the minor loop mapping feature.
 * If enabled, the NBYTES is redefined to include the individual enable fields and the NBYTES field. The
 * individual enable fields allow the minor loop offset to be applied to the source address, the
 * destination address, or both. The NBYTES field is reduced when either offset is enabled.
 *
 * @param base Register base address for eDMA module.
 * @param enable Enables (true) or Disable (false) minor loop mapping.
 */
#ifdef FEATURE_DMA_HWV3
static inline void EDMA_SetMinorLoopMappingCmd(DMA_Type * base, uint8_t channel, bool enable)
{
    uint32_t regValTemp;
    regValTemp = base->TCD[channel].NBYTES.MLOFFNO;
    regValTemp &= ~(DMA_TCD_NBYTES_MLOFFNO_SMLOE_MASK);
    regValTemp &= ~(DMA_TCD_NBYTES_MLOFFNO_DMLOE_MASK);
    regValTemp |= DMA_TCD_NBYTES_MLOFFNO_SMLOE(enable ? 1UL : 0UL);
    regValTemp |= DMA_TCD_NBYTES_MLOFFNO_DMLOE(enable ? 1UL : 0UL);
    base->TCD[channel].NBYTES.MLOFFNO = regValTemp;
}
#else
static inline void EDMA_SetMinorLoopMappingCmd(DMA_Type * base, bool enable)
{
    uint32_t regValTemp;
    regValTemp = base->CR;
    regValTemp &= ~(DMA_CR_EMLM_MASK);
    regValTemp |= DMA_CR_EMLM(enable ? 1UL : 0UL);
    base->CR = regValTemp;
}
#endif

/*!
 * @brief Enables/Disables the error interrupt for channels.
 *
 * @param base Register base address for eDMA module.
 * @param channel Channel indicator.
 * @param enable Enable(true) or Disable (false) error interrupt.
 */
void EDMA_SetErrorIntCmd(DMA_Type * base, uint8_t channel, bool enable);

/*!
 * @brief Gets the eDMA error interrupt status.
 *
 * @param base Register base address for eDMA module.
 * @return 32 bit variable indicating error channels. If error happens on eDMA channel n, the bit n
 * of this variable is '1'. If not, the bit n of this variable is '0'.
 */
static inline uint32_t EDMA_GetErrorIntStatusFlag(const DMA_Type * base)
{
#ifdef FEATURE_DMA_HWV3
    return ((base->MP_ES & DMA_MP_ES_ERRCHN_MASK) >> DMA_MP_ES_ERRCHN_SHIFT);
#else
    return base->ERR;
#endif
}

/*!
 * @brief Clears the error interrupt status for the eDMA channel or channels.
 *
 * @param base Register base address for eDMA module.
 * @param channel Channel indicator.
 */
static inline void EDMA_ClearErrorIntStatusFlag(DMA_Type * base, uint8_t channel)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
#ifdef FEATURE_DMA_HWV3
    base->TCD[channel].CH_ES |= DMA_TCD_CH_ES_ERR_MASK;
#else
    base->CERR = (uint8_t)channel;
#endif
}

/*!
 * @brief Enables/Disables the DMA request for the channel or all channels.
 *
 * @param base Register base address for eDMA module.
 * @param enable Enable(true) or Disable (false) DMA request.
 * @param channel Channel indicator.
 */
void EDMA_SetDmaRequestCmd(DMA_Type * base, uint8_t channel, bool enable);

/*!
 * @brief Clears the done status for a channel or all channels.
 *
 * @param base Register base address for eDMA module.
 * @param channel Channel indicator.
 */
static inline void EDMA_ClearDoneStatusFlag(DMA_Type * base, uint8_t channel)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
#ifdef FEATURE_DMA_HWV3
    base->TCD[channel].CH_CSR |= DMA_TCD_CH_CSR_DONE_MASK;
#else
    base->CDNE = (uint8_t)channel;
#endif
}

/*!
 * @brief Triggers the eDMA channel.
 *
 * @param base Register base address for eDMA module.
 * @param channel Channel indicator.
 */
static inline void EDMA_TriggerChannelStart(DMA_Type * base, uint8_t channel)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
#ifdef FEATURE_DMA_HWV3
    base->TCD[channel].CSR |= DMA_TCD_CSR_START_MASK;
#else
    base->SSRT = (uint8_t)channel;
#endif
}

/*!
 * @brief Clears the interrupt status for the eDMA channel or all channels.
 *
 * @param base Register base address for eDMA module.
 * @param channel Channel indicator.
 */
static inline void EDMA_ClearIntStatusFlag(DMA_Type * base, uint8_t channel)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
#ifdef FEATURE_DMA_HWV3
    base->TCD[channel].CH_INT |= DMA_TCD_CH_INT_INT_MASK;
#else
    base->CINT = (uint8_t)channel;
#endif
}

/*! @} */

/*!
 * @name eDMA HAL driver TCD configuration functions
 * @{
 */

/*!
 * @brief Clears all registers to 0 for the hardware TCD.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 */
void EDMA_TCDClearReg(DMA_Type * base, uint8_t channel);

#ifdef FEATURE_DMA_ENGINE_STALL
/*!
 * @brief Configures DMA engine to stall for a number of cycles after each R/W.
 *
 * @param base Register base address for eDMA module.
 * @param channel Channel indicator.
 * @param cycles Number of cycles the DMA engine is stalled after each R/W.
 */
static inline void EDMA_TCDSetEngineStall(DMA_Type * base, uint8_t channel, edma_engine_stall_t cycles)
{
#ifdef DEV_ERROR_DETECT
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif    
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= ~(DMA_TCD_CSR_BWC_MASK);    
    regValTemp |= DMA_TCD_CSR_BWC(cycles);
    base->TCD[channel].CSR = regValTemp;
}
#endif

/*!
 * @brief Configures the source address for the hardware TCD.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param address The pointer to the source memory address.
 */
static inline void EDMA_TCDSetSrcAddr(DMA_Type * base, uint8_t channel, uint32_t address)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    base->TCD[channel].SADDR = address;
}

/*!
 * @brief Configures the source address signed offset for the hardware TCD.
 *
 * Sign-extended offset applied to the current source address to form the next-state value as each
 * source read is complete.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param offset signed-offset for source address.
 */
static inline void EDMA_TCDSetSrcOffset(DMA_Type * base, uint8_t channel, int16_t offset)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    base->TCD[channel].SOFF = (uint16_t)offset;
}

/*!
 * @brief Configures the transfer attribute for the eDMA channel.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param srcModulo enumeration type for an allowed source modulo. The value defines a specific address range
 * specified as the value after the SADDR + SOFF calculation is performed on the original register
 * value. Setting this field provides the ability to implement a circular data. For data queues
 * requiring power-of-2 size bytes, the queue should start at a 0-modulo-size address and the SMOD
 * field should be set to the appropriate value for the queue, freezing the desired number of upper
 * address bits. The value programmed into this field specifies the number of the lower address bits
 * allowed to change. For a circular queue application, the SOFF is typically set to the transfer
 * size to implement post-increment addressing with SMOD function restricting the addresses to a
 * 0-modulo-size range.
 * @param destModulo Enum type for an allowed destination modulo.
 * @param srcTransferSize Enum type for source transfer size.
 * @param destTransferSize Enum type for destination transfer size.
 */
void EDMA_TCDSetAttribute(
                DMA_Type * base, uint8_t channel,
                edma_modulo_t srcModulo, edma_modulo_t destModulo,
                edma_transfer_size_t srcTransferSize, edma_transfer_size_t destTransferSize);

/*!
 * @brief Sets the source transfer size.
 *
 * Configures the source data read transfer size (1/2/4/16/32 bytes).
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param size Source transfer size.
 */
static inline void EDMA_TCDSetSrcTransferSize(DMA_Type * base, uint8_t channel, edma_transfer_size_t size)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].ATTR;
    regValTemp &= (uint16_t)(~(DMA_TCD_ATTR_SSIZE_MASK));
    regValTemp |= (uint16_t)(DMA_TCD_ATTR_SSIZE((uint16_t)size));
    base->TCD[channel].ATTR = regValTemp;
}

/*!
 * @brief Sets the destination transfer size.
 *
 * Configures the destination data write transfer size (1/2/4/16/32 bytes).
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param size Destination transfer size.
 */
static inline void EDMA_TCDSetDestTransferSize(DMA_Type * base, uint8_t channel, edma_transfer_size_t size)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].ATTR;
    regValTemp &= (uint16_t)(~(DMA_TCD_ATTR_DSIZE_MASK));
    regValTemp |= (uint16_t)(DMA_TCD_ATTR_DSIZE((uint16_t)size));
    base->TCD[channel].ATTR = regValTemp;
}

/*!
 * @brief Configures the nbytes for the eDMA channel.
 *
 * Note here that user need firstly configure the minor loop mapping feature and then call this
 * function.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param nbytes Number of bytes to be transferred in each service request of the channel
 */
void EDMA_TCDSetNbytes(DMA_Type * base, uint8_t channel, uint32_t nbytes);

/*!
 * @brief Enables/disables the source minor loop offset feature for the TCD.
 *
 * Configures whether the minor loop offset is applied to the source address
 * upon minor loop completion.
 * NOTE: EMLM bit needs to be enabled prior to calling this function, otherwise
 * it has no effect.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param enable Enables (true) or disables (false) source minor loop offset.
 */
static inline void EDMA_TCDSetSrcMinorLoopOffsetCmd(DMA_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
#ifdef FEATURE_DMA_HWV3
    uint32_t smloe = (base->TCD[channel].NBYTES.MLOFFNO & DMA_TCD_NBYTES_MLOFFNO_SMLOE_MASK) >> DMA_TCD_NBYTES_MLOFFNO_SMLOE_SHIFT;
    uint32_t dmloe = (base->TCD[channel].NBYTES.MLOFFNO & DMA_TCD_NBYTES_MLOFFNO_DMLOE_MASK) >> DMA_TCD_NBYTES_MLOFFNO_DMLOE_SHIFT;
    if ((smloe != 0UL) || (dmloe != 0UL))
    {
        uint32_t regValTemp;
        regValTemp = base->TCD[channel].NBYTES.MLOFFYES;
        regValTemp &= ~(DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK);
        regValTemp |= DMA_TCD_NBYTES_MLOFFYES_SMLOE(enable ? 1UL : 0UL);
        base->TCD[channel].NBYTES.MLOFFYES = regValTemp;
    }
#else
    if (((base->CR >> DMA_CR_EMLM_SHIFT) & 1U) != 0U)
    {
        uint32_t regValTemp;
        regValTemp = base->TCD[channel].NBYTES.MLOFFYES;
        regValTemp &= ~(DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK);
        regValTemp |= DMA_TCD_NBYTES_MLOFFYES_SMLOE(enable ? 1UL : 0UL);
        base->TCD[channel].NBYTES.MLOFFYES = regValTemp;
    }
#endif
}

/*!
 * @brief Enables/disables the destination minor loop offset feature for the TCD.
 *
 * Configures whether the minor loop offset is applied to the destination address
 * upon minor loop completion.
 * NOTE: EMLM bit needs to be enabled prior to calling this function, otherwise
 * it has no effect.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param enable Enables (true) or disables (false) destination minor loop offset.
 */
static inline void EDMA_TCDSetDestMinorLoopOffsetCmd(DMA_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
#ifdef FEATURE_DMA_HWV3
    uint32_t smloe = (base->TCD[channel].NBYTES.MLOFFNO & DMA_TCD_NBYTES_MLOFFNO_SMLOE_MASK) >> DMA_TCD_NBYTES_MLOFFNO_SMLOE_SHIFT;
    uint32_t dmloe = (base->TCD[channel].NBYTES.MLOFFNO & DMA_TCD_NBYTES_MLOFFNO_DMLOE_MASK) >> DMA_TCD_NBYTES_MLOFFNO_DMLOE_SHIFT;
    if ((smloe != 0UL) || (dmloe != 0UL))
    {
        uint32_t regValTemp;
        regValTemp = base->TCD[channel].NBYTES.MLOFFYES;
        regValTemp &= ~(DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK);
        regValTemp |= DMA_TCD_NBYTES_MLOFFYES_DMLOE(enable ? 1UL : 0UL);
        base->TCD[channel].NBYTES.MLOFFYES = regValTemp;
    }
#else
    if (((base->CR >> DMA_CR_EMLM_SHIFT) & 1U) != 0U)
    {
        uint32_t regValTemp;
        regValTemp = base->TCD[channel].NBYTES.MLOFFYES;
        regValTemp &= ~(DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK);
        regValTemp |= DMA_TCD_NBYTES_MLOFFYES_DMLOE(enable ? 1UL : 0UL);
        base->TCD[channel].NBYTES.MLOFFYES = regValTemp;
    }
#endif
}

/*!
 * @brief Configures the minor loop offset for the TCD.
 *
 * Configures the offset value. If neither source nor destination offset is enabled,
 * offset is not configured.
 * NOTE: EMLM bit needs to be enabled prior to calling this function, otherwise
 * it has no effect.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param offset Minor loop offset
 */
void EDMA_TCDSetMinorLoopOffset(DMA_Type * base, uint8_t channel, int32_t offset);

/*!
 * @brief Configures the last source address adjustment for the TCD.
 *
 * Adjustment value added to the source address at the completion of the major iteration count. This
 * value can be applied to restore the source address to the initial value, or adjust the address to
 * reference the next data structure.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param size adjustment value
 */
static inline void EDMA_TCDSetSrcLastAdjust(DMA_Type * base, uint8_t channel, int32_t size)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    base->TCD[channel].SLAST = (uint32_t)size;
}

/*!
 * @brief Configures the destination address for the TCD.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param address The pointer to the destination address.
 */
static inline void EDMA_TCDSetDestAddr(DMA_Type * base, uint8_t channel, uint32_t address)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    base->TCD[channel].DADDR = address;
}

/*!
 * @brief Configures the destination address signed offset for the TCD.
 *
 * Sign-extended offset applied to the current source address to form the next-state value as each
 * destination write is complete.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param offset signed-offset
 */
static inline void EDMA_TCDSetDestOffset(DMA_Type * base, uint8_t channel, int16_t offset)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    base->TCD[channel].DOFF = (uint16_t)offset;
}

/*!
 * @brief Configures the last source address adjustment.
 *
 * This function adds an adjustment value added to the source address at the completion of the major
 * iteration count. This value can be applied to restore the source address to the initial value, or
 * adjust the address to reference the next data structure.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param adjust adjustment value
 */
static inline void EDMA_TCDSetDestLastAdjust(DMA_Type * base, uint8_t channel, int32_t adjust)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    base->TCD[channel].DLASTSGA = (uint32_t)adjust;
}

/*!
 * @brief Configures the memory address for the next transfer TCD for the TCD.
 *
 *
 * This function enables the scatter/gather feature for the TCD and configures the next
 * TCD's address. This address points to the beginning of a 0-modulo-32 byte region containing
 * the next transfer TCD to be loaded into this channel. The channel reload is performed as the
 * major iteration count completes. The scatter/gather address must be 0-modulo-32-byte. Otherwise,
 * a configuration error is reported.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param nextTCDAddr The address of the next TCD to be linked to this TCD.
 */
void EDMA_TCDSetScatterGatherLink(DMA_Type * base, uint8_t channel, uint32_t nextTCDAddr);

/*!
 * @brief Enables/Disables the scatter/gather feature for the TCD.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param enable Enables (true) /Disables (false) scatter/gather feature.
 */
static inline void EDMA_TCDSetScatterGatherCmd(DMA_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= (uint16_t)~(DMA_TCD_CSR_ESG_MASK);
    regValTemp |= (uint16_t)DMA_TCD_CSR_ESG(enable ? 1UL : 0UL);
    base->TCD[channel].CSR = regValTemp;
}

/*!
 * @brief Configures the major channel link the TCD.
 *
 * If the major link is enabled, after the major loop counter is exhausted, the eDMA engine initiates a
 * channel service request at the channel defined by these six bits by setting that channel start
 * bits.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param majorLinkChannel channel number for major link
 * @param enable Enables (true) or Disables (false) channel major link.
 */
static inline void EDMA_TCDSetChannelMajorLink(DMA_Type * base, uint8_t channel, uint32_t majorLinkChannel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= (uint16_t)~(DMA_TCD_CSR_MAJORLINKCH_MASK);
    regValTemp |= (uint16_t)DMA_TCD_CSR_MAJORLINKCH(majorLinkChannel);
    base->TCD[channel].CSR = regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= (uint16_t)~(DMA_TCD_CSR_MAJORELINK_MASK);
    regValTemp |= (uint16_t)DMA_TCD_CSR_MAJORELINK(enable ? 1UL : 0UL);
    base->TCD[channel].CSR = regValTemp;
}

/*!
 * @brief Disables/Enables the DMA request after the major loop completes for the TCD.
 *
 * If disabled, the eDMA hardware automatically clears the corresponding DMA request when the
 * current major iteration count reaches zero.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param disable Disable (true)/Enable (false) DMA request after TCD complete.
 */
static inline void EDMA_TCDSetDisableDmaRequestAfterTCDDoneCmd(DMA_Type * base, uint8_t channel, bool disable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= (uint16_t)~(DMA_TCD_CSR_DREQ_MASK);
    regValTemp |= (uint16_t)DMA_TCD_CSR_DREQ(disable ? 1UL : 0UL);
    base->TCD[channel].CSR = regValTemp;
}

/*!
 * @brief Enables/Disables the half complete interrupt for the TCD.
 *
 * If set, the channel generates an interrupt request by setting the appropriate bit in the
 * interrupt register when the current major iteration count reaches the halfway point. Specifically,
 * the comparison performed by the eDMA engine is (CITER == (BITER >> 1)). This half-way point
 * interrupt request is provided to support the double-buffered schemes or other types of data movement
 * where the processor needs an early indication of the transfer's process.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param enable Enable (true) /Disable (false) half complete interrupt.
 */
static inline void EDMA_TCDSetMajorHalfCompleteIntCmd(DMA_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= (uint16_t)~(DMA_TCD_CSR_INTHALF_MASK);
    regValTemp |= (uint16_t)DMA_TCD_CSR_INTHALF(enable ? 1UL : 0UL);
    base->TCD[channel].CSR = regValTemp;
}

/*!
 * @brief Enables/Disables the interrupt after the major loop completes for the TCD.
 *
 * If enabled, the channel generates an interrupt request by setting the appropriate bit in the
 * interrupt register when the current major iteration count reaches zero.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param enable Enable (true) /Disable (false) interrupt after TCD done.
 */
static inline void EDMA_TCDSetMajorCompleteIntCmd(DMA_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMA_CHANNELS);
#endif
    uint16_t regValTemp;
    regValTemp = base->TCD[channel].CSR;
    regValTemp &= (uint16_t)~(DMA_TCD_CSR_INTMAJOR_MASK);
    regValTemp |= (uint16_t)DMA_TCD_CSR_INTMAJOR(enable ? 1UL : 0UL);
    base->TCD[channel].CSR = regValTemp;
}

/*!
 * @brief Sets the channel minor link for the TCD.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param linkChannel Channel to be linked on minor loop complete.
 * @param enable Enable (true)/Disable (false) channel minor link.
 */
void EDMA_TCDSetChannelMinorLink(DMA_Type * base, uint8_t channel, uint32_t linkChannel, bool enable);

/*!
 * @brief Sets the major iteration count according to minor loop channel link setting.
 *
 * Note here that user need to first set the minor loop channel link and then call this function.
 * The execute flow inside this function is dependent on the minor loop channel link setting.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @param count major loop count
 */
void EDMA_TCDSetMajorCount(DMA_Type * base, uint8_t channel, uint32_t count);

/*!
 * @brief Returns the current major iteration count.
 *
 * Gets the current major iteration count according to minor loop channel link settings.
 *
 * @param base Register base address for eDMA module.
 * @param channel eDMA channel number.
 * @return current iteration count
 */
uint32_t EDMA_TCDGetCurrentMajorCount(const DMA_Type * base, uint8_t channel);

#ifdef FEATURE_DMAMUX_AVAILABLE
/*!
 * @brief Initializes the DMAMUX module to the reset state.
 *
 * Initializes the DMAMUX module to the reset state.
 *
 * @param base Register base address for DMAMUX module.
 */
void DMAMUX_Init(DMAMUX_Type * base);
#endif

#ifdef FEATURE_DMAMUX_AVAILABLE
/*!
 * @brief Enables/Disables the DMAMUX channel.
 *
 * Enables the hardware request. If enabled, the hardware request is  sent to
 * the corresponding DMA channel.
 *
 * @param base Register base address for DMAMUX module.
 * @param channel DMAMUX channel number.
 * @param enable Enables (true) or Disables (false) DMAMUX channel.
 */
static inline void DMAMUX_SetChannelCmd(DMAMUX_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMAMUX_CHANNELS);
#endif
    uint32_t regIndex = FEATURE_DMAMUX_CHN_REG_INDEX(channel);
    uint8_t regValTemp = base->CHCFG[regIndex];
    regValTemp &= (uint8_t)~(DMAMUX_CHCFG_ENBL_MASK);
    regValTemp |= (uint8_t)DMAMUX_CHCFG_ENBL(enable ? 1U : 0U);
    base->CHCFG[regIndex] = regValTemp;
}
#endif

#ifdef FEATURE_DMAMUX_AVAILABLE
#ifdef FEATURE_DMAMUX_HAS_TRIG
/*!
 * @brief Configure DMA Channel Trigger bit in DMAMUX.
 *
 * Enables/Disables DMA Channel Trigger bit in DMAMUX.
 *
 * @param base Register base address for DMAMUX module.
 * @param channel DMAMUX channel number.
 * @param enable/disable command.
 */
static inline void DMAMUX_SetChannelTrigger(DMAMUX_Type * base, uint8_t channel, bool enable)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMAMUX_CHANNELS);
#endif
    uint32_t regIndex = FEATURE_DMAMUX_CHN_REG_INDEX(channel);
    uint8_t regValTemp;
    regValTemp = base->CHCFG[regIndex];
    regValTemp &= (uint8_t)~(DMAMUX_CHCFG_TRIG_MASK);
    regValTemp |= (uint8_t)DMAMUX_CHCFG_TRIG(enable ? 1U : 0U);
    base->CHCFG[regIndex] = regValTemp;
}
#endif
#endif

#ifdef FEATURE_DMAMUX_AVAILABLE
/*!
 * @brief Configures the DMA request for the DMAMUX channel.
 *
 * Selects which DMA source is routed to a DMA channel. The DMA sources are defined in the file
 * <MCU>_Features.h
 *
 * @param base Register base address for DMAMUX module.
 * @param channel DMAMUX channel number.
 * @param source DMA request source.
 */
static inline void DMAMUX_SetChannelSource(DMAMUX_Type * base, uint8_t channel, uint8_t source)
{
#if defined (CUSTOM_DEVASSERT) || defined (DEV_ERROR_DETECT)
    DEV_ASSERT(channel < FEATURE_DMAMUX_CHANNELS);
#endif
    uint32_t regIndex = FEATURE_DMAMUX_CHN_REG_INDEX(channel);
    uint8_t regValTemp;
    regValTemp = base->CHCFG[regIndex];
    regValTemp &= (uint8_t)~(DMAMUX_CHCFG_SOURCE_MASK);
    regValTemp |= (uint8_t)DMAMUX_CHCFG_SOURCE(source);
    base->CHCFG[regIndex] = regValTemp;
}
#endif

/*!
 * @brief Returns DMA Register Base Address.
 *
 * Gets the address of the selected DMA module.
 *
 * @param instance DMA instance to be returned.
 * @return DMA register base address
 */
DMA_Type * EDMA_DRV_GetDmaRegBaseAddr(uint32_t instance);

#if defined(__cplusplus)
}
#endif

#endif /* EDMA_HW_ACCESS_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/


