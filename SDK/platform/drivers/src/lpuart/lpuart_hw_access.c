/*
 * Copyright (c) 2013 - 2016, Freescale Semiconductor, Inc.
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
 * @file lpuart_hw_access.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * The functions are only used by LPUART driver, errors appear for LIN driver where
 * they are not used.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, Expression assigned to a
 * narrower or different essential type.
 * The assign operations are safe as the baud rate calculation algorithm cannot
 * overflow the result.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, Return statement before end of
 * function .
 * The return statement before end of function is used for simpler code structure
 * and better readability.
 */

#include "lpuart_hw_access.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_Init
 * Description   : Initializes the LPUART controller to known state, using
 *                 register reset values defined in the reference manual.
 *END**************************************************************************/
void LPUART_Init(LPUART_Type * base)
{
    /* Set the default oversampling ratio (16) and baud-rate divider (4) */
    base->BAUD = ((uint32_t)((FEATURE_LPUART_DEFAULT_OSR << LPUART_BAUD_OSR_SHIFT) | \
                 (FEATURE_LPUART_DEFAULT_SBR << LPUART_BAUD_SBR_SHIFT)));
    /* Clear the error/interrupt flags */
    base->STAT = FEATURE_LPUART_STAT_REG_FLAGS_MASK;
    /* Reset all features/interrupts by default */
    base->CTRL = 0x00000000;
    /* Reset match addresses */
    base->MATCH = 0x00000000;
#if FEATURE_LPUART_HAS_MODEM_SUPPORT
    /* Reset IrDA modem features */
    base->MODIR = 0x00000000;
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
    /* Reset FIFO feature */
    base->FIFO = FEATURE_LPUART_FIFO_RESET_MASK;
    /* Reset FIFO Watermark values */
    base->WATER = 0x00000000;
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_SetBitCountPerChar
 * Description   : Configures the number of bits per char in LPUART controller.
 * In some LPUART instances, the user should disable the transmitter/receiver
 * before calling this function.
 * Generally, this may be applied to all LPUARTs to ensure safe operation.
 *END**************************************************************************/
void LPUART_SetBitCountPerChar(LPUART_Type * base, lpuart_bit_count_per_char_t bitCountPerChar, bool parity)
{
    uint32_t tmpBitCountPerChar = (uint32_t)bitCountPerChar;
    if (parity)
    {
        tmpBitCountPerChar += 1U;
    }

    if (tmpBitCountPerChar == (uint32_t)LPUART_10_BITS_PER_CHAR)
    {
        base->BAUD = (base->BAUD & ~LPUART_BAUD_M10_MASK) | ((uint32_t)1U << LPUART_BAUD_M10_SHIFT);
    }
    else
    {
        /* config 8-bit (M=0) or 9-bits (M=1) */
        base->CTRL = (base->CTRL & ~LPUART_CTRL_M_MASK) | (tmpBitCountPerChar << LPUART_CTRL_M_SHIFT);
        /* clear M10 to make sure not 10-bit mode */
        base->BAUD &= ~LPUART_BAUD_M10_MASK;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_SetParityMode
 * Description   : Configures parity mode in the LPUART controller.
 * In some LPUART instances, the user should disable the transmitter/receiver
 * before calling this function.
 * Generally, this may be applied to all LPUARTs to ensure safe operation.
 *END**************************************************************************/
void LPUART_SetParityMode(LPUART_Type * base, lpuart_parity_mode_t parityModeType)
{
    base->CTRL = (base->CTRL & ~LPUART_CTRL_PE_MASK) | (((uint32_t)parityModeType >> 1U) << LPUART_CTRL_PE_SHIFT);
    base->CTRL = (base->CTRL & ~LPUART_CTRL_PT_MASK) | (((uint32_t)parityModeType & 1U) << LPUART_CTRL_PT_SHIFT);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_Putchar9
 * Description   : Sends the LPUART 9-bit character.
 *END**************************************************************************/
void LPUART_Putchar9(LPUART_Type * base, uint16_t data)
{
    uint8_t ninthDataBit;
    volatile uint8_t * dataRegBytes = (volatile uint8_t *)(&(base->DATA));


    ninthDataBit = (uint8_t)((data >> 8U) & 0x1U);

    /* write to ninth data bit T8(where T[0:7]=8-bits, T8=9th bit) */
    base->CTRL = (base->CTRL & ~LPUART_CTRL_R9T8_MASK) | ((uint32_t)(ninthDataBit) << LPUART_CTRL_R9T8_SHIFT);

    /* write 8-bits to the data register*/
    dataRegBytes[0] = (uint8_t)data;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_Putchar10
 * Description   : Sends the LPUART 10-bit character.
 *END**************************************************************************/
void LPUART_Putchar10(LPUART_Type * base, uint16_t data)
{
    uint8_t ninthDataBit, tenthDataBit;
    uint32_t ctrlRegVal;
    volatile uint8_t * dataRegBytes = (volatile uint8_t *)(&(base->DATA));

    ninthDataBit = (uint8_t)((data >> 8U) & 0x1U);
    tenthDataBit = (uint8_t)((data >> 9U) & 0x1U);

    /* write to ninth/tenth data bit (T[0:7]=8-bits, T8=9th bit, T9=10th bit) */
    ctrlRegVal = base->CTRL;
    ctrlRegVal = (ctrlRegVal & ~LPUART_CTRL_R9T8_MASK) | ((uint32_t)ninthDataBit << LPUART_CTRL_R9T8_SHIFT);
    ctrlRegVal = (ctrlRegVal & ~LPUART_CTRL_R8T9_MASK) | ((uint32_t)tenthDataBit << LPUART_CTRL_R8T9_SHIFT);
    base->CTRL = ctrlRegVal;

    /* write to 8-bits to the data register */
    dataRegBytes[0] = (uint8_t)data;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_Getchar9
 * Description   : Gets the LPUART 9-bit character.
 *END**************************************************************************/
void LPUART_Getchar9(const LPUART_Type * base, uint16_t *readData)
{
    DEV_ASSERT(readData != NULL);

    /* get ninth bit from lpuart data register */
    *readData = (uint16_t)(((base->CTRL >> LPUART_CTRL_R8T9_SHIFT) & 1U) << 8);

    /* get 8-bit data from the lpuart data register */
    *readData |= (uint8_t)base->DATA;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_Getchar10
 * Description   : Gets the LPUART 10-bit character
 *END**************************************************************************/
void LPUART_Getchar10(const LPUART_Type * base, uint16_t *readData)
{
    DEV_ASSERT(readData != NULL);

    /* read tenth data bit */
    *readData = (uint16_t)(((base->CTRL >> LPUART_CTRL_R9T8_SHIFT) & 1U) << 9);
    /* read ninth data bit */
    *readData |= (uint16_t)(((base->CTRL >> LPUART_CTRL_R8T9_SHIFT) & 1U) << 8);

    /* get 8-bit data */
    *readData |= (uint8_t)base->DATA;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_SetIntMode
 * Description   : Configures the LPUART module interrupts to enable/disable
 * various interrupt sources.
 *END**************************************************************************/
void LPUART_SetIntMode(LPUART_Type * base, lpuart_interrupt_t intSrc, bool enable)
{
    uint32_t reg = (uint32_t)(intSrc) >> LPUART_SHIFT;
    uint32_t intRegOffset = (uint16_t)(intSrc);

    switch (reg)
    {
        case LPUART_BAUD_REG_ID:
            base->BAUD = (base->BAUD & ~(1UL << intRegOffset)) | ((enable ? 1U : 0U) << intRegOffset);
            break;
        case LPUART_CTRL_REG_ID:
            base->CTRL = (base->CTRL & ~(1UL << intRegOffset)) | ((enable ? 1U : 0U) << intRegOffset);
            break;
#if FEATURE_LPUART_HAS_MODEM_SUPPORT
        case LPUART_MODIR_REG_ID:
            base->MODIR = (base->MODIR & ~(1UL << intRegOffset)) | ((enable ? 1U : 0U) << intRegOffset);
            break;
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
        case LPUART_FIFO_REG_ID:
            base->FIFO = (base->FIFO & (~FEATURE_LPUART_FIFO_REG_FLAGS_MASK & ~(1UL << intRegOffset))) | \
                         ((enable ? 1U : 0U) << intRegOffset);
            break;
#endif
        default :
            /* Invalid parameter: return */
            break;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_GetIntMode
 * Description   : Returns whether LPUART module interrupt is enabled/disabled.
 *END**************************************************************************/
bool LPUART_GetIntMode(const LPUART_Type * base, lpuart_interrupt_t intSrc)
{
    uint32_t reg = (uint32_t)(intSrc) >> LPUART_SHIFT;
    bool retVal = false;

    switch ( reg )
    {
        case LPUART_BAUD_REG_ID:
            retVal = (((base->BAUD >> (uint16_t)(intSrc)) & 1U) > 0U);
            break;
        case LPUART_CTRL_REG_ID:
            retVal = (((base->CTRL >> (uint16_t)(intSrc)) & 1U) > 0U);
            break;
#if FEATURE_LPUART_HAS_MODEM_SUPPORT
        case LPUART_MODIR_REG_ID:
            retVal = (((base->MODIR >> (uint16_t)(intSrc)) & 1U) > 0U);
            break;
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
        case LPUART_FIFO_REG_ID:
            retVal = (((base->FIFO >> (uint16_t)(intSrc)) & 1U) > 0U);
            break;
#endif
        default :
            /* Invalid parameter: return */
            break;
    }

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_GetStatusFlag
 * Description   : LPUART get status flag by passing flag enum.
 *END**************************************************************************/
bool LPUART_GetStatusFlag(const LPUART_Type * base, lpuart_status_flag_t statusFlag)
{
    uint32_t reg = (uint32_t)(statusFlag) >> LPUART_SHIFT;
    bool retVal = false;

    switch ( reg )
    {
        case LPUART_STAT_REG_ID:
            retVal = (((base->STAT >> (uint16_t)(statusFlag)) & 1U) > 0U);
            break;
        case LPUART_DATA_REG_ID:
            retVal = (((base->DATA >> (uint16_t)(statusFlag)) & 1U) > 0U);
            break;
#if FEATURE_LPUART_HAS_MODEM_SUPPORT
        case LPUART_MODIR_REG_ID:
            retVal = (((base->MODIR >> (uint16_t)(statusFlag)) & 1U) > 0U);
            break;
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
        case LPUART_FIFO_REG_ID:
            retVal = (((base->FIFO >> (uint16_t)(statusFlag)) & 1U) > 0U);
            break;
#endif
        default:
            /* Invalid parameter: return */
            break;
    }

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_ClearStatusFlag
 * Description   : LPUART clears an individual status flag
 * (see lpuart_status_flag_t for list of status bits).
 *END**************************************************************************/
status_t LPUART_ClearStatusFlag(LPUART_Type * base,
                                    lpuart_status_flag_t statusFlag)
{
    status_t returnCode = STATUS_SUCCESS;

    switch(statusFlag)
    {
        /* These flags are cleared automatically by other lpuart operations
         * and cannot be manually cleared, return error code */
        case LPUART_TX_DATA_REG_EMPTY:
        case LPUART_TX_COMPLETE:
        case LPUART_RX_DATA_REG_FULL:
        case LPUART_RX_ACTIVE:
#if FEATURE_LPUART_HAS_EXTENDED_DATA_REGISTER_FLAGS
        case LPUART_NOISE_IN_CURRENT_WORD:
        case LPUART_PARITY_ERR_IN_CURRENT_WORD:
#endif
            returnCode = STATUS_ERROR;
            break;

        case LPUART_IDLE_LINE_DETECT:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_IDLE_MASK;
            break;

        case LPUART_RX_OVERRUN:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_OR_MASK;
            break;

        case LPUART_NOISE_DETECT:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_NF_MASK;
            break;

        case LPUART_FRAME_ERR:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_FE_MASK;
            break;

        case LPUART_PARITY_ERR:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_PF_MASK;
            break;

        case LPUART_LIN_BREAK_DETECT:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_LBKDIF_MASK;
            break;

        case LPUART_RX_ACTIVE_EDGE_DETECT:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_RXEDGIF_MASK;
            break;

#if FEATURE_LPUART_HAS_ADDRESS_MATCHING
        case LPUART_MATCH_ADDR_ONE:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_MA1F_MASK;
            break;
        case LPUART_MATCH_ADDR_TWO:
            base->STAT = (base->STAT & (~FEATURE_LPUART_STAT_REG_FLAGS_MASK)) | LPUART_STAT_MA2F_MASK;
            break;
#endif
#if FEATURE_LPUART_FIFO_SIZE > 0U
        case LPUART_FIFO_TX_OF:
            base->FIFO = (base->FIFO & (~FEATURE_LPUART_FIFO_REG_FLAGS_MASK)) | LPUART_FIFO_TXOF_MASK;
            break;
        case LPUART_FIFO_RX_UF:
            base->FIFO = (base->FIFO & (~FEATURE_LPUART_FIFO_REG_FLAGS_MASK)) | LPUART_FIFO_RXUF_MASK;
            break;
#endif
        default:
            returnCode = STATUS_ERROR;
            break;
    }

    return (returnCode);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : LPUART_SetErrorInterrupts
 * Description   : Enable or disable the LPUART error interrupts.
 *END**************************************************************************/
void LPUART_SetErrorInterrupts(LPUART_Type * base, bool enable)
{
    /* Configure the error interrupts */
    LPUART_SetIntMode(base, LPUART_INT_RX_OVERRUN, enable);
    LPUART_SetIntMode(base, LPUART_INT_PARITY_ERR_FLAG, enable);
    LPUART_SetIntMode(base, LPUART_INT_NOISE_ERR_FLAG, enable);
    LPUART_SetIntMode(base, LPUART_INT_FRAME_ERR_FLAG, enable);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
