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

/*!
 * @file lptmr_hw_access.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 */

#include "lptmr_hw_access.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : LPTMR_Init
 * Description   : This function configures all registers of the LPTMR instance to reset value.
 *
 *END**************************************************************************/
void LPTMR_Init(LPTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    /* First, disable the module so we can write the registers */
    uint32_t tmp = base->CSR;
    tmp &= ~(LPTMR_CSR_TEN_MASK | LPTMR_CSR_TCF_MASK);
    tmp |= LPTMR_CSR_TEN(0u);
    base->CSR = tmp;

    base->CSR = LPTMR_CSR_TEN(0u) | \
                LPTMR_CSR_TMS(0u) | \
                LPTMR_CSR_TFC(0u) | \
                LPTMR_CSR_TPP(0u) | \
                LPTMR_CSR_TPS(0u) | \
                LPTMR_CSR_TIE(0u) | \
                LPTMR_CSR_TCF(0u) | \
                LPTMR_CSR_TDRE(0u);

    base->PSR = LPTMR_PSR_PCS(0u) | \
                LPTMR_PSR_PBYP(0u) | \
                LPTMR_PSR_PRESCALE(0u);

    base->CMR = LPTMR_CMR_COMPARE(0u);
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
