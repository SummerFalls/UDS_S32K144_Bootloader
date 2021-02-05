/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
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

 /*!
 * @file lpuart_hw_access.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.3, Global typedef not referenced.
 * This increases ease of use: allows users to access the corresponding field in the register
 * using an already defined type.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
 * The macros were defined for consistency reasons, all the registers have a corresponding ID.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, Expression assigned to a narrower or different essential type.
 * The cast is required to perform a conversion between an unsigned integer and an enum type.
 */

#ifndef LPUART_HW_ACCESS_H__
#define LPUART_HW_ACCESS_H__

#include "lpuart_driver.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPUART_SHIFT (16U)
#define LPUART_BAUD_REG_ID (1U)
#define LPUART_STAT_REG_ID (2U)
#define LPUART_CTRL_REG_ID (3U)
#define LPUART_DATA_REG_ID (4U)
#define LPUART_MATCH_REG_ID (5U)
#define LPUART_MODIR_REG_ID (6U)
#define LPUART_FIFO_REG_ID (7U)

/*! @brief LPUART wakeup from standby method constants
 */
typedef enum
{
    LPUART_IDLE_LINE_WAKE = 0x0U, /*!< Idle-line wakes the LPUART receiver from standby. */
    LPUART_ADDR_MARK_WAKE = 0x1U  /*!< Addr-mark wakes LPUART receiver from standby.*/
} lpuart_wakeup_method_t;

/*!
 * @brief LPUART break character length settings for transmit/detect.
 *
 * The actual maximum bit times may vary depending on the LPUART instance.
 */
typedef enum
{
    LPUART_BREAK_CHAR_10_BIT_MINIMUM = 0x0U, /*!< LPUART break char length 10 bit times (if M = 0, SBNS = 0)
                                                  or 11 (if M = 1, SBNS = 0 or M = 0, SBNS = 1) or 12 (if M = 1,
                                                  SBNS = 1 or M10 = 1, SNBS = 0) or 13 (if M10 = 1, SNBS = 1) */
    LPUART_BREAK_CHAR_13_BIT_MINIMUM = 0x1U  /*!< LPUART break char length 13 bit times (if M = 0, SBNS = 0
                                                  or M10 = 0, SBNS = 1) or 14 (if M = 1, SBNS = 0 or M = 1,
                                                  SBNS = 1) or 15 (if M10 = 1, SBNS = 1 or M10 = 1, SNBS = 0) */
} lpuart_break_char_length_t;

/*!
 * @brief LPUART status flags.
 *
 * This provides constants for the LPUART status flags for use in the UART functions.
 */
