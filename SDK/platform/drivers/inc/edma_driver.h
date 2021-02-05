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
 * @file edma_driver.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
 * This is required to enable the use of a macro for computing TCD related values needed by
 * the user code (even if the macro is not used inside the EDMA driver code).
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.9, Function-like macro defined.
 * This is required to allow the use of a macro for computing TCD related values needed by
 * the user.
 */

#if !defined(EDMA_DRIVER_H)
#define EDMA_DRIVER_H

#include "device_registers.h"
#include "status.h"
#include <stddef.h>

/*!
 * @addtogroup edma_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Macro for the memory size needed for the software TCD.
 *
 * Software TCD is aligned to 32 bytes. We don't need a software TCD structure for the first
 * descriptor, since the configuration is pushed directly to registers.
 * To make sure the software TCD can meet the eDMA module requirement regarding alignment,
 * allocate memory for the remaining descriptors with extra 31 bytes.
 */
#define STCD_SIZE(number)           (((number) * 32U) - 1U)
#define STCD_ADDR(address)          (((uint32_t)address + 31UL) & ~0x1FUL)

/*!
 * @brief Macro for accessing the least significant bit of the ERR register.
 *
 * The erroneous channels are retrieved from ERR register by subsequently right
 * shifting all the ERR bits + "AND"-ing the result with this mask.
 */
#define EDMA_ERR_LSB_MASK           1U

/*! @brief eDMA channel interrupts.
 * Implements : edma_channel_interrupt_t_Class
 */
typedef enum {
    EDMA_CHN_ERR_INT = 0U,         /*!< Error interrupt */
    EDMA_CHN_HALF_MAJOR_LOOP_INT,  /*!< Half major loop interrupt. */
    EDMA_CHN_MAJOR_LOOP_INT        /*!< Complete major loop interrupt. */
} edma_channel_interrupt_t;

/*! @brief eDMA channel arbitration algorithm used for selection among channels.
 * Implements : edma_arbitration_algorithm_t_Class
 */
typedef enum {
    EDMA_ARBITRATION_FIXED_PRIORITY = 0U,  /*!< Fixed Priority */
    EDMA_ARBITRATION_ROUND_ROBIN           /*!< Round-Robin arbitration */
} edma_arbitration_algorithm_t;

/*! @brief eDMA channel priority setting
 * Implements : edma_channel_priority_t_Class
 */
typedef enum {
    EDMA_CHN_PRIORITY_0 = 0U,
    EDMA_CHN_PRIORITY_1 = 1U,
    EDMA_CHN_PRIORITY_2 = 2U,
    EDMA_CHN_PRIORITY_3 = 3U,
#ifndef FEATURE_DMA_4_CH_PRIORITIES    
    EDMA_CHN_PRIORITY_4 = 4U,
    EDMA_CHN_PRIORITY_5 = 5U,
    EDMA_CHN_PRIORITY_6 = 6U,
    EDMA_CHN_PRIORITY_7 = 7U,
#ifndef FEATURE_DMA_8_CH_PRIORITIES
    EDMA_CHN_PRIORITY_8 = 8U,
    EDMA_CHN_PRIORITY_9 = 9U,
    EDMA_CHN_PRIORITY_10 = 10U,
    EDMA_CHN_PRIORITY_11 = 11U,
    EDMA_CHN_PRIORITY_12 = 12U,
    EDMA_CHN_PRIORITY_13 = 13U,
    EDMA_CHN_PRIORITY_14 = 14U,
    EDMA_CHN_PRIORITY_15 = 15U,
#endif
#endif
    EDMA_CHN_DEFAULT_PRIORITY = 255U
} edma_channel_priority_t;

#if FEATURE_DMA_CHANNEL_GROUP_COUNT > 0x1U
/*! @brief eDMA group priority setting
 * Implements : edma_group_priority_t_Class
 */
