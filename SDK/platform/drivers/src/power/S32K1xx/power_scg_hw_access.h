/*
 * Copyright 2018 NXP
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

#ifndef POWER_SCG_HW_ACCESS_H
#define POWER_SCG_HW_ACCESS_H

#include "device_registers.h"

/*!
 * @file power_scg_hw_access.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed
 * between pointer to void and an arithmetic type.
 * The address of hardware modules is provided as integer so
 * it needs to be cast to pointer.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, A conversion should not be performed
 * between a pointer to object and an integer type.
 * The address of hardware modules is provided as integer so
 * a conversion between a pointer and an integer has to be performed
 */

/*!
 * power_scg_hw_access
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#if FEATURE_HAS_SPLL_CLK
/*!
 * @brief Set the SPLL clock source was enabled or disabled
 *
 * This function sets SPLL clock source enable or disabled.
 * This one is used in the switching very low power mode sequence.
 *
 * @param[in] enable This variable select the SPLL available or not.
 */
static inline void SCG_SetEnableSPLL(bool enable)
{
    uint32_t regValue = SCG->SPLLCSR;
    regValue &= ~SCG_SPLLCSR_SPLLEN_MASK;
    if (enable == true)
    {
        regValue |= SCG_SPLLCSR_SPLLEN(1U);
    }
    else
    {
        regValue |= SCG_SPLLCSR_SPLLEN(0U);
    }
    SCG->SPLLCSR = regValue;
}

/*!
 * @brief Get status of SPLL clock source was enabled or disabled
 *
 * This function gets SPLL clock source enable or disabled.
 * This one is used in the switching very low power mode sequence.
 *
 * @return true  : SPLL enabled
           false : SPLL disable
 */
static inline bool SCG_GetEnableSPLL(void)
{
    uint32_t regValue = SCG->SPLLCSR;
    regValue = (regValue & SCG_SPLLCSR_SPLLEN_MASK) >> SCG_SPLLCSR_SPLLEN_SHIFT;

    return (regValue == 0U) ? false : true;
}

/*!
 * @brief Check clock source in HSRUN mode
 *
 * This function checks SPLL as source in HSRUN mode.
 * This one is used in the switching very low power mode sequence.
 *
 * @return true  : SPLL is clock source in HSRUN.
           false : SPLL is not clock source in HSRUN.
 */
static inline bool SCG_GetHsrunSelectSPLL(void)
{
    uint32_t regValue = SCG->HCCR;
    regValue = (regValue & SCG_HCCR_SCS_MASK) >> SCG_HCCR_SCS_SHIFT;

    return (regValue == FEATURE_SCG_SPLL_VALUE) ? true : false;
}

#endif

/*!
 * @brief Set the FIRC clock source was enabled or disabled
 *
 * This function sets FIRC clock source enable or disabled.
 * This one is used in the switching very low power mode sequence.
 *
 * @param[in] enable This variable select the FIRC available or not.
 */
static inline void SCG_SetEnableFIRC(bool enable)
{
    uint32_t regValue = SCG->FIRCCSR;
    regValue &= ~SCG_FIRCCSR_FIRCEN_MASK;
    if (enable == true)
    {
        regValue |= SCG_FIRCCSR_FIRCEN(1U);
    }
    else
    {
        regValue |= SCG_FIRCCSR_FIRCEN(0U);
    }
    SCG->FIRCCSR = regValue;
}

/*!
 * @brief Get status of FIRC clock source was enabled or disabled
 *
 * This function gets FIRC clock source enable or disabled.
 * This one is used in the switching very low power mode sequence.
 *
 * @return true  : FIRC enabled
           false : FIRC disable
 */
static inline bool SCG_GetEnableFIRC(void)
{
    uint32_t regValue = SCG->FIRCCSR;
    regValue = (regValue & SCG_FIRCCSR_FIRCEN_MASK) >> SCG_FIRCCSR_FIRCEN_SHIFT;

    return (regValue == 0U) ? false : true;
}

/*!
 * @brief Set the SOSC clock source was enabled or disabled
 *
 * This function sets SOSC clock source enable or disabled.
 * This one is used in the switching very low power mode sequence.
 *
 * @param[in] enable This variable select the SOSC available or not.
 */
static inline void SCG_SetEnableSOSC(bool enable)
{
    uint32_t regValue = SCG->SOSCCSR;
    regValue &= ~SCG_SOSCCSR_SOSCEN_MASK;
    if (enable == true)
    {
        regValue |= SCG_SOSCCSR_SOSCEN(1U);
    }
    else
    {
        regValue |= SCG_SOSCCSR_SOSCEN(0U);
    }
    SCG->SOSCCSR = regValue;
}

/*!
 * @brief Get status of SOSC clock source was enabled or disabled
 *
 * This function gets SOSC clock source enable or disabled.
 * This one is used in the switching very low power mode sequence.
 *
 * @return true  : SOSC enabled
           false : SOSC disable
 */
static inline bool SCG_GetEnableSOSC(void)
{
    uint32_t regValue = SCG->SOSCCSR;
    regValue = (regValue & SCG_SOSCCSR_SOSCEN_MASK) >> SCG_SOSCCSR_SOSCEN_SHIFT;

    return (regValue == 0U) ? false : true;
}

/*@}*/

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*! @}*/

#endif /* POWER_SCG_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
