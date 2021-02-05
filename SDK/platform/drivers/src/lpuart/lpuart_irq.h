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

#ifndef LPUART_IRQ_H__
#define LPUART_IRQ_H__

#include "device_registers.h"
#include "interrupt_manager.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void LPUART_DRV_IRQHandler(uint32_t instance);

/*******************************************************************************
 *  Default interrupt handlers signatures
 ******************************************************************************/

#if (LPUART_INSTANCE_COUNT > 0U)
/*! @brief LPUART0 interrupt handler. */
void LPUART0_IrqHandler(void);
#endif

#if (LPUART_INSTANCE_COUNT > 1U)
/*! @brief LPUART1 interrupt handler. */
void LPUART1_IrqHandler(void);
#endif

#if (LPUART_INSTANCE_COUNT > 2U)
/*! @brief LPUART2 interrupt handler. */
void LPUART2_IrqHandler(void);
#endif

#if (LPUART_INSTANCE_COUNT > 3U)
/*! @brief LPUART3 interrupt handler. */
void LPUART3_IrqHandler(void);
#endif

/*! Array storing references to LPUART irq handlers */
extern isr_t g_lpuartIsr[LPUART_INSTANCE_COUNT];

#endif /* LPUART_IRQ_H__ */
/*******************************************************************************
 * EOF
 ******************************************************************************/