typedef enum {
#ifdef FEATURE_DMA_HWV3
    EDMA_CHN_GROUP_0 = 0U,
    EDMA_CHN_GROUP_1 = 1U,
    EDMA_CHN_GROUP_2 = 2U,
    EDMA_CHN_GROUP_3 = 3U,
    EDMA_CHN_GROUP_4 = 4U,
    EDMA_CHN_GROUP_5 = 5U,
    EDMA_CHN_GROUP_6 = 6U,
    EDMA_CHN_GROUP_7 = 7U,
    EDMA_CHN_GROUP_8 = 8U,
    EDMA_CHN_GROUP_9 = 9U,
    EDMA_CHN_GROUP_10 = 10U,
    EDMA_CHN_GROUP_11 = 11U,
    EDMA_CHN_GROUP_12 = 12U,
    EDMA_CHN_GROUP_13 = 13U,
    EDMA_CHN_GROUP_14 = 14U,
    EDMA_CHN_GROUP_15 = 15U,
    EDMA_CHN_GROUP_16 = 16U,
    EDMA_CHN_GROUP_17 = 17U,
    EDMA_CHN_GROUP_18 = 18U,
    EDMA_CHN_GROUP_19 = 19U,
    EDMA_CHN_GROUP_20 = 20U,
    EDMA_CHN_GROUP_21 = 21U,
    EDMA_CHN_GROUP_22 = 22U,
    EDMA_CHN_GROUP_23 = 23U,
    EDMA_CHN_GROUP_24 = 24U,
    EDMA_CHN_GROUP_25 = 25U,
    EDMA_CHN_GROUP_26 = 26U,
    EDMA_CHN_GROUP_27 = 27U,
    EDMA_CHN_GROUP_28 = 28U,
    EDMA_CHN_GROUP_29 = 29U,
    EDMA_CHN_GROUP_30 = 30U,
    EDMA_CHN_GROUP_31 = 31U
#else
    EDMA_GRP0_PRIO_LOW_GRP1_PRIO_HIGH = 0U,
    EDMA_GRP0_PRIO_HIGH_GRP1_PRIO_LOW = 1U
#endif
} edma_group_priority_t;
#endif

/*! @brief eDMA modulo configuration
 * Implements : edma_modulo_t_Class
 */
typedef enum {
    EDMA_MODULO_OFF = 0U,
    EDMA_MODULO_2B,
    EDMA_MODULO_4B,
    EDMA_MODULO_8B,
    EDMA_MODULO_16B,
    EDMA_MODULO_32B,
    EDMA_MODULO_64B,
    EDMA_MODULO_128B,
    EDMA_MODULO_256B,
    EDMA_MODULO_512B,
    EDMA_MODULO_1KB,
    EDMA_MODULO_2KB,
    EDMA_MODULO_4KB,
    EDMA_MODULO_8KB,
    EDMA_MODULO_16KB,
    EDMA_MODULO_32KB,
    EDMA_MODULO_64KB,
    EDMA_MODULO_128KB,
    EDMA_MODULO_256KB,
    EDMA_MODULO_512KB,
    EDMA_MODULO_1MB,
    EDMA_MODULO_2MB,
    EDMA_MODULO_4MB,
    EDMA_MODULO_8MB,
    EDMA_MODULO_16MB,
    EDMA_MODULO_32MB,
    EDMA_MODULO_64MB,
    EDMA_MODULO_128MB,
    EDMA_MODULO_256MB,
    EDMA_MODULO_512MB,
    EDMA_MODULO_1GB,
    EDMA_MODULO_2GB
} edma_modulo_t;

/*! @brief eDMA transfer configuration
 * Implements : edma_transfer_size_t_Class
 */
typedef enum {
#ifdef FEATURE_DMA_HWV3
    EDMA_TRANSFER_SIZE_1B  = 0x0U,
    EDMA_TRANSFER_SIZE_2B  = 0x1U,
    EDMA_TRANSFER_SIZE_4B  = 0x2U,
    EDMA_TRANSFER_SIZE_8B  = 0x3U,
    EDMA_TRANSFER_SIZE_16B = 0x4U,
    EDMA_TRANSFER_SIZE_32B = 0x5U,
    EDMA_TRANSFER_SIZE_64B = 0x6U
#else
    EDMA_TRANSFER_SIZE_1B  = 0x0U,
    EDMA_TRANSFER_SIZE_2B  = 0x1U,
    EDMA_TRANSFER_SIZE_4B  = 0x2U,
#ifdef FEATURE_DMA_TRANSFER_SIZE_8B
    EDMA_TRANSFER_SIZE_8B  = 0x3U,
#endif
#ifdef FEATURE_DMA_TRANSFER_SIZE_16B 
    EDMA_TRANSFER_SIZE_16B = 0x4U,
#endif
#ifdef FEATURE_DMA_TRANSFER_SIZE_32B
    EDMA_TRANSFER_SIZE_32B = 0x5U,
#endif    
#ifdef FEATURE_DMA_TRANSFER_SIZE_64B
    EDMA_TRANSFER_SIZE_64B = 0x6U
#endif
#endif
} edma_transfer_size_t;

/*!
 * @brief The user configuration structure for the eDMA driver.
 *
 * Use an instance of this structure with the EDMA_DRV_Init() function. This allows the user to configure
 * settings of the EDMA peripheral with a single function call.
 * Implements : edma_user_config_t_Class
 */