typedef enum
{
    LPUART_TX_DATA_REG_EMPTY          = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_TDRE_SHIFT), \
                                        /*!< Tx data register empty flag, sets when Tx buffer is empty */
    LPUART_TX_COMPLETE                = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_TC_SHIFT), \
                                        /*!< Transmission complete flag, sets when transmission activity complete */
    LPUART_RX_DATA_REG_FULL           = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_RDRF_SHIFT), \
                                        /*!< Rx data register full flag, sets when the receive data buffer is full */
    LPUART_IDLE_LINE_DETECT           = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_IDLE_SHIFT), \
                                        /*!< Idle line detect flag, sets when idle line detected */
    LPUART_RX_OVERRUN                 = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_OR_SHIFT), \
                                        /*!< Rx Overrun sets if new data is received before data is read */
    LPUART_NOISE_DETECT               = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_NF_SHIFT), \
                                        /*!< Rx takes 3 samples of each received bit. If these differ, the flag sets */
    LPUART_FRAME_ERR                  = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_FE_SHIFT), \
                                        /*!< Frame error flag, sets if logic 0 was detected where stop bit expected */
    LPUART_PARITY_ERR                 = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_PF_SHIFT), \
                                        /*!< If parity enabled, sets upon parity error detection */
    LPUART_LIN_BREAK_DETECT           = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_LBKDIF_SHIFT), \
                                        /*!< LIN break detect interrupt flag, sets when LIN break char detected */
    LPUART_RX_ACTIVE_EDGE_DETECT      = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_RXEDGIF_SHIFT), \
                                        /*!< Rx pin active edge interrupt flag, sets when active edge detected */
    LPUART_RX_ACTIVE                  = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_RAF_SHIFT), \
                                        /*!< Receiver Active Flag (RAF), sets at beginning of valid start bit */
    LPUART_NOISE_IN_CURRENT_WORD      = (((uint32_t)LPUART_DATA_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_DATA_NOISY_SHIFT), \
                                        /*!< NOISY bit, sets if noise detected in current data word */
    LPUART_PARITY_ERR_IN_CURRENT_WORD = (((uint32_t)LPUART_DATA_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_DATA_PARITYE_SHIFT), \
                                        /*!< PARITYE bit, sets if noise detected in current data word */
#if FEATURE_LPUART_HAS_ADDRESS_MATCHING
    LPUART_MATCH_ADDR_ONE             = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_MA1F_SHIFT), \
                                        /*!< Address one match flag */
    LPUART_MATCH_ADDR_TWO             = (((uint32_t)LPUART_STAT_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_STAT_MA2F_SHIFT), \
                                        /*!< Address two match flag */
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
    LPUART_FIFO_TX_OF                 = (((uint32_t)LPUART_FIFO_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_FIFO_TXOF_SHIFT), \
                                        /*!< Transmitter FIFO buffer overflow */
    LPUART_FIFO_RX_UF                 = (((uint32_t)LPUART_FIFO_REG_ID << (uint32_t)LPUART_SHIFT) \
                                        | (uint32_t)LPUART_FIFO_RXUF_SHIFT) \
                                        /*!< Receiver FIFO buffer underflow */
#endif
} lpuart_status_flag_t;

/*! @brief LPUART interrupt configuration structure, default settings are 0 (disabled) */
typedef enum
{
    LPUART_INT_LIN_BREAK_DETECT  = (((uint32_t)LPUART_BAUD_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_BAUD_LBKDIE_SHIFT),  /*!< LIN break detect. */
    LPUART_INT_RX_ACTIVE_EDGE    = (((uint32_t)LPUART_BAUD_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_BAUD_RXEDGIE_SHIFT), /*!< RX Active Edge. */
    LPUART_INT_TX_DATA_REG_EMPTY = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_TIE_SHIFT),     /*!< Transmit data register empty. */
    LPUART_INT_TX_COMPLETE       = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_TCIE_SHIFT),    /*!< Transmission complete. */
    LPUART_INT_RX_DATA_REG_FULL  = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_RIE_SHIFT),     /*!< Receiver data register full. */
    LPUART_INT_IDLE_LINE         = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_ILIE_SHIFT),    /*!< Idle line. */
    LPUART_INT_RX_OVERRUN        = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_ORIE_SHIFT),    /*!< Receiver Overrun. */
    LPUART_INT_NOISE_ERR_FLAG    = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_NEIE_SHIFT),    /*!< Noise error flag. */
    LPUART_INT_FRAME_ERR_FLAG    = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_FEIE_SHIFT),    /*!< Framing error flag. */
    LPUART_INT_PARITY_ERR_FLAG   = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_PEIE_SHIFT),    /*!< Parity error flag. */
