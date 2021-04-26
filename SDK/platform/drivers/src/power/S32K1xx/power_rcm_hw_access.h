/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
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

#ifndef POWER_RCM_HW_ACCESS_H
#define POWER_RCM_HW_ACCESS_H

#include "device_registers.h"
#include "power_manager_S32K1xx.h"

/*! @file power_rcm_hw_access.h */

/*!
 * @ingroup power_rcm_hw_access
 * @defgroup power_rcm_hw_access
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*! @name Reset Control Module APIs*/
/*@{*/

static inline bool RCM_GetSrcStatusCmd(const RCM_Type * const baseAddr,
                         const rcm_source_names_t srcName)
{
    bool retValue;
    uint32_t regValue = (uint32_t)baseAddr->SRS;

    DEV_ASSERT(srcName < RCM_SRC_NAME_MAX);

    switch (srcName)
    {
        case RCM_LOW_VOLT_DETECT:              /* low voltage detect reset */
            regValue = (regValue & RCM_SRS_LVD_MASK) >> RCM_SRS_LVD_SHIFT;
            break;
        case RCM_LOSS_OF_CLK:                  /* loss of clock reset */
            regValue = (regValue & RCM_SRS_LOC_MASK) >> RCM_SRS_LOC_SHIFT;
            break;
        case RCM_LOSS_OF_LOCK:                 /* loss of lock reset */
            regValue = (regValue & RCM_SRS_LOL_MASK) >> RCM_SRS_LOL_SHIFT;
            break;
#if FEATURE_RCM_HAS_CMU_LOSS_OF_CLOCK          /*!< CMU Loss of lock reset */
        case RCM_CMU_LOC:
            regValue = (regValue & RCM_SRS_CMU_LOC_MASK) >> RCM_SRS_CMU_LOC_SHIFT;
            break;     
#endif
        case RCM_WATCH_DOG:                    /* watch dog reset */
            regValue = (regValue & RCM_SRS_WDOG_MASK) >> RCM_SRS_WDOG_SHIFT;
            break;
        case RCM_EXTERNAL_PIN:                 /* external pin reset */
            regValue = (regValue & RCM_SRS_PIN_MASK) >> RCM_SRS_PIN_SHIFT;
            break;
        case RCM_POWER_ON:                     /* power on reset */
            regValue = (regValue & RCM_SRS_POR_MASK) >> RCM_SRS_POR_SHIFT;
            break;
        case RCM_SJTAG:                        /* JTAG generated reset */
            regValue = (regValue & RCM_SSRS_SJTAG_MASK) >> RCM_SSRS_SJTAG_SHIFT;
            break;
        case RCM_CORE_LOCKUP:                  /* core lockup reset */
            regValue = (regValue & RCM_SRS_LOCKUP_MASK) >> RCM_SRS_LOCKUP_SHIFT;
            break;
        case RCM_SOFTWARE:                     /* software reset */
            regValue = (regValue & RCM_SRS_SW_MASK) >> RCM_SRS_SW_SHIFT;
            break;
        case RCM_SMDM_AP:                      /* MDM-AP system reset */
            regValue = (regValue & RCM_SSRS_SMDM_AP_MASK) >> RCM_SSRS_SMDM_AP_SHIFT;
            break;
        case RCM_STOP_MODE_ACK_ERR:            /* stop mode ack error reset */
            regValue = (regValue & RCM_SRS_SACKERR_MASK) >> RCM_SRS_SACKERR_SHIFT;
            break;
        default:
            /* invalid command */
            regValue = 0U;
            break;
    }

    retValue = (regValue == 0UL) ? false : true;

    return retValue;
}

/*@}*/

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*! @}*/

#endif /* POWER_RCM_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