typedef struct {
    edma_arbitration_algorithm_t chnArbitration;         /*!< eDMA channel arbitration. */
#if FEATURE_DMA_CHANNEL_GROUP_COUNT > 0x1U
#ifndef FEATURE_DMA_HWV3
    edma_arbitration_algorithm_t groupArbitration;       /*!< eDMA group arbitration. */
    edma_group_priority_t groupPriority;                 /*!< eDMA group priority. Used while eDMA
                                                              group arbitration is set to fixed priority. */
#endif
#endif
    bool haltOnError;                                    /*!< Any error causes the HALT bit to set. Subsequently, all
                                                              service requests are ignored until the HALT bit is cleared. */													  
} edma_user_config_t;

/*!
 * @brief Channel status for eDMA channel.
 *
 * A structure describing the eDMA channel status. The user can get the status by callback parameter
 * or by calling EDMA_DRV_getStatus() function.
 * Implements : edma_chn_status_t_Class
 */
typedef enum {
    EDMA_CHN_NORMAL = 0U,           /*!< eDMA channel normal state. */
    EDMA_CHN_ERROR                  /*!< An error occurred in the eDMA channel. */
} edma_chn_status_t;

/*!
 * @brief Definition for the eDMA channel callback function.
 *
 * Prototype for the callback function registered in the eDMA driver.
 * Implements : edma_callback_t_Class
 */
typedef void (*edma_callback_t)(void *parameter, edma_chn_status_t status);

/*! @brief Data structure for the eDMA channel state.
 * Implements : edma_chn_state_t_Class
 */
typedef struct {
    uint8_t virtChn;                     /*!< Virtual channel number. */
    edma_callback_t callback;            /*!< Callback function pointer for the eDMA channel. It will
                                              be called at the eDMA channel complete and eDMA channel
                                              error. */
    void *parameter;                     /*!< Parameter for the callback function pointer. */
    volatile edma_chn_status_t status;   /*!< eDMA channel status. */
} edma_chn_state_t;

/*!
 * @brief The user configuration structure for the an eDMA driver channel.
 *
 * Use an instance of this structure with the EDMA_DRV_ChannelInit() function. This allows the user to configure
 * settings of the EDMA channel with a single function call.
 * Implements : edma_channel_config_t_Class
 */
typedef struct {
#if FEATURE_DMA_CHANNEL_GROUP_COUNT > 0x1U
#ifdef FEATURE_DMA_HWV3
    edma_group_priority_t groupPriority;     /*!< eDMA group priority. Used while eDMA
                                              group arbitration is set to fixed priority. */
#endif
#endif
    edma_channel_priority_t channelPriority; /*!< eDMA channel priority - only used when channel
                                                  arbitration mode is 'Fixed priority'. */
    uint8_t virtChnConfig;                   /*!< eDMA virtual channel number */
#ifdef FEATURE_DMAMUX_AVAILABLE    
    dma_request_source_t source;             /*!< Selects the source of the DMA request for this channel */
#endif    
    edma_callback_t callback;                /*!< Callback that will be registered for this channel */
    void * callbackParam;                    /*!< Parameter passed to the channel callback */
    bool enableTrigger;                      /*!< Enables the periodic trigger capability for the DMA channel. */			
} edma_channel_config_t;

/*! @brief A type for the DMA transfer.
 * Implements : edma_transfer_type_t_Class
 */
typedef enum {
    EDMA_TRANSFER_PERIPH2MEM = 0U,   /*!< Transfer from peripheral to memory */
    EDMA_TRANSFER_MEM2PERIPH,        /*!< Transfer from memory to peripheral */
    EDMA_TRANSFER_MEM2MEM,           /*!< Transfer from memory to memory */
    EDMA_TRANSFER_PERIPH2PERIPH      /*!< Transfer from peripheral to peripheral */
} edma_transfer_type_t;

/*! @brief Data structure for configuring a discrete memory transfer.
 * Implements : edma_scatter_gather_list_t_Class
 */
typedef struct {
    uint32_t address;           /*!< Address of buffer. */
    uint32_t length;            /*!< Length of buffer. */
    edma_transfer_type_t type;  /*!< Type of the DMA transfer */
} edma_scatter_gather_list_t;

/*!
 * @brief Runtime state structure for the eDMA driver.
 *
 * This structure holds data that is used by the eDMA peripheral driver to manage
 * multi eDMA channels.
 * The user passes the memory for this run-time state structure and the eDMA
 * driver populates the members.
 * Implements : edma_state_t_Class
 */
