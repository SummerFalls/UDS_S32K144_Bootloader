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
 * @file lpuart_irq.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, Function not defined with external linkage.
 * The functions are not defined static because they are referenced in .s startup files.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in driver c file, so it must remain global.
 */

#include "lpuart_irq.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

#if (LPUART_INSTANCE_COUNT > 0U)
/* Implementation of LPUART0 handler named in startup code. */
void LPUART0_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(0);
}
#endif

#if (LPUART_INSTANCE_COUNT > 1U)
/* Implementation of LPUART1 handler named in startup code. */
void LPUART1_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(1);
}
#endif

#if (LPUART_INSTANCE_COUNT > 2U)
/* Implementation of LPUART2 handler named in startup code. */
void LPUART2_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(2);
}
#endif

#if (LPUART_INSTANCE_COUNT > 3U)
/* Implementation of LPUART3 handler named in startup code. */
void LPUART3_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(3);
}
#endif

/* Array storing references to LPUART irq handlers */
isr_t g_lpuartIsr[LPUART_INSTANCE_COUNT] =
{
#if (LPUART_INSTANCE_COUNT > 0U)
    LPUART0_IrqHandler,
#endif
#if (LPUART_INSTANCE_COUNT > 1U)
    LPUART1_IrqHandler,
#endif
#if (LPUART_INSTANCE_COUNT > 2U)
    LPUART2_IrqHandler,
#endif
#if (LPUART_INSTANCE_COUNT > 3U)
    LPUART3_IrqHandler,
#endif
};

/*******************************************************************************
 * EOF
 ******************************************************************************/