#if FEATURE_LPUART_HAS_ADDRESS_MATCHING
    LPUART_INT_MATCH_ADDR_ONE    = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_MA1IE_SHIFT),   /*!< Match address one flag. */
    LPUART_INT_MATCH_ADDR_TWO    = (((uint32_t)LPUART_CTRL_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_CTRL_MA2IE_SHIFT),   /*!< Match address two flag. */
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
    LPUART_INT_FIFO_TXOF         = (((uint32_t)LPUART_FIFO_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_FIFO_TXOFE_SHIFT),    /*!< Transmitter FIFO buffer interrupt */
    LPUART_INT_FIFO_RXUF         = (((uint32_t)LPUART_FIFO_REG_ID << (uint32_t)LPUART_SHIFT) \
                                   | (uint32_t)LPUART_FIFO_RXUFE_SHIFT)     /*!< Receiver FIFO buffer interrupt */
#endif
} lpuart_interrupt_t;


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name LPUART Common Configurations
 * @{
 */

/*!
 * @brief Initializes the LPUART controller.
 *
 * This function Initializes the LPUART controller to known state.
 *
 *
 * @param base LPUART base pointer.
 */
void LPUART_Init(LPUART_Type * base);

/*!
 * @brief Enable/Disable the LPUART transmitter.
 *
 * This function enables or disables the LPUART transmitter, based on the
 * parameter received.
 *
 *
 * @param base LPUART base pointer.
 * @param enable Enable(true) or disable(false) transmitter.
 */
static inline void LPUART_SetTransmitterCmd(LPUART_Type * base, bool enable)
{
    base->CTRL = (base->CTRL & ~LPUART_CTRL_TE_MASK) | ((enable ? 1UL : 0UL) << LPUART_CTRL_TE_SHIFT);
    /* Wait for the register write operation to complete */
    while((bool)((base->CTRL & LPUART_CTRL_TE_MASK) != 0U) != enable) {}
}

/*!
 * @brief Enable/Disable the LPUART receiver.
 *
 * This function enables or disables the LPUART receiver, based on the
 * parameter received.
 *
 *
 * @param base LPUART base pointer
 * @param enable Enable(true) or disable(false) receiver.
 */
static inline void LPUART_SetReceiverCmd(LPUART_Type * base, bool enable)
{
    base->CTRL = (base->CTRL & ~LPUART_CTRL_RE_MASK) | ((enable ? 1UL : 0UL) << LPUART_CTRL_RE_SHIFT);
    /* Wait for the register write operation to complete */
    while((bool)((base->CTRL & LPUART_CTRL_RE_MASK) != 0U) != enable) {}
}

/*!
 * @brief Sets the LPUART baud rate modulo divisor.
 *
 * This function sets the LPUART baud rate modulo divisor.
 *
 *
 * @param base LPUART base pointer.
 * @param baudRateDivisor The baud rate modulo division "SBR"
 */
static inline void LPUART_SetBaudRateDivisor(LPUART_Type * base, uint32_t baudRateDivisor)
{
    DEV_ASSERT((baudRateDivisor <= 0x1FFFU) && (baudRateDivisor >= 1U));
    uint32_t baudRegValTemp;

    baudRegValTemp = base->BAUD;
    baudRegValTemp &= ~(LPUART_BAUD_SBR_MASK);
    /* Removed the shift operation as the SBR field position is zero; shifting with 0 violates MISRA */
    baudRegValTemp |= baudRateDivisor & LPUART_BAUD_SBR_MASK;
    base->BAUD = baudRegValTemp;
}

/*!
 * @brief Gets the LPUART baud rate modulo divisor.
 *
 * This function gets the LPUART baud rate modulo divisor.
 *
 *
 * @param base LPUART base pointer.
 * @return The baud rate modulo division "SBR"
 */
static inline uint16_t LPUART_GetBaudRateDivisor(const LPUART_Type * base)
{
    return ((uint16_t)((base->BAUD & LPUART_BAUD_SBR_MASK) >> LPUART_BAUD_SBR_SHIFT));
}

#if FEATURE_LPUART_HAS_BAUD_RATE_OVER_SAMPLING_SUPPORT
/*!
 * @brief Sets the LPUART baud rate oversampling ratio
 *
 * This function sets the LPUART baud rate oversampling ratio.
 * (Note: Feature available on select LPUART instances used together with baud rate programming)
 * The oversampling ratio should be set between 4x (00011) and 32x (11111). Writing
 * an invalid oversampling ratio results in an error and is set to a default
 * 16x (01111) oversampling ratio.
 * Disable the transmitter/receiver before calling this function.
 *
 *
 * @param base LPUART base pointer.
 * @param overSamplingRatio The oversampling ratio "OSR"
 */
static inline void LPUART_SetOversamplingRatio(LPUART_Type * base, uint32_t overSamplingRatio)
{
    DEV_ASSERT(overSamplingRatio <= 0x1FU);
    uint32_t baudRegValTemp;

    baudRegValTemp = base->BAUD;
    baudRegValTemp &= ~(LPUART_BAUD_OSR_MASK);
    baudRegValTemp |= LPUART_BAUD_OSR(overSamplingRatio);
    base->BAUD = baudRegValTemp;
}

/*!
 * @brief Gets the LPUART baud rate oversampling ratio
 *
 * This function gets the LPUART baud rate oversampling ratio.
 * (Note: Feature available on select LPUART instances used together with baud rate programming)
 *
 *
 * @param base LPUART base pointer.
 * @return The oversampling ratio "OSR"
 */
static inline uint8_t LPUART_GetOversamplingRatio(const LPUART_Type * base)
{
    return ((uint8_t)((base->BAUD & LPUART_BAUD_OSR_MASK) >> LPUART_BAUD_OSR_SHIFT));
}
#endif

#if FEATURE_LPUART_HAS_BOTH_EDGE_SAMPLING_SUPPORT
/*!
 * @brief Configures the LPUART baud rate both edge sampling
 *
 * This function configures the LPUART baud rate both edge sampling.
 * (Note: Feature available on select LPUART instances used with baud rate programming)
 * When enabled, the received data is sampled on both edges of the baud rate clock.
 * This must be set when the oversampling ratio is between 4x and 7x.
 * This function should only be called when the receiver is disabled.
 *
 *
 * @param base LPUART base pointer.
 * @param enable   Enable (1) or Disable (0) Both Edge Sampling
 */
static inline void LPUART_EnableBothEdgeSamplingCmd(LPUART_Type * base)
{
    base->BAUD |= LPUART_BAUD_BOTHEDGE_MASK;
}
#endif

/*!
 * @brief Configures the number of bits per character in the LPUART controller.
 *
 * This function configures the number of bits per character in the LPUART controller.
 * In some LPUART instances, the user should disable the transmitter/receiver
 * before calling this function.
 * Generally, this may be applied to all LPUARTs to ensure safe operation.
 *
 * @param base LPUART base pointer.
 * @param bitCountPerChar  Number of bits per char (8, 9, or 10, depending on the LPUART instance)
 * @param parity  Specifies whether parity bit is enabled
 */
void LPUART_SetBitCountPerChar(LPUART_Type * base, lpuart_bit_count_per_char_t bitCountPerChar, bool parity);

/*!
 * @brief Configures parity mode in the LPUART controller.
 *
 * This function configures parity mode in the LPUART controller.
 * In some LPUART instances, the user should disable the transmitter/receiver
 * before calling this function.
 * Generally, this may be applied to all LPUARTs to ensure safe operation.
 *
 * @param base LPUART base pointer.
 * @param parityModeType  Parity mode (enabled, disable, odd, even - see parity_mode_t struct)
 */
void LPUART_SetParityMode(LPUART_Type * base, lpuart_parity_mode_t parityModeType);

/*!
 * @brief Configures the number of stop bits in the LPUART controller.
 *
 * This function configures the number of stop bits in the LPUART controller.
 * In some LPUART instances, the user should disable the transmitter/receiver
 * before calling this function.
 * Generally, this may be applied to all LPUARTs to ensure safe operation.
 *
 * @param base LPUART base pointer.
 * @param stopBitCount Number of stop bits (1 or 2 - see lpuart_stop_bit_count_t struct)
 */
static inline void LPUART_SetStopBitCount(LPUART_Type * base, lpuart_stop_bit_count_t stopBitCount)
{
    base->BAUD = (base->BAUD & ~LPUART_BAUD_SBNS_MASK) | ((uint32_t)stopBitCount << LPUART_BAUD_SBNS_SHIFT);
}

/*@}*/

/*!
 * @name LPUART Interrupts and DMA
 * @{
 */

/*!
 * @brief Configures the LPUART module interrupts.
 *
 * This function configures the LPUART module interrupts to enable/disable various interrupt sources.
 *
 *
 * @param   base LPUART module base pointer.
 * @param   intSrc LPUART interrupt configuration data.
 * @param   enable   true: enable, false: disable.
 */
void LPUART_SetIntMode(LPUART_Type * base, lpuart_interrupt_t intSrc, bool enable);

/*!
 * @brief Returns LPUART module interrupts state.
 *
 * This function returns whether a certain LPUART module interrupt is enabled or disabled.
 *
 *
 * @param   base LPUART module base pointer.
 * @param   intSrc LPUART interrupt configuration data.
 * @return  true: enable, false: disable.
 */
bool LPUART_GetIntMode(const LPUART_Type * base, lpuart_interrupt_t intSrc);

#if FEATURE_LPUART_HAS_DMA_ENABLE
/*!
 * @brief Configures DMA requests.
 *
 * This function configures DMA requests for LPUART Transmitter.
 *
 *
 * @param base LPUART base pointer
 * @param enable Transmit DMA request configuration (enable:1 /disable: 0)
 */
static inline void LPUART_SetTxDmaCmd(LPUART_Type * base, bool enable)
{
    base->BAUD = (base->BAUD & ~LPUART_BAUD_TDMAE_MASK) | ((enable ? 1UL : 0UL) << LPUART_BAUD_TDMAE_SHIFT);
}

/*!
 * @brief Configures DMA requests.
 *
 * This function configures DMA requests for LPUART Receiver.
 *
 *
 * @param base LPUART base pointer
 * @param enable Receive DMA request configuration (enable: 1/disable: 0)
 */
static inline void LPUART_SetRxDmaCmd(LPUART_Type * base, bool enable)
{
    base->BAUD = (base->BAUD & ~LPUART_BAUD_RDMAE_MASK) | ((enable ? 1UL : 0UL) << LPUART_BAUD_RDMAE_SHIFT);
}
#endif

/*@}*/

/*!
 * @name LPUART Transfer Functions
 * @{
 */

/*!
 * @brief Sends the LPUART 8-bit character.
 *
 * This functions sends an 8-bit character.
 *
 *
 * @param base LPUART Instance
 * @param data     data to send (8-bit)
 */
static inline void LPUART_Putchar(LPUART_Type * base, uint8_t data)
{
    volatile uint8_t * dataRegBytes = (volatile uint8_t *)(&(base->DATA));
    dataRegBytes[0] = data;
}

/*!
 * @brief Sends the LPUART 9-bit character.
 *
 * This functions sends a 9-bit character.
 *
 *
 * @param base LPUART Instance
 * @param data     data to send (9-bit)
 */
void LPUART_Putchar9(LPUART_Type * base, uint16_t data);

/*!
 * @brief Sends the LPUART 10-bit character (Note: Feature available on select LPUART instances).
 *
 * This functions sends a 10-bit character.
 *
 *
 * @param base LPUART Instance
 * @param data   data to send (10-bit)
 */
void LPUART_Putchar10(LPUART_Type * base, uint16_t data);

/*!
 * @brief Gets the LPUART 8-bit character.
 *
 * This functions receives an 8-bit character.
 *
 *
 * @param base LPUART base pointer
 * @param readData Data read from receive (8-bit)
 */
static inline void LPUART_Getchar(const LPUART_Type * base, uint8_t *readData)
{
    DEV_ASSERT(readData != NULL);

    *readData = (uint8_t)base->DATA;
}

/*!
 * @brief Gets the LPUART 9-bit character.
 *
 * This functions receives a 9-bit character.
 *
 *
 * @param base LPUART base pointer
 * @param readData Data read from receive (9-bit)
 */
void LPUART_Getchar9(const LPUART_Type * base, uint16_t *readData);

/*!
 * @brief Gets the LPUART 10-bit character.
 *
 * This functions receives a 10-bit character.
 *
 *
 * @param base LPUART base pointer
 * @param readData Data read from receive (10-bit)
 */
void LPUART_Getchar10(const LPUART_Type * base, uint16_t *readData);

/*@}*/

/*!
 * @name LPUART Status Flags
 * @{
 */

/*!
 * @brief  LPUART get status flag
 *
 * This function returns the state of a status flag.
 *
 *
 * @param base LPUART base pointer
 * @param statusFlag  The status flag to query
 * @return Whether the current status flag is set(true) or not(false).
 */
bool LPUART_GetStatusFlag(const LPUART_Type * base, lpuart_status_flag_t statusFlag);

/*!
 * @brief LPUART clears an individual status flag.
 *
 * This function clears an individual status flag (see lpuart_status_flag_t for list of status bits).
 *
 *
 * @param base LPUART base pointer
 * @param statusFlag  Desired LPUART status flag to clear
 * @return STATUS_SUCCESS if successful or STATUS_ERROR if an error occured
 */
status_t LPUART_ClearStatusFlag(LPUART_Type * base, lpuart_status_flag_t statusFlag);

/*@}*/

/*!
 * @name LPUART Special Feature Configurations
 * @{
 */

/*!
 * @brief Enable or disable the LPUART error interrupts.
 *
 * This function configures the error interrupts (parity, noise, overrun, framing).
 *
 *
 * @param base LPUART base pointer
 * @param enable true - enable, false - disable error interrupts
 */
void LPUART_SetErrorInterrupts(LPUART_Type * base, bool enable);

/*!
 * @brief  Clears the error flags treated by the driver
 *
 * This function clears the error flags treated by the driver.
 * *
 * @param base LPUART base pointer
 */
static inline void LPUART_DRV_ClearErrorFlags(LPUART_Type * base)
{
    uint32_t mask = LPUART_STAT_OR_MASK | \
                    LPUART_STAT_NF_MASK | \
                    LPUART_STAT_FE_MASK | \
                    LPUART_STAT_PF_MASK;

    base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | mask;
}

/*@}*/

#if defined(__cplusplus)
}
#endif

#endif /* LPUART_HW_ACCESS_H__ */
/*******************************************************************************
 * EOF
 ******************************************************************************/