typedef struct {
    edma_chn_state_t * volatile virtChnState[(uint32_t)FEATURE_DMA_VIRTUAL_CHANNELS];   /*!< Pointer array storing channel state. */
} edma_state_t;

/*!
 * @brief eDMA loop transfer configuration.
 *
 * This structure configures the basic minor/major loop attributes.
 * Implements : edma_loop_transfer_config_t_Class
 */
typedef struct {
    uint32_t majorLoopIterationCount;       /*!< Number of major loop iterations. */
    bool srcOffsetEnable;                   /*!< Selects whether the minor loop offset is applied to the
                                                 source address upon minor loop completion. */
    bool dstOffsetEnable;                   /*!< Selects whether the minor loop offset is applied to the
                                                 destination address upon minor loop completion. */
    int32_t minorLoopOffset;                /*!< Sign-extended offset applied to the source or destination address
                                                 to form the next-state value after the minor loop completes. */
    bool minorLoopChnLinkEnable;            /*!< Enables channel-to-channel linking on minor loop complete. */
    uint8_t minorLoopChnLinkNumber;         /*!< The number of the next channel to be started by DMA
                                                 engine when minor loop completes. */
    bool majorLoopChnLinkEnable;            /*!< Enables channel-to-channel linking on major loop complete. */
    uint8_t majorLoopChnLinkNumber;         /*!< The number of the next channel to be started by DMA
                                                 engine when major loop completes. */
} edma_loop_transfer_config_t;

/*!
 * @brief eDMA transfer size configuration.
 *
 * This structure configures the basic source/destination transfer attribute.
 * Implements : edma_transfer_config_t_Class
 */
typedef struct {
    uint32_t srcAddr;                                 /*!< Memory address pointing to the source data. */
    uint32_t destAddr;                                /*!< Memory address pointing to the destination data. */
    edma_transfer_size_t srcTransferSize;             /*!< Source data transfer size. */
    edma_transfer_size_t destTransferSize;            /*!< Destination data transfer size. */
    int16_t srcOffset;                                /*!< Sign-extended offset applied to the current source address
                                                           to form the next-state value as each source read/write
                                                           is completed. */
    int16_t destOffset;                               /*!< Sign-extended offset applied to the current destination
                                                           address to form the next-state value as each source
                                                           read/write is completed. */
    int32_t srcLastAddrAdjust;                        /*!< Last source address adjustment. */
    int32_t destLastAddrAdjust;                       /*!< Last destination address adjustment. Note here it is only
                                                           valid when scatter/gather feature is not enabled. */
    edma_modulo_t srcModulo;                          /*!< Source address modulo. */
    edma_modulo_t destModulo;                         /*!< Destination address modulo. */
    uint32_t minorByteTransferCount;                  /*!< Number of bytes to be transferred in each service request
                                                           of the channel. */
    bool scatterGatherEnable;                         /*!< Enable scatter gather feature. */
    uint32_t scatterGatherNextDescAddr;               /*!< The address of the next descriptor to be used, when
                                                           scatter/gather feature is enabled.
                                                           Note: this value is not used when scatter/gather
                                                                 feature is disabled. */
    bool interruptEnable;                             /*!< Enable the interrupt request when the major loop
                                                           count completes */
    edma_loop_transfer_config_t *loopTransferConfig;  /*!< Pointer to loop transfer configuration structure
                                                           (defines minor/major loop attributes)
                                                           Note: this field is only used when minor loop mapping is
                                                                 enabled from DMA configuration. */
} edma_transfer_config_t;

#if (defined(CORE_LITTLE_ENDIAN))
/*! @brief eDMA TCD
 * Implements : edma_software_tcd_t_Class
 */
typedef struct {
    uint32_t SADDR;
    int16_t SOFF;
    uint16_t ATTR;
    uint32_t NBYTES;
    int32_t SLAST;
    uint32_t DADDR;
    int16_t DOFF;
    uint16_t CITER;
    int32_t DLAST_SGA;
    uint16_t CSR;
    uint16_t BITER;
} edma_software_tcd_t;
#elif (defined(CORE_BIG_ENDIAN))
/*! @brief eDMA TCD
 * Implements : edma_software_tcd_t_Class
 */
typedef struct {
    uint32_t SADDR;
    uint16_t ATTR;
    int16_t SOFF;
    uint32_t NBYTES;
    int32_t SLAST;
    uint32_t DADDR;
    uint16_t CITER;
    int16_t DOFF;
    int32_t DLAST_SGA;
    uint16_t BITER;
    uint16_t CSR;
} edma_software_tcd_t;
#else
    #error "Endianness not defined!"
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
  * @name eDMA Peripheral Driver
  * @{
  */

/*!
  * @name eDMA peripheral driver module level functions
  * @{
  */

/*!
 * @brief Initializes the eDMA module.
 *
 * This function initializes the run-time state structure to provide the eDMA channel allocation
 * release, protect, and track the state for channels. This function also resets the eDMA modules,
 * initializes the module to user-defined settings and default settings.
 * @param edmaState The pointer to the eDMA peripheral driver state structure. The user passes
 * the memory for this run-time state structure and the eDMA peripheral driver populates the
 * members. This run-time state structure keeps track of the eDMA channels status. The memory must
 * be kept valid before calling the EDMA_DRV_DeInit.
 * @param userConfig User configuration structure for eDMA peripheral drivers. The user populates the
 * members of this structure and passes the pointer of this structure into the function.
 * @param chnStateArray Array of pointers to run-time state structures for eDMA channels;
 * will populate the state structures inside the eDMA driver state structure.
 * @param chnConfigArray Array of pointers to channel initialization structures.
 * @param chnCount The number of eDMA channels to be initialized.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_Init(edma_state_t * edmaState,
                       const edma_user_config_t *userConfig,
                       edma_chn_state_t * const chnStateArray[],
                       const edma_channel_config_t * const chnConfigArray[],
                       uint32_t chnCount);

/*!
 * @brief De-initializes the eDMA module.
 *
 * This function resets the eDMA module to reset state and disables the interrupt to the core.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_Deinit(void);

/*! @} */

/*!
  * @name eDMA peripheral driver channel management functions
  * @{
  */

/*!
 * @brief Initializes an eDMA channel.
 *
 * This function initializes the run-time state structure for a eDMA channel, based on user
 * configuration. It will request the channel, set up the channel priority and install the
 * callback.
 *
 * @param edmaChannelState Pointer to the eDMA channel state structure. The user passes
 * the memory for this run-time state structure and the eDMA peripheral driver populates the
 * members. This run-time state structure keeps track of the eDMA channel status. The memory must
 * be kept valid before calling the EDMA_DRV_ReleaseChannel.
 * @param edmaChannelConfig User configuration structure for eDMA channel. The user populates the
 * members of this structure and passes the pointer of this structure into the function.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_ChannelInit(edma_chn_state_t *edmaChannelState,
                              const edma_channel_config_t *edmaChannelConfig);

/*!
 * @brief Releases an eDMA channel.
 *
 * This function stops the eDMA channel and disables the interrupt of this channel. The channel state
 * structure can be released after this function is called.
 *
 * @param virtualChannel eDMA virtual channel number.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_ReleaseChannel(uint8_t virtualChannel);

/*! @} */

/*!
  * @name eDMA peripheral driver transfer setup functions
  * @{
  */

/*!
 * @brief Copies the channel configuration to the TCD registers.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param tcd Pointer to the channel configuration structure.
 */
void EDMA_DRV_PushConfigToReg(uint8_t virtualChannel,
                              const edma_transfer_config_t *tcd);

/*!
 * @brief Copies the channel configuration to the software TCD structure.
 *
 * This function copies the properties from the channel configuration to the software TCD structure; the address
 * of the software TCD can be used to enable scatter/gather operation (pointer to the next TCD).

 * @param config Pointer to the channel configuration structure.
 * @param stcd Pointer to the software TCD structure.
 */
void EDMA_DRV_PushConfigToSTCD(const edma_transfer_config_t *config,
                               edma_software_tcd_t *stcd);

/*!
 * @brief Configures a simple single block data transfer with DMA.
 *
 * This function configures the descriptor for a single block transfer.
 * The function considers contiguous memory blocks, thus it configures the TCD
 * source/destination offset fields to cover the data buffer without gaps,
 * according to "transferSize" parameter (the offset is equal to the number of
 * bytes transferred in a source read/destination write).
 *
 * NOTE: For memory-to-peripheral or peripheral-to-memory transfers, make sure
 * the transfer size is equal to the data buffer size of the peripheral used,
 * otherwise only truncated chunks of data may be transferred (e.g. for a
 * communication IP with an 8-bit data register the transfer size should be 1B,
 * whereas for a 32-bit data register, the transfer size should be 4B). The
 * rationale of this constraint is that, on the peripheral side, the address
 * offset is set to zero, allowing to read/write data from/to the peripheral
 * in a single source read/destination write operation.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param type Transfer type (M->M, P->M, M->P, P->P).
 * @param srcAddr A source register address or a source memory address.
 * @param destAddr A destination register address or a destination memory address.
 * @param transferSize The number of bytes to be transferred on every DMA write/read.
 *        Source/Dest share the same write/read size.
 * @param dataBufferSize The total number of bytes to be transferred.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS
 */
status_t EDMA_DRV_ConfigSingleBlockTransfer(uint8_t virtualChannel,
                                            edma_transfer_type_t type,
                                            uint32_t srcAddr,
                                            uint32_t destAddr,
                                            edma_transfer_size_t transferSize,
                                            uint32_t dataBufferSize);

/*!
 * @brief Configures a multiple block data transfer with DMA.
 *
 * This function configures the descriptor for a multi-block transfer.
 * The function considers contiguous memory blocks, thus it configures the TCD
 * source/destination offset fields to cover the data buffer without gaps,
 * according to "transferSize" parameter (the offset is equal to the number of
 * bytes transferred in a source read/destination write). The buffer is divided
 * in multiple block, each block being transferred upon a single DMA request.
 *
 * NOTE: For transfers to/from peripherals, make sure
 * the transfer size is equal to the data buffer size of the peripheral used,
 * otherwise only truncated chunks of data may be transferred (e.g. for a
 * communication IP with an 8-bit data register the transfer size should be 1B,
 * whereas for a 32-bit data register, the transfer size should be 4B). The
 * rationale of this constraint is that, on the peripheral side, the address
 * offset is set to zero, allowing to read/write data from/to the peripheral
 * in a single source read/destination write operation.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param type Transfer type (M->M, P->M, M->P, P->P).
 * @param srcAddr A source register address or a source memory address.
 * @param destAddr A destination register address or a destination memory address.
 * @param transferSize The number of bytes to be transferred on every DMA write/read.
 *        Source/Dest share the same write/read size.
 * @param blockSize The total number of bytes inside a block.
 * @param blockCount The total number of data blocks (one block is transferred upon a DMA request).
 * @param disableReqOnCompletion This parameter specifies whether the DMA channel should
 *        be disabled when the transfer is complete (further requests will remain untreated).
 *
 * @return STATUS_ERROR or STATUS_SUCCESS
 */
status_t EDMA_DRV_ConfigMultiBlockTransfer(uint8_t virtualChannel,
                                           edma_transfer_type_t type,
                                           uint32_t srcAddr,
                                           uint32_t destAddr,
                                           edma_transfer_size_t transferSize,
                                           uint32_t blockSize,
                                           uint32_t blockCount,
                                           bool disableReqOnCompletion);

/*!
 * @brief Configures the DMA transfer in loop mode.
 *
 * This function configures the DMA transfer in a loop chain. The user passes a block of memory into this
 * function that configures the loop transfer properties (minor/major loop count, address offsets, channel linking).
 * The DMA driver copies the configuration to TCD registers, only when the loop properties are set up correctly
 * and minor loop mapping is enabled for the eDMA module.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param transferConfig Pointer to the transfer configuration strucutre; this structure defines fields for setting
 * up the basic transfer and also a pointer to a memory strucure that defines the loop chain properties (minor/major).
 *
 * @return STATUS_ERROR or STATUS_SUCCESS
 */
status_t EDMA_DRV_ConfigLoopTransfer(uint8_t virtualChannel,
                                     const edma_transfer_config_t *transferConfig);

/*!
 * @brief Configures the DMA transfer in a scatter-gather mode.
 *
 * This function configures the descriptors into a single-ended chain. The user passes blocks of memory into
 * this function. The interrupt is triggered only when the last memory block is completed. The memory block
 * information is passed with the edma_scatter_gather_list_t data structure, which can tell
 * the memory address and length.
 * The DMA driver configures the descriptor for each memory block, transfers the descriptor from the
 * first one to the last one, and stops.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param stcd Array of empty software TCD structures. The user must prepare this memory block. We don't need a
 * software TCD structure for the first descriptor, since the configuration is pushed directly to registers.The "stcd"
 * buffer must align with 32 bytes; if not, an error occurs in the eDMA driver. Thus, the required
 * memory size for "stcd" is equal to tcdCount * size_of(edma_software_tcd_t) - 1; the driver will take
 * care of the memory alignment if the provided memory buffer is big enough. For proper allocation of the
 * "stcd" buffer it is recommended to use STCD_SIZE macro.
 * @param transferSize The number of bytes to be transferred on every DMA write/read.
 * @param bytesOnEachRequest Bytes to be transferred in each DMA request.
 * @param srcList Data structure storing the address, length and type of transfer (M->M, M->P, P->M, P->P) for
 * the bytes to be transferred for source memory blocks. If the source memory is peripheral, the length
 * is not used.
 * @param destList Data structure storing the address, length and type of transfer (M->M, M->P, P->M, P->P) for
 * the bytes to be transferred for destination memory blocks. In the memory-to-memory transfer mode, the
 * user must ensure that the length of the destination scatter gather list is equal to the source
 * scatter gather list. If the destination memory is a peripheral register, the length is not used.
 * @param tcdCount The number of TCD memory blocks contained in the scatter gather list.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS
 */
status_t EDMA_DRV_ConfigScatterGatherTransfer(uint8_t virtualChannel,
                                              edma_software_tcd_t *stcd,
                                              edma_transfer_size_t transferSize,
                                              uint32_t bytesOnEachRequest,
                                              const edma_scatter_gather_list_t *srcList,
                                              const edma_scatter_gather_list_t *destList,
                                              uint8_t tcdCount);

/*!
 * @brief Cancel the running transfer.
 *
 * This function cancels the current transfer, optionally signalling an error.
 *
 * @param bool error If true, an error will be logged for the current transfer.
 */
void EDMA_DRV_CancelTransfer(bool error);

/*! @} */

/*!
  * @name eDMA Peripheral driver channel operation functions
  * @{
  */
/*!
 * @brief Starts an eDMA channel.
 *
 * This function enables the eDMA channel DMA request.
 *
 * @param virtualChannel eDMA virtual channel number.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_StartChannel(uint8_t virtualChannel);

/*!
 * @brief Stops the eDMA channel.
 *
 * This function disables the eDMA channel DMA request.
 *
 * @param virtualChannel eDMA virtual channel number.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_StopChannel(uint8_t virtualChannel);

/*!
 * @brief Configures the DMA request for the eDMA channel.
 *
 * Selects which DMA source is routed to a DMA channel. The DMA sources are defined in the file
 * <MCU>_Features.h
 * Configures the periodic trigger capability for the triggered DMA channel.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param request DMA request source.
 * @param enableTrigger DMA channel periodic trigger.
 *
 * @return STATUS_SUCCESS or STATUS_UNSUPPORTED.
 */
status_t EDMA_DRV_SetChannelRequestAndTrigger(uint8_t virtualChannel,
                                              uint8_t request,
                                              bool enableTrigger);

/*!
 * @brief Clears all registers to 0 for the channel's TCD.
 *
 * @param virtualChannel eDMA virtual channel number.
 */
void EDMA_DRV_ClearTCD(uint8_t virtualChannel);

/*!
 * @brief Configures the source address for the eDMA channel.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param address The pointer to the source memory address.
 */
void EDMA_DRV_SetSrcAddr(uint8_t virtualChannel,
                         uint32_t address);

/*!
 * @brief Configures the source address signed offset for the eDMA channel.
 *
 * Sign-extended offset applied to the current source address to form the next-state value as each
 * source read is complete.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param offset Signed-offset for source address.
 */
void EDMA_DRV_SetSrcOffset(uint8_t virtualChannel,
                           int16_t offset);

/*!
 * @brief Configures the source data chunk size (transferred in a read sequence).
 *
 * Source data read transfer size (1/2/4/16/32 bytes).
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param size Source transfer size.
 */
void EDMA_DRV_SetSrcReadChunkSize(uint8_t virtualChannel,
                                  edma_transfer_size_t size);

/*!
 * @brief Configures the source address last adjustment.
 *
 * Adjustment value added to the source address at the completion of the major iteration count. This
 * value can be applied to restore the source address to the initial value, or adjust the address to
 * reference the next data structure.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param adjust Adjustment value.
 */
void EDMA_DRV_SetSrcLastAddrAdjustment(uint8_t virtualChannel,
                                       int32_t adjust);

/*!
 * @brief Configures the destination address for the eDMA channel.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param address The pointer to the destination memory address.
 */
void EDMA_DRV_SetDestAddr(uint8_t virtualChannel,
                          uint32_t address);

/*!
 * @brief Configures the destination address signed offset for the eDMA channel.
 *
 * Sign-extended offset applied to the current destination address to form the next-state value as each
 * destination write is complete.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param offset signed-offset
 */
void EDMA_DRV_SetDestOffset(uint8_t virtualChannel,
                            int16_t offset);

/*!
 * @brief Configures the destination data chunk size (transferred in a write sequence).
 *
 * Destination data write transfer size (1/2/4/16/32 bytes).
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param size Destination transfer size.
 */
void EDMA_DRV_SetDestWriteChunkSize(uint8_t virtualChannel,
                                    edma_transfer_size_t size);

/*!
 * @brief Configures the destination address last adjustment.
 *
 * Adjustment value added to the destination address at the completion of the major iteration count. This
 * value can be applied to restore the destination address to the initial value, or adjust the address to
 * reference the next data structure.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param adjust Adjustment value.
 */
void EDMA_DRV_SetDestLastAddrAdjustment(uint8_t virtualChannel,
                                        int32_t adjust);

/*!
 * @brief Configures the number of bytes to be transferred in each service request of the channel.
 *
 * Sets the number of bytes to be transferred each time a request is received (one major loop iteration).
 * This number needs to be a multiple of the source/destination transfer size, as the data block will be
 * transferred within multiple read/write sequences (minor loops).
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param nbytes Number of bytes to be transferred in each service request of the channel
 */
void EDMA_DRV_SetMinorLoopBlockSize(uint8_t virtualChannel,
                                    uint32_t nbytes);

/*!
 * @brief Configures the number of major loop iterations.
 *
 * Sets the number of major loop iterations; each major loop iteration will be served upon a request
 * for the current channel, transferring the data block configured for the minor loop (NBYTES).
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param majorLoopCount Number of major loop iterations.
 */
void EDMA_DRV_SetMajorLoopIterationCount(uint8_t virtualChannel,
                                         uint32_t majorLoopCount);

/*!
 * @brief Returns the remaining major loop iteration count.
 *
 * Gets the number minor loops yet to be triggered (major loop iterations).
 *
 * @param virtualChannel eDMA virtual channel number.
 * @return number of major loop iterations yet to be triggered
 */
uint32_t EDMA_DRV_GetRemainingMajorIterationsCount(uint8_t virtualChannel);

/*!
 * @brief Configures the memory address of the next TCD, in scatter/gather mode.
 *
 * This function configures the address of the next TCD to be loaded form memory, when scatter/gather
 * feature is enabled. This address points to the beginning of a 0-modulo-32 byte region containing
 * the next transfer TCD to be loaded into this channel. The channel reload is performed as the
 * major iteration count completes. The scatter/gather address must be 0-modulo-32-byte. Otherwise,
 * a configuration error is reported.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param nextTCDAddr The address of the next TCD to be linked to this TCD.
 */
void EDMA_DRV_SetScatterGatherLink(uint8_t virtualChannel,
                                   uint32_t nextTCDAddr);

/*!
 * @brief Disables/Enables the DMA request after the major loop completes for the TCD.
 *
 * If disabled, the eDMA hardware automatically clears the corresponding DMA request when the
 * current major iteration count reaches zero.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param disable Disable (true)/Enable (false) DMA request after TCD complete.
 */
void EDMA_DRV_DisableRequestsOnTransferComplete(uint8_t virtualChannel,
                                                bool disable);

/*!
 * @brief Disables/Enables the channel interrupt requests.
 *
 * This function enables/disables error, half major loop and complete major loop interrupts
 * for the current channel.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param interrupt Interrupt event (error/half major loop/complete major loop).
 * @param enable Enable (true)/Disable (false) interrupts for the current channel.
 */
void EDMA_DRV_ConfigureInterrupt(uint8_t virtualChannel,
                                 edma_channel_interrupt_t intSrc,
                                 bool enable);

/*!
 * @brief Triggers a sw request for the current channel.
 *
 * This function starts a transfer using the current channel (sw request).
 *
 * @param virtualChannel eDMA virtual channel number.
 */
void EDMA_DRV_TriggerSwRequest(uint8_t virtualChannel);

/*! @} */

/*!
  * @name eDMA Peripheral callback and interrupt functions
  * @{
  */

/*!
 * @brief Registers the callback function and the parameter for eDMA channel.
 *
 * This function registers the callback function and the parameter into the eDMA channel state structure.
 * The callback function is called when the channel is complete or a channel error occurs. The eDMA
 * driver passes the channel status to this callback function to indicate whether it is caused by the
 * channel complete event or the channel error event.
 *
 * To un-register the callback function, set the callback function to "NULL" and call this
 * function.
 *
 * @param virtualChannel eDMA virtual channel number.
 * @param callback The pointer to the callback function.
 * @param parameter The pointer to the callback function's parameter.
 *
 * @return STATUS_ERROR or STATUS_SUCCESS.
 */
status_t EDMA_DRV_InstallCallback(uint8_t virtualChannel,
                                  edma_callback_t callback,
                                  void *parameter);

/*! @} */

/*!
  * @name eDMA Peripheral driver miscellaneous functions
  * @{
  */
/*!
 * @brief Gets the eDMA channel status.
 *
 * @param virtualChannel eDMA virtual channel number.
 *
 * @return Channel status.
 */
edma_chn_status_t EDMA_DRV_GetChannelStatus(uint8_t virtualChannel);

/*! @} */

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* EDMA_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/

