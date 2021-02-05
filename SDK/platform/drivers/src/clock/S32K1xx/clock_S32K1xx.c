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
/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, There shall be no occurrence of
 * undefined or critical unspecified behaviour.
 * The addresses of the stack variables are only used at local scope.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, Expression assigned to a narrower or different essential type.
 * The cast is required to perform a conversion between an unsigned integer and an enum type.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed
 * between pointer to void and an arithmetic type.
 * The base address parameter from HAL functions is provided as integer so
 * it needs to be cast to pointer.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, A conversion should not be performed
 * between a pointer to object and an integer type.
 * The base address parameter from HAL functions is provided as integer so
 * a conversion between a pointer and an integer has to be performed
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
* @section [global]
 * Violates MISRA 2012 Required Rule 13.5, side effects on right hand of logical operator, ''&&''
 * Those functions do not have side effects.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.3, controlling expressions shall not be invariant.
 * Code must address all devices.
 */

#include "device_registers.h"
#include "sim_hw_access.h"
#include "scg_hw_access.h"
#include "pcc_hw_access.h"
#include "pmc_hw_access.h"
#include "smc_hw_access.h"
#include "clock.h"
#include "interrupt_manager.h"
#include <stddef.h>   /* This header is included for bool type */
/*
 * README:
 * This file provides these APIs:
 * 1. APIs to get the frequency of output clocks in Reference Manual ->
 * Chapter Clock Distribution -> Figure Clocking diagram.
 * 2. APIs for IP modules listed in Reference Manual -> Chapter Clock Distribution
 * -> Module clocks.
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

static clock_manager_state_t g_clockState;

/* This frequency values should be set by different boards. */
/* SIM */
uint32_t g_TClkFreq[NUMBER_OF_TCLK_INPUTS];      /* TCLKx clocks    */

/* RTC */
uint32_t g_RtcClkInFreq;                         /* RTC CLKIN clock */

/* SCG */
uint32_t g_xtal0ClkFreq;                         /* EXTAL0 clock    */


#if FEATURE_HAS_SPLL_CLK
/* @brief System PLL base multiplier value, it is the multiplier value when SCG_SPLLCFG[MULT]=0. */
#define SCG_SPLL_MULT_BASE 16U

/*
 * @brief System PLL base divider value, it is the PLL reference clock divider value when
 * SCG_SPLLCFG[PREDIV]=0.
 */
#define SCG_SPLL_PREDIV_BASE 1U

/*
 * @brief System PLL reference clock after SCG_SPLLCFG[PREDIV] should be in the range of
 * SCG_SPLL_REF_MIN to SCG_SPLL_REF_MAX.
 */
#define SCG_SPLL_REF_MIN 8000000U

/*
 * @brief System PLL reference clock after SCG_SPLLCFG[PREDIV] should be in the range of
 * SCG_SPLL_REF_MIN to SCG_SPLL_REF_MAX.
 */
#define SCG_SPLL_REF_MAX 32000000U
#endif

/*
 * @brief LPO 128K fixed clock frequency.
 */
#define LPO_128K_FREQUENCY 128000UL

/*
 * @brief LPO 32K fixed clock frequency.
 */
#define LPO_32K_FREQUENCY 32000UL

/*
 * @brief LPO 1K fixed clock frequency.
 */
#define LPO_1K_FREQUENCY 1000UL

/*
 * @brief Running modes.
 */
#define HIGH_SPEED_RUNNING_MODE (1UL << 7U)
#define RUN_SPEED_RUNNING_MODE  (1UL << 0U)
#define VLPR_SPEED_RUNNING_MODE (1UL << 2U)


#define MODES_MAX_NO 7U
#define CLOCK_MAX_FREQUENCIES_VLPR_MODE                                                                    \
{/* SYS_CLK    BUS_CLK     SLOW_CLK */                                                                     \
{       0UL,       0UL,         0UL},               /*!< Invalid entry */                                  \
{ 4000000UL, 4000000UL,   1000000UL},               /*!< Maximum frequencies when system clock is SOSC */  \
{ 4000000UL, 4000000UL,   1000000UL},               /*!< Maximum frequencies when system clock is SIRC */  \
{ 4000000UL, 4000000UL,   1000000UL},               /*!< Maximum frequencies when system clock is FIRC */  \
{       0UL,       0UL,         0UL},               /*!< Invalid entry */                                  \
{       0UL,       0UL,         0UL},               /*!< Invalid entry */                                  \
{ 4000000UL, 4000000UL,   1000000UL},               /*!< Maximum frequencies when system clock is SPLL */  \
}
#define CLOCK_MAX_FREQUENCIES_RUN_MODE                                                                     \
{/* SYS_CLK    BUS_CLK     SLOW_CLK */                                                                     \
{       0UL,        0UL,         0UL},              /*!< Invalid entry */                                  \
{80000000UL, 48000000UL,  26670000UL},              /*!< Maximum frequencies when system clock is SOSC */  \
{80000000UL, 48000000UL,  26670000UL},              /*!< Maximum frequencies when system clock is SIRC */  \
{80000000UL, 48000000UL,  26670000UL},              /*!< Maximum frequencies when system clock is FIRC */  \
{       0UL,        0UL,         0UL},              /*!< Invalid entry */                                  \
{       0UL,        0UL,         0UL},              /*!< Invalid entry */                                  \
{80000000UL, 40000000UL,  26670000UL},              /*!< Maximum frequencies when system clock is SPLL */  \
}
#if FEATURE_HAS_HIGH_SPEED_RUN_MODE
#define CLOCK_MAX_FREQUENCIES_HSRUN_MODE                                                                    \
{/* SYS_CLK    BUS_CLK     SLOW_CLK */                                                                      \
{        0UL,        0UL,         0UL},              /*!< Invalid entry */                                  \
{112000000UL, 56000000UL,  28000000UL},              /*!< Maximum frequencies when system clock is SOSC */  \
{112000000UL, 56000000UL,  28000000UL},              /*!< Maximum frequencies when system clock is SIRC */  \
{112000000UL, 56000000UL,  28000000UL},              /*!< Maximum frequencies when system clock is FIRC */  \
{        0UL,        0UL,         0UL},              /*!< Invalid entry */                                  \
{        0UL,        0UL,         0UL},              /*!< Invalid entry */                                  \
{112000000UL, 56000000UL,  28000000UL},              /*!< Maximum frequencies when system clock is SPLL */  \
}
#endif

/*
 * @brief Number of peripheral clocks.
 */
#if defined(PCC_FTFC_INDEX)
    #define TMP_FTFC 1U
#else
    #define TMP_FTFC 0U
#endif
#if defined(PCC_DMAMUX_INDEX)
    #define TMP_DMAMUX 1U
#else
    #define TMP_DMAMUX 0U
#endif
#if defined(PCC_FlexCAN0_INDEX)
    #define TMP_FlexCAN0 1U
#else
    #define TMP_FlexCAN0 0U
#endif
#if defined(PCC_FlexCAN1_INDEX)
    #define TMP_FlexCAN1 1U
#else
    #define TMP_FlexCAN1 0U
#endif
#if defined(PCC_FTM3_INDEX)
    #define TMP_FTM3 1U
#else
    #define TMP_FTM3 0U
#endif
#if defined(PCC_ADC1_INDEX)
    #define TMP_ADC1 1U
#else
    #define TMP_ADC1 0U
#endif
#if defined(PCC_FlexCAN2_INDEX)
    #define TMP_FlexCAN2 1U
#else
    #define TMP_FlexCAN2 0U
#endif
#if defined(PCC_LPSPI0_INDEX)
    #define TMP_LPSPI0 1U
#else
    #define TMP_LPSPI0 0U
#endif
#if defined(PCC_LPSPI1_INDEX)
    #define TMP_LPSPI1 1U
#else
    #define TMP_LPSPI1 0U
#endif
#if defined(PCC_LPSPI2_INDEX)
    #define TMP_LPSPI2 1U
#else
    #define TMP_LPSPI2 0U
#endif
#if defined(PCC_PDB1_INDEX)
    #define TMP_PDB1 1U
#else
    #define TMP_PDB1 0U
#endif
#if defined(PCC_CRC_INDEX)
    #define TMP_CRC 1U
#else
    #define TMP_CRC 0U
#endif
#if defined(PCC_PDB0_INDEX)
    #define TMP_PDB0 1U
#else
    #define TMP_PDB0 0U
#endif
#if defined(PCC_LPIT_INDEX)
    #define TMP_LPIT 1U
#else
    #define TMP_LPIT 0U
#endif
#if defined(PCC_FTM0_INDEX)
    #define TMP_FTM0 1U
#else
    #define TMP_FTM0 0U
#endif
#if defined(PCC_FTM1_INDEX)
    #define TMP_FTM1 1U
#else
    #define TMP_FTM1 0U
#endif
#if defined(PCC_FTM2_INDEX)
    #define TMP_FTM2 1U
#else
    #define TMP_FTM2 0U
#endif
#if defined(PCC_ADC0_INDEX)
    #define TMP_ADC0 1U
#else
    #define TMP_ADC0 0U
#endif
#if defined(PCC_RTC_INDEX)
    #define TMP_RTC 1U
#else
    #define TMP_RTC 0U
#endif
#if defined(PCC_LPTMR0_INDEX)
    #define TMP_LPTMR0 1U
#else
    #define TMP_LPTMR0 0U
#endif
#if defined(PCC_PORTA_INDEX)
    #define TMP_PORTA 1U
#else
    #define TMP_PORTA 0U
#endif
#if defined(PCC_PORTB_INDEX)
    #define TMP_PORTB 1U
#else
    #define TMP_PORTB 0U
#endif
#if defined(PCC_PORTC_INDEX)
    #define TMP_PORTC 1U
#else
    #define TMP_PORTC 0U
#endif
#if defined(PCC_PORTD_INDEX)
    #define TMP_PORTD 1U
#else
    #define TMP_PORTD 0U
#endif
#if defined(PCC_PORTE_INDEX)
    #define TMP_PORTE 1U
#else
    #define TMP_PORTE 0U
#endif
#if defined(PCC_SAI0_INDEX)
    #define TMP_SAI0 1U
#else
    #define TMP_SAI0 0U
#endif
#if defined(PCC_SAI1_INDEX)
    #define TMP_SAI1 1U
#else
    #define TMP_SAI1 0U
#endif
#if defined(PCC_FlexIO_INDEX)
    #define TMP_FlexIO 1U
#else
    #define TMP_FlexIO 0U
#endif
#if defined(PCC_EWM_INDEX)
    #define TMP_EWM 1U
#else
    #define TMP_EWM 0U
#endif
#if defined(PCC_LPI2C0_INDEX)
    #define TMP_LPI2C0 1U
#else
    #define TMP_LPI2C0 0U
#endif
#if defined(PCC_LPI2C1_INDEX)
    #define TMP_LPI2C1 1U
#else
    #define TMP_LPI2C1 0U
#endif
#if defined(PCC_LPUART0_INDEX)
    #define TMP_LPUART0 1U
#else
    #define TMP_LPUART0 0U
#endif
#if defined(PCC_LPUART1_INDEX)
    #define TMP_LPUART1 1U
#else
    #define TMP_LPUART1 0U
#endif
#if defined(PCC_LPUART2_INDEX)
    #define TMP_LPUART2 1U
#else
    #define TMP_LPUART2 0U
#endif
#if defined(PCC_FTM4_INDEX)
    #define TMP_FTM4 1U
#else
    #define TMP_FTM4 0U
#endif
#if defined(PCC_FTM5_INDEX)
    #define TMP_FTM5 1U
#else
    #define TMP_FTM5 0U
#endif
#if defined(PCC_FTM6_INDEX)
    #define TMP_FTM6 1U
#else
    #define TMP_FTM6 0U
#endif
#if defined(PCC_FTM7_INDEX)
    #define TMP_FTM7 1U
#else
    #define TMP_FTM7 0U
#endif
#if defined(PCC_CMP0_INDEX)
    #define TMP_CMP0 1U
#else
    #define TMP_CMP0 0U
#endif
#if defined(PCC_QSPI_INDEX)
    #define TMP_QSPI 1U
#else
    #define TMP_QSPI 0U
#endif
#if defined(PCC_ENET_INDEX)
    #define TMP_ENET 1U
#else
    #define TMP_ENET 0U
#endif

#define CLOCK_PERIPHERALS_COUNT (TMP_FTFC + TMP_DMAMUX + TMP_FlexCAN0 + TMP_FlexCAN1 + TMP_FTM3 + TMP_ADC1 + TMP_FlexCAN2 + TMP_LPSPI0 + TMP_LPSPI1 + TMP_LPSPI2 + TMP_PDB1 + TMP_CRC + TMP_PDB0 + TMP_LPIT + TMP_FTM0 + TMP_FTM1 + TMP_FTM2 + TMP_ADC0 + TMP_RTC + TMP_LPTMR0 + TMP_PORTA + TMP_PORTB + TMP_PORTC + TMP_PORTD + TMP_PORTE + TMP_SAI0 + TMP_SAI1 + TMP_FlexIO + TMP_EWM + TMP_LPI2C0 + TMP_LPI2C1 + TMP_LPUART0 + TMP_LPUART1 + TMP_LPUART2 + TMP_FTM4 + TMP_FTM5 + TMP_FTM6 + TMP_FTM7 + TMP_CMP0 + TMP_QSPI + TMP_ENET)


/*! @brief Clock name mappings
 *         Constant array storing the mappings between clock names and peripheral clock control indexes.
 *         If there is no peripheral clock control index for a clock name, then the corresponding value is
 *         PCC_INVALID_INDEX.
 */
const uint16_t clockNameMappings[] = PCC_CLOCK_NAME_MAPPINGS;

/*! @brief Peripheral features list
 *         Constant array storing the mappings between clock names of the peripherals and feature lists.
 */
const uint8_t peripheralFeaturesList[] = PERIPHERAL_FEATURES;

/*!
 * @brief SCG system clock type.
 * Implements scg_system_clock_type_t_Class
 */
typedef enum
{
    SCG_SYSTEM_CLOCK_CORE,  /*!< Core clock.        */
    SCG_SYSTEM_CLOCK_BUS,   /*!< BUS clock.         */
    SCG_SYSTEM_CLOCK_SLOW,  /*!< System slow clock. */
    SCG_SYSTEM_CLOCK_MAX,   /*!< Max value.         */
} scg_system_clock_type_t;

/*!
 * @brief SCG asynchronous clock type.
 * Implements scg_async_clock_type_t_Class
 */
typedef enum
{
    SCG_ASYNC_CLOCK_DIV1   = 0U,   /*!< Clock divider 1  */
    SCG_ASYNC_CLOCK_DIV2   = 1U,   /*!< Clock divider 2  */
    SCG_ASYNC_CLOCK_MAX    = 2U,   /*!< Max value.       */
} scg_async_clock_type_t;

/*!
 * @brief SCG system clock modes.
 * Implements scg_system_clock_mode_t_Class
 */
typedef enum
{
    SCG_SYSTEM_CLOCK_MODE_CURRENT = 0U,  /*!< Current mode.            */
    SCG_SYSTEM_CLOCK_MODE_RUN     = 1U,  /*!< Run mode.                */
    SCG_SYSTEM_CLOCK_MODE_VLPR    = 2U,  /*!< Very Low Power Run mode. */
    SCG_SYSTEM_CLOCK_MODE_HSRUN   = 3U,  /*!< High Speed Run mode.     */
    SCG_SYSTEM_CLOCK_MODE_NONE           /*!< MAX value.               */
} scg_system_clock_mode_t;


/*******************************************************************************
 * INTERNAL FUNCTIONS
 ******************************************************************************/
static void CLOCK_SYS_GetDefaultConfiguration(clock_manager_user_config_t * config);

static status_t CLOCK_SYS_GetScgClockFreq(clock_names_t clockName,
                                          uint32_t * frequency);

#ifdef QuadSPI_INSTANCE_COUNT
static uint32_t CLOCK_SYS_GetQSPIInternalReferenceClock(void);

static uint32_t CLOCK_SYS_GetQspiSfifClkHyp(void);

static uint32_t CLOCK_SYS_GetQspiIpgClk(void);

static uint32_t CLOCK_SYS_GetQspiIpgClkSfif(void);

static uint32_t CLOCK_SYS_GetQspiIpgClk2Xsfif(void);
#endif

static status_t CLOCK_SYS_GetSimClockFreq(clock_names_t clockName,
                                          uint32_t * frequency);

static status_t CLOCK_SYS_GetPccClockFreq(clock_names_t clockName,
                                          uint32_t * frequency);

static uint32_t CLOCK_SYS_GetPeripheralClock(clock_names_t clockName,
                                             scg_async_clock_type_t divider);

static scg_system_clock_mode_t CLOCK_SYS_GetCurrentRunMode(void);

static status_t CLOCK_SYS_TransitionSystemClock(const scg_system_clock_config_t * to_clk);

static uint32_t CLOCK_SYS_GetSimClkOutFreq(void);

static uint32_t CLOCK_SYS_GetScgClkOutFreq(void);

static uint32_t CLOCK_SYS_GetSimRtcClkFreq(void);

static status_t CLOCK_SYS_ConfigureTemporarySystemClock(void);

static status_t CLOCK_SYS_ConfigureModulesFromScg(const scg_config_t * scgConfig);

static status_t CLOCK_SYS_ConfigureSIRC(bool enable, const scg_sirc_config_t * sircConfig);

static status_t CLOCK_SYS_ConfigureFIRC(bool enable, const scg_firc_config_t * fircConfig);

static status_t CLOCK_SYS_ConfigureSOSC(bool enable, const scg_sosc_config_t * soscConfig);

#if FEATURE_HAS_SPLL_CLK
static status_t CLOCK_SYS_ConfigureSPLL(bool enable, const scg_spll_config_t * spllConfig);
#endif

static uint32_t CLOCK_SYS_GetSystemClockFreq(scg_system_clock_type_t type);

static status_t CLOCK_SYS_SetSystemClockConfig(scg_system_clock_mode_t mode,
                                               scg_system_clock_config_t const * config);

static uint32_t CLOCK_SYS_GetSysAsyncFreq(clock_names_t clockSource,
                                          scg_async_clock_type_t type);

static uint32_t CLOCK_SYS_GetSircFreq(void);

static uint32_t CLOCK_SYS_GetFircFreq(void);

static uint32_t CLOCK_SYS_GetSysOscFreq(void);

#if FEATURE_HAS_SPLL_CLK
static uint32_t CLOCK_SYS_GetSysPllFreq(void);
#endif

static uint32_t CLOCK_SYS_GetLpoFreq(void);

static status_t CLOCK_SYS_TransitionToTmpSysClk(scg_system_clock_src_t currentSysClkSrc);

static void CLOCK_SYS_GetCurrentSysClkConfig(scg_system_clock_config_t * sysClockConfig);

static status_t CLOCK_SYS_SetScgConfiguration(const scg_config_t * scgConfig);

static void CLOCK_SYS_SetPccConfiguration(const pcc_config_t * peripheralClockConfig);

static void CLOCK_SYS_SetSimConfiguration(const sim_clock_config_t * simClockConfig);

static void CLOCK_SYS_SetPmcConfiguration(const pmc_config_t * pmcConfig);

static scg_async_clock_div_t CLOCK_SYS_ConvertAsyncDividerValue(uint16_t divider);

static status_t CLOCK_SYS_SetSircClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2);

static status_t CLOCK_SYS_SetFircClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2);

static status_t CLOCK_SYS_SetSoscClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2);

#if FEATURE_HAS_SPLL_CLK
static status_t CLOCK_SYS_SetSpllClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2);
#endif

static scg_sosc_ext_ref_t CLOCK_SYS_GetExtRef(xosc_ref_t refClk);

static clock_names_t CLOCK_SYS_GetDefaultModuleClkCfgSource(void);

static scg_system_clock_mode_t CLOCK_SYS_GetSysClockMode(const pwr_modes_t mode);

static scg_system_clock_src_t CLOCK_SYS_GetSysClockSource(clock_names_t src);

static uint32_t CLOCK_SYS_GetSrcFreq(scg_system_clock_src_t src);

static status_t CLOCK_SYS_CheckPCCClock(clock_names_t clockName);

static void CLOCK_SYS_SetClockGate(clock_names_t peripheralClock, bool gating);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_DRV_Init
 * Description   : This function sets the system to target configuration, it
 * only sets the clock modules registers for clock mode change, but not send
 * notifications to drivers.
 *
 * Implements CLOCK_DRV_Init_Activity
 * END**************************************************************************/
status_t CLOCK_DRV_Init(clock_manager_user_config_t const * config)
{
    status_t result;
    clock_manager_user_config_t config_default;
    clock_manager_user_config_t const * cfg = config;

    DEV_ASSERT(CLOCK_SYS_GetCurrentRunMode() == SCG_SYSTEM_CLOCK_MODE_RUN);

    if (config == NULL)
    {
        /* Get default configuration */
        CLOCK_SYS_GetDefaultConfiguration(&config_default);
        cfg = &config_default;
    }

    /* Set SCG settings. */
    result = CLOCK_SYS_SetScgConfiguration(&cfg->scgConfig);

    if (STATUS_SUCCESS == result)
    {
        /* Set PCC settings. */
        CLOCK_SYS_SetPccConfiguration(&cfg->pccConfig);

        /* Set SIM settings. */
        CLOCK_SYS_SetSimConfiguration(&cfg->simConfig);

        /* Set PMC settings. */
        CLOCK_SYS_SetPmcConfiguration(&cfg->pmcConfig);
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_SetScgConfiguration
 * Description   : This function configures the SCG blocks
 *
 * END**************************************************************************/
static status_t CLOCK_SYS_SetScgConfiguration(const scg_config_t * scgConfig)
{
    status_t status = STATUS_SUCCESS;
    DEV_ASSERT(scgConfig != NULL);

    if (scgConfig != NULL)
    {
        /* Configure a temporary system clock source: FIRC */
        status = CLOCK_SYS_ConfigureTemporarySystemClock();

        if (status == STATUS_SUCCESS)
        {
            /* Configure clock sources from SCG */
            status = CLOCK_SYS_ConfigureModulesFromScg(scgConfig);
        }

        if (status == STATUS_SUCCESS)
        {
            /* Configure RTC. */
            if (scgConfig->rtcConfig.initialize)
            {
                /* RTC Clock settings. */
                g_RtcClkInFreq = scgConfig->rtcConfig.rtcClkInFreq;
            }

            /* Configure SCG ClockOut. */
            if (scgConfig->clockOutConfig.initialize)
            {
                /* ClockOut settings. */
                SCG_SetClockoutSourceSel(SCG, (uint32_t)scgConfig->clockOutConfig.source);
            }

            /* Configure SCG clock modes. */
            if (scgConfig->clockModeConfig.initialize)
            {
                /* Configure SCG clock modes */
                status = CLOCK_SYS_SetSystemClockConfig(SCG_SYSTEM_CLOCK_MODE_RUN, &(scgConfig->clockModeConfig.rccrConfig));
                if (status == STATUS_SUCCESS)
                {
                    status = CLOCK_SYS_SetSystemClockConfig(SCG_SYSTEM_CLOCK_MODE_VLPR, &(scgConfig->clockModeConfig.vccrConfig));
                }

#if FEATURE_HAS_HIGH_SPEED_RUN_MODE
                if (status == STATUS_SUCCESS)
                {
                    status = CLOCK_SYS_SetSystemClockConfig(SCG_SYSTEM_CLOCK_MODE_HSRUN, &(scgConfig->clockModeConfig.hccrConfig));
                }
#endif
            }
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_SetPccConfiguration
 * Description   : This function configures the PCC block
 *
 * END**************************************************************************/
static void CLOCK_SYS_SetPccConfiguration(const pcc_config_t * peripheralClockConfig)
{
    DEV_ASSERT(peripheralClockConfig != NULL);

    uint32_t i;

    if ((peripheralClockConfig != NULL) && (peripheralClockConfig->peripheralClocks != NULL))
    {
        for (i = 0U; i < peripheralClockConfig->count; i++)
        {
            /* Disable the peripheral clock */
            PCC_SetClockMode(PCC, peripheralClockConfig->peripheralClocks[i].clockName, false);

            /* Set peripheral clock control */
            PCC_SetPeripheralClockControl(PCC,
                                          peripheralClockConfig->peripheralClocks[i].clockName,
                                          peripheralClockConfig->peripheralClocks[i].clkGate,
                                          (uint32_t)peripheralClockConfig->peripheralClocks[i].clkSrc,
                                          (uint32_t)peripheralClockConfig->peripheralClocks[i].divider,
                                          (uint32_t)peripheralClockConfig->peripheralClocks[i].frac);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_SetSimConfiguration
 * Description   : This function configures the SIM block
 *
 * END**************************************************************************/
static void CLOCK_SYS_SetSimConfiguration(const sim_clock_config_t * simClockConfig)
{
    DEV_ASSERT(simClockConfig != NULL);
    uint8_t i;

    /* ClockOut settings. */
    if (simClockConfig->clockOutConfig.initialize)
    {
        SIM_SetClockout(SIM, simClockConfig->clockOutConfig.enable, simClockConfig->clockOutConfig.source, simClockConfig->clockOutConfig.divider);
    }

    /* Low Power Clock settings from SIM. */
    if (simClockConfig->lpoClockConfig.initialize)
    {
        SIM_SetLpoClocks(SIM,
                         simClockConfig->lpoClockConfig.enableLpo1k,
                         simClockConfig->lpoClockConfig.enableLpo32k,
                         simClockConfig->lpoClockConfig.sourceLpoClk,
                         simClockConfig->lpoClockConfig.sourceRtcClk);
    }

    /* Platform Gate Clock settings. */
    if (simClockConfig->platGateConfig.initialize)
    {
        SIM_SetMscmClockGate(SIM, simClockConfig->platGateConfig.enableMscm);
        SIM_SetMpuClockGate(SIM, simClockConfig->platGateConfig.enableMpu);
        SIM_SetDmaClockGate(SIM, simClockConfig->platGateConfig.enableDma);
        SIM_SetErmClockGate(SIM, simClockConfig->platGateConfig.enableErm);
        SIM_SetEimClockGate(SIM, simClockConfig->platGateConfig.enableEim);
#if defined (QuadSPI_INSTANCE_COUNT)
        SIM_SetQspiIntRefClockGate(SIM, simClockConfig->qspiRefClkGating.enableQspiRefClk);
#endif
    }

    /* TCLK Clock settings. */
    if (simClockConfig->tclkConfig.initialize)
    {
        for (i = 0; i < NUMBER_OF_TCLK_INPUTS; i++)
        {
            if (i < NUMBER_OF_TCLK_INPUTS)
            {
                g_TClkFreq[i] = simClockConfig->tclkConfig.tclkFreq[i];
            }
        }
    }

    /* Debug trace Clock settings. */
    if (simClockConfig->traceClockConfig.initialize)
    {
        SIM_ClearTraceClockConfig(SIM);
        SIM_SetTraceClockSource(SIM, simClockConfig->traceClockConfig.source);

        SIM_SetTraceClockConfig(SIM, false, 0U, 0U);

        if (simClockConfig->traceClockConfig.divEnable)
        {
            SIM_SetTraceClockConfig(SIM,
                                    simClockConfig->traceClockConfig.divEnable,
                                    simClockConfig->traceClockConfig.divider,
                                    simClockConfig->traceClockConfig.divFraction);
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_SetPmcConfiguration
 * Description   : This function configures the PMC block
 *
 * END**************************************************************************/
static void CLOCK_SYS_SetPmcConfiguration(const pmc_config_t * pmcConfig)
{
    DEV_ASSERT(pmcConfig != NULL);

    /* Low Power Clock settings from PMC. */
    if (pmcConfig->lpoClockConfig.initialize)
    {
        /* Enable/disable the low power oscillator. */
        PMC_SetLpoMode(PMC, pmcConfig->lpoClockConfig.enable);

        /* Write trimming value. */
        PMC_SetLpoTrimValue(PMC, pmcConfig->lpoClockConfig.trimValue);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetDefaultConfiguration
 * Description   : This function gets the system to a default configuration, it
 * only gets the clock modules registers for clock mode change, but not send
 * notifications to drivers.
 *
 * END**************************************************************************/
static void CLOCK_SYS_GetDefaultConfiguration(clock_manager_user_config_t * config)
{
    static peripheral_clock_config_t peripheralClockConfig[CLOCK_PERIPHERALS_COUNT] = {
#ifdef PCC_ADC0_INDEX
        {
            .clockName        = ADC0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_ADC1_INDEX
        {
            .clockName        = ADC1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_CMP0_INDEX
        {
            .clockName        = CMP0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_CRC_INDEX
        {
            .clockName        = CRC0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_DMAMUX_INDEX
        {
            .clockName        = DMAMUX0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_ENET_INDEX
        {
            .clockName        = ENET0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_EWM_INDEX
        {
            .clockName        = EWM0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FlexCAN0_INDEX
        {
            .clockName        = FlexCAN0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FlexCAN1_INDEX
        {
            .clockName        = FlexCAN1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FlexCAN2_INDEX
        {
            .clockName        = FlexCAN2_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FlexIO_INDEX
        {
            .clockName        = FLEXIO0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTFC_INDEX
        {
            .clockName        = FTFC0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM0_INDEX
        {
            .clockName        = FTM0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM1_INDEX
        {
            .clockName        = FTM1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM2_INDEX
        {
            .clockName        = FTM2_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM3_INDEX
        {
            .clockName        = FTM3_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM4_INDEX
        {
            .clockName        = FTM4_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM5_INDEX
        {
            .clockName        = FTM5_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM6_INDEX
        {
            .clockName        = FTM6_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_FTM7_INDEX
        {
            .clockName        = FTM7_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPI2C0_INDEX
        {
            .clockName        = LPI2C0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPI2C1_INDEX
        {
            .clockName        = LPI2C1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPIT_INDEX
        {
            .clockName        = LPIT0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPSPI0_INDEX
        {
            .clockName        = LPSPI0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPSPI1_INDEX
        {
            .clockName        = LPSPI1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPSPI2_INDEX
        {
            .clockName        = LPSPI2_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPTMR0_INDEX
        {
            .clockName        = LPTMR0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPUART0_INDEX
        {
            .clockName        = LPUART0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPUART1_INDEX
        {
            .clockName        = LPUART1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_LPUART2_INDEX
        {
            .clockName        = LPUART2_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_SIRC_DIV1,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PDB0_INDEX
        {
            .clockName        = PDB0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PDB1_INDEX
        {
            .clockName        = PDB1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PORTA_INDEX
        {
            .clockName        = PORTA_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PORTB_INDEX
        {
            .clockName        = PORTB_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PORTC_INDEX
        {
            .clockName        = PORTC_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PORTD_INDEX
        {
            .clockName        = PORTD_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_PORTE_INDEX
        {
            .clockName        = PORTE_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_QSPI_INDEX
        {
            .clockName        = QSPI0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_RTC_INDEX
        {
            .clockName        = RTC0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_SAI0_INDEX
        {
            .clockName        = SAI0_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
#ifdef PCC_SAI1_INDEX
        {
            .clockName        = SAI1_CLK,
            .clkGate          = true,
            .clkSrc           = CLK_SRC_OFF,
            .frac             = MULTIPLY_BY_ONE,
            .divider          = DIVIDE_BY_ONE,
        },
#endif
    };

    /* SCG */
    config->scgConfig.sircConfig.initialize               = true;                               /*!< Initialize */
    /* SIRCCSR */
    config->scgConfig.sircConfig.enableInStop             = false;                              /*!< SIRCSTEN  */
    config->scgConfig.sircConfig.enableInLowPower         = true;                               /*!< SIRCLPEN  */
    config->scgConfig.sircConfig.locked                   = false;                              /*!< LK        */
    /* SIRCCFG */
    config->scgConfig.sircConfig.range                    = SCG_SIRC_RANGE_HIGH;                /*!< RANGE - High range (8 MHz) */
    /* SIRCDIV */
    config->scgConfig.sircConfig.div1                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< SIRCDIV1  */
    config->scgConfig.sircConfig.div2                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< SIRCDIV2  */

    config->scgConfig.fircConfig.initialize               = true;                               /*!< Initialize */
     /* FIRCCSR */
    config->scgConfig.fircConfig.regulator                = true;                               /*!< FIRCREGOFF */
    config->scgConfig.fircConfig.locked                   = false;                              /*!< LK         */
    /* FIRCCFG */
    config->scgConfig.fircConfig.range                    = SCG_FIRC_RANGE_48M;                 /*!< RANGE      */
    /* FIRCDIV */
    config->scgConfig.fircConfig.div1                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< FIRCDIV1   */
    config->scgConfig.fircConfig.div2                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< FIRCDIV2   */

    config->scgConfig.rtcConfig.initialize                = true;                               /*!< Initialize  */
    config->scgConfig.rtcConfig.rtcClkInFreq              = 0U;                                 /*!< RTC_CLKIN   */

    config->scgConfig.soscConfig.initialize               = true;                               /*!< Initialize */
    config->scgConfig.soscConfig.freq                     = 8000000U;                           /*!< Frequency  */
    /* SOSCCSR */
    config->scgConfig.soscConfig.monitorMode              = SCG_SOSC_MONITOR_DISABLE;           /*!< SOSCCM      */
    config->scgConfig.soscConfig.locked                   = false;                              /*!< LK          */
    /* SOSCCFG */
    config->scgConfig.soscConfig.extRef                   = SCG_SOSC_REF_OSC;                   /*!< EREFS       */
    config->scgConfig.soscConfig.gain                     = SCG_SOSC_GAIN_LOW;                  /*!< HGO         */
    config->scgConfig.soscConfig.range                    = SCG_SOSC_RANGE_MID;                 /*!< RANGE       */
    /* SOSCDIV */
    config->scgConfig.soscConfig.div1                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< SOSCDIV1    */
    config->scgConfig.soscConfig.div2                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< SOSCDIV2    */

    config->scgConfig.spllConfig.initialize               = true;                               /*!< Initialize */
    /* SPLLCSR */
    config->scgConfig.spllConfig.monitorMode              = SCG_SPLL_MONITOR_DISABLE;           /*!< SPLLCM     */
    config->scgConfig.spllConfig.locked                   = false;                              /*!< LK         */
    /* SPLLCFG */
    config->scgConfig.spllConfig.prediv                   = SCG_SPLL_CLOCK_PREDIV_BY_1;         /*!< PREDIV     */
    config->scgConfig.spllConfig.mult                     = SCG_SPLL_CLOCK_MULTIPLY_BY_28;      /*!< MULT       */
    config->scgConfig.spllConfig.src                      = 0U;                                 /*!< SOURCE     */
    /* SPLLDIV */
    config->scgConfig.spllConfig.div1                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< SPLLDIV1   */
    config->scgConfig.spllConfig.div2                     = SCG_ASYNC_CLOCK_DIV_BY_1;           /*!< SPLLDIV2   */

    config->scgConfig.clockOutConfig.initialize           = true;                               /*!< Initialize    */
    config->scgConfig.clockOutConfig.source               = SCG_CLOCKOUT_SRC_FIRC;              /*!< SCG CLKOUTSEL     */

    config->scgConfig.clockModeConfig.initialize          = true;                               /*!< Initialize */
    /*!< RCCR - Run Clock Control Register          */
    config->scgConfig.clockModeConfig.rccrConfig.src      = SCG_SYSTEM_CLOCK_SRC_FIRC;          /*!< SCS        */
    config->scgConfig.clockModeConfig.rccrConfig.divCore  = SCG_SYSTEM_CLOCK_DIV_BY_1;          /*!< DIVCORE    */
    config->scgConfig.clockModeConfig.rccrConfig.divBus   = SCG_SYSTEM_CLOCK_DIV_BY_2;          /*!< DIVBUS     */
    config->scgConfig.clockModeConfig.rccrConfig.divSlow  = SCG_SYSTEM_CLOCK_DIV_BY_2;          /*!< DIVSLOW    */
    /*!< VCCR - VLPR Clock Control Register        */
    config->scgConfig.clockModeConfig.vccrConfig.src      = SCG_SYSTEM_CLOCK_SRC_SIRC;          /*!< SCS        */
    config->scgConfig.clockModeConfig.vccrConfig.divCore  = SCG_SYSTEM_CLOCK_DIV_BY_2;          /*!< DIVCORE    */
    config->scgConfig.clockModeConfig.vccrConfig.divBus   = SCG_SYSTEM_CLOCK_DIV_BY_1;          /*!< DIVBUS     */
    config->scgConfig.clockModeConfig.vccrConfig.divSlow  = SCG_SYSTEM_CLOCK_DIV_BY_4;          /*!< DIVSLOW    */

#if FEATURE_HAS_HIGH_SPEED_RUN_MODE
    /*!< HCCR - HSRUN Clock Control Register        */
    config->scgConfig.clockModeConfig.hccrConfig.src      = SCG_SYSTEM_CLOCK_SRC_SYS_PLL;       /*!< SCS        */
    config->scgConfig.clockModeConfig.hccrConfig.divCore  = SCG_SYSTEM_CLOCK_DIV_BY_1;          /*!< DIVCORE    */
    config->scgConfig.clockModeConfig.hccrConfig.divBus   = SCG_SYSTEM_CLOCK_DIV_BY_2;          /*!< DIVBUS     */
    config->scgConfig.clockModeConfig.hccrConfig.divSlow  = SCG_SYSTEM_CLOCK_DIV_BY_4;          /*!< DIVSLOW    */
#endif

    /* PCC */
    config->pccConfig.peripheralClocks                    = peripheralClockConfig;             /*!< Peripheral clock control configurations  */
    config->pccConfig.count                               = CLOCK_PERIPHERALS_COUNT;         /*!< Number of the peripheral clock control configurations  */

    /* SIM */
    /*!< Clock Out configuration.           */
    config->simConfig.clockOutConfig.initialize           = true;                               /*!< Initialize    */
    config->simConfig.clockOutConfig.enable               = false;                              /*!< CLKOUTEN      */
    config->simConfig.clockOutConfig.source               = SIM_CLKOUT_SEL_SYSTEM_SCG_CLKOUT;   /*!< CLKOUTSEL     */
    config->simConfig.clockOutConfig.divider              = SIM_CLKOUT_DIV_BY_1;                /*!< CLKOUTDIV     */
    /*!< Low Power Clock configuration.     */
    config->simConfig.lpoClockConfig.initialize           = true;                               /*!< Initialize    */
    config->simConfig.lpoClockConfig.enableLpo1k          = true;                               /*!< LPO1KCLKEN    */
    config->simConfig.lpoClockConfig.enableLpo32k         = true;                               /*!< LPO32KCLKEN   */
    config->simConfig.lpoClockConfig.sourceLpoClk         = SIM_LPO_CLK_SEL_LPO_128K;           /*!< LPOCLKSEL     */
    config->simConfig.lpoClockConfig.sourceRtcClk         = SIM_RTCCLK_SEL_SOSCDIV1_CLK;        /*!< RTCCLKSEL     */
    /*!< Platform Gate Clock configuration. */
    config->simConfig.platGateConfig.initialize           = true;                               /*!< Initialize    */
    config->simConfig.platGateConfig.enableMscm           = true;                               /*!< CGCMSCM       */
    config->simConfig.platGateConfig.enableMpu            = true;                               /*!< CGCMPU       */
    config->simConfig.platGateConfig.enableDma            = true;                               /*!< CGCDMA       */
    config->simConfig.platGateConfig.enableErm            = true;                               /*!< CGCERM       */
    config->simConfig.platGateConfig.enableEim            = true;                               /*!< CGCMEIM       */
    /*!< Quad Spi Internal Reference Clock Gating. */
    config->simConfig.qspiRefClkGating.enableQspiRefClk   = true;                               /*!< Qspi reference clock gating    */
    /*!< TCLK CLOCK configuration. */
    config->simConfig.tclkConfig.initialize               = true;                               /*!< Initialize    */
    config->simConfig.tclkConfig.tclkFreq[0]              = 0U;                                 /*!< TCLK0         */
    config->simConfig.tclkConfig.tclkFreq[1]              = 0U;                                 /*!< TCLK0         */
    config->simConfig.tclkConfig.tclkFreq[2]              = 0U;                                 /*!< TCLK0         */
     /*!< Debug trace Clock Configuration. */
    config->simConfig.traceClockConfig.initialize         = true;                               /*!< Initialize    */
    config->simConfig.traceClockConfig.divEnable          = true;                               /*!< TRACEDIVEN    */
    config->simConfig.traceClockConfig.source             = CLOCK_TRACE_SRC_CORE_CLK;           /*!< TRACECLK_SEL  */
    config->simConfig.traceClockConfig.divider            = 0U;                                 /*!< TRACEDIV      */
    config->simConfig.traceClockConfig.divFraction        = false;                              /*!< TRACEFRAC     */

    /* PMC */
    /*!< Low Power Clock configuration.     */
    config->pmcConfig.lpoClockConfig.initialize           = true;                               /*!< Initialize             */
    config->pmcConfig.lpoClockConfig.enable               = true;                               /*!< Enable/disable LPO     */
    config->pmcConfig.lpoClockConfig.trimValue            = 0;                                  /*!< Trimming value for LPO */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetScgClockFreq
 * Description   : This function returns the frequency of a given clock from SCG
 *
 * END**************************************************************************/
static status_t CLOCK_SYS_GetScgClockFreq(clock_names_t clockName,
                                          uint32_t * frequency)
{
    status_t returnCode = STATUS_SUCCESS;
    uint32_t freq = 0U;

    switch (clockName)
    {
        /* Main clocks */
        case CORE_CLK:
            freq = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_CORE);
            break;
        case BUS_CLK:
            freq = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_BUS);
            break;
        case SLOW_CLK:
            freq = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_SLOW);
            break;
        case CLKOUT_CLK:
            freq = CLOCK_SYS_GetSimClkOutFreq();
            break;

        /* Other internal clocks used by peripherals. */
        case SIRC_CLK:
            freq = CLOCK_SYS_GetSircFreq();
            break;
        case FIRC_CLK:
            freq = CLOCK_SYS_GetFircFreq();
            break;
        case SOSC_CLK:
            freq = CLOCK_SYS_GetSysOscFreq();
            break;
#if FEATURE_HAS_SPLL_CLK
        case SPLL_CLK:
            freq = CLOCK_SYS_GetSysPllFreq();
            break;
#endif
        case RTC_CLKIN_CLK:
            freq = g_RtcClkInFreq;
            break;
        case SCG_CLKOUT_CLK:
            freq = CLOCK_SYS_GetScgClkOutFreq();
            break;
        case SIRCDIV1_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(SIRC_CLK, SCG_ASYNC_CLOCK_DIV1);
            break;
        case SIRCDIV2_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(SIRC_CLK, SCG_ASYNC_CLOCK_DIV2);
            break;
        case FIRCDIV1_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(FIRC_CLK, SCG_ASYNC_CLOCK_DIV1);
            break;
        case FIRCDIV2_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(FIRC_CLK, SCG_ASYNC_CLOCK_DIV2);
            break;
        case SOSCDIV1_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(SOSC_CLK, SCG_ASYNC_CLOCK_DIV1);
            break;
        case SOSCDIV2_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(SOSC_CLK, SCG_ASYNC_CLOCK_DIV2);
            break;
#if FEATURE_HAS_SPLL_CLK
        case SPLLDIV1_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(SPLL_CLK, SCG_ASYNC_CLOCK_DIV1);
            break;
        case SPLLDIV2_CLK:
            freq = CLOCK_SYS_GetSysAsyncFreq(SPLL_CLK, SCG_ASYNC_CLOCK_DIV2);
            break;
#endif
        default:
            returnCode = STATUS_UNSUPPORTED;
            break;
    }

    if (frequency != NULL)
    {
        *frequency = freq;
    }

    return returnCode;
}

#ifdef QuadSPI_INSTANCE_COUNT
static uint32_t CLOCK_SYS_GetQSPIInternalReferenceClock(void)
{
    uint32_t freq = 0U;
    uint32_t divValue = 0U;

    if (SIM_GetClockingModeSelection(SIM))
    {
	    if (QSPI_GetClockingModeSelection(QuadSPI))
		{
            freq = CLOCK_SYS_GetSysAsyncFreq(FIRC_CLK, SCG_ASYNC_CLOCK_DIV1);
        }
        else
        {
            freq = CLOCK_SYS_GetSysAsyncFreq(SPLL_CLK, SCG_ASYNC_CLOCK_DIV1);
        }

	    divValue = QSPI_GetClockingProgrammableDividerValue(QuadSPI);
        freq /= (divValue + 1U);
    }

    return freq;
}

static uint32_t CLOCK_SYS_GetQspiSfifClkHyp(void)
{
    uint32_t freq = 0U;

    freq = CLOCK_SYS_GetQSPIInternalReferenceClock();
    freq >>= 1U;

    return freq;
}

static uint32_t CLOCK_SYS_GetQspiIpgClk(void)
{
    uint32_t freq = 0U;

    if (PCC_GetClockMode(PCC, QSPI0_CLK))
    {
	    if (QSPI_GetClockingModeSelection(QuadSPI))
	    {
		    freq = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_BUS);
	    }
	    else
	    {
		    freq = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_CORE);
	    }
    }

    return freq;
}

static uint32_t CLOCK_SYS_GetQspiIpgClkSfif(void)
{
	uint32_t freq = 0U;

	freq = CLOCK_SYS_GetQSPIInternalReferenceClock();

	if (QSPI_GetClockingModeSelection(QuadSPI))
	    {
	        freq >>= 1U;
	    }

    return freq;
}

static uint32_t CLOCK_SYS_GetQspiIpgClk2Xsfif(void)
{
    uint32_t freq = 0U;

    if (QSPI_GetClockingModeSelection(QuadSPI))
    {
        freq = CLOCK_SYS_GetQSPIInternalReferenceClock();
    }

    return freq;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetSimClockFreq
 * Description   : This function returns the frequency of a given clock from SIM
 *
 * END**************************************************************************/
static status_t CLOCK_SYS_GetSimClockFreq(clock_names_t clockName,
                                          uint32_t * frequency)
{
    status_t returnCode = STATUS_SUCCESS;
    uint32_t clockPinSelect;
    uint32_t freq = 0U;

    switch (clockName)
    {
        /* SIM clocks */
        case SIM_FTM0_CLOCKSEL:
            clockPinSelect = SIM_GetFtm0ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
        case SIM_FTM1_CLOCKSEL:
            clockPinSelect = SIM_GetFtm1ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#if FTM_INSTANCE_COUNT > 2U
        case SIM_FTM2_CLOCKSEL:
            clockPinSelect = SIM_GetFtm2ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#endif
#if FTM_INSTANCE_COUNT > 3U
        case SIM_FTM3_CLOCKSEL:
            clockPinSelect = SIM_GetFtm3ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#endif
#if FTM_INSTANCE_COUNT > 4U
        case SIM_FTM4_CLOCKSEL:
            clockPinSelect = SIM_GetFtm4ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#endif
#if FTM_INSTANCE_COUNT > 5U
        case SIM_FTM5_CLOCKSEL:
            clockPinSelect = SIM_GetFtm5ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#endif
#if FTM_INSTANCE_COUNT > 6U
        case SIM_FTM6_CLOCKSEL:
            clockPinSelect = SIM_GetFtm6ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#endif
#if FTM_INSTANCE_COUNT > 7U
        case SIM_FTM7_CLOCKSEL:
            clockPinSelect = SIM_GetFtm7ExternalClkPinMode(SIM);
            if (clockPinSelect < NUMBER_OF_TCLK_INPUTS)
            {
                freq = g_TClkFreq[clockPinSelect];
            }

            break;
#endif
        case SIM_CLKOUTSELL:
            freq = CLOCK_SYS_GetSimClkOutFreq();
            break;
        case SIM_RTCCLK_CLK:
            freq = CLOCK_SYS_GetSimRtcClkFreq();
            break;
        case SIM_LPO_CLK:
            if (PMC_GetLpoMode(PMC))
            {
                freq = CLOCK_SYS_GetLpoFreq();
            }

            break;
        case SIM_LPO_1K_CLK:
            if (PMC_GetLpoMode(PMC))
            {
                freq = ((SIM_GetLpo32KStatus(SIM)) && (SIM_GetLpo1KStatus(SIM))) ? LPO_1K_FREQUENCY : 0UL;
            }

            break;
        case SIM_LPO_32K_CLK:
            if (PMC_GetLpoMode(PMC))
            {
                freq = SIM_GetLpo32KStatus(SIM) ? LPO_32K_FREQUENCY : 0UL;
            }

            break;
        case SIM_LPO_128K_CLK:
            if (PMC_GetLpoMode(PMC))
            {
                freq = LPO_128K_FREQUENCY;
            }

            break;
        case SIM_EIM_CLK:
            if (!SIM_GetEimClockGate(SIM))
            {
                /* EIM is not clocked. */
                returnCode = STATUS_MCU_GATED_OFF;
            }

            break;
        case SIM_ERM_CLK:
            if (!SIM_GetErmClockGate(SIM))
            {
                /* ERM is not clocked. */
                returnCode = STATUS_MCU_GATED_OFF;
            }

            break;
        case SIM_DMA_CLK:
            if (!SIM_GetDmaClockGate(SIM))
            {
                /* DMA is not clocked. */
                returnCode = STATUS_MCU_GATED_OFF;
            }

            break;
        case SIM_MPU_CLK:
            if (!SIM_GetMpuClockGate(SIM))
            {
                /* MPU is not clocked. */
                returnCode = STATUS_MCU_GATED_OFF;
            }

            break;
        case SIM_MSCM_CLK:
            if (!SIM_GetMscmClockGate(SIM))
            {
                /* MSCM is not clocked. */
                returnCode = STATUS_MCU_GATED_OFF;
            }

            break;
#ifdef QuadSPI_INSTANCE_COUNT
        case QSPI_MODULE_SFIF_CLK_HYP:
        	freq = CLOCK_SYS_GetQspiSfifClkHyp();
        	break;

        case QSPI_MODULE_CLK:
            freq = CLOCK_SYS_GetQspiIpgClk();
        	break;

        case QSPI_MODULE_CLK_SFIF:
        	freq = CLOCK_SYS_GetQspiIpgClkSfif();
        	break;

        case QSPI_MODULE_CLK_2XSFIF:
        	freq = CLOCK_SYS_GetQspiIpgClk2Xsfif();
        	break;
#endif
        default:
            returnCode = STATUS_UNSUPPORTED;
            break;
    }

    if (frequency != NULL)
    {
        *frequency = freq;
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_CheckPCCClock
 * Description   : Checks if PCC clock name is valid and if the module is enabled.
 * END**************************************************************************/
static status_t CLOCK_SYS_CheckPCCClock(clock_names_t clockName)
{
	status_t returnCode = STATUS_SUCCESS;

	/* Invalid PCC clock names. */
	if ((clockName <= SIM_END_OF_CLOCKS) ||
		(clockName == PCC_END_OF_BUS_CLOCKS) ||
		(clockName == PCC_END_OF_SYS_CLOCKS) ||
		(clockName == PCC_END_OF_SLOW_CLOCKS) ||
		(clockName == PCC_END_OF_ASYNCH_DIV1_CLOCKS) ||
		(clockName == PCC_END_OF_ASYNCH_DIV2_CLOCKS))
	{
		returnCode = STATUS_UNSUPPORTED;
	}
	else if (PCC_GetClockMode(PCC, clockName) == false)
	{
		/* Module is not clocked. */
		returnCode = STATUS_MCU_GATED_OFF;
	}
	else
	{
		returnCode = STATUS_SUCCESS;
	}

	return returnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetPccClockFreq
 * Description   : This function returns the clock frequency of peripheral functional clock.
 * END**************************************************************************/
static status_t CLOCK_SYS_GetPccClockFreq(clock_names_t clockName,
                                          uint32_t * frequency)
{
    status_t returnCode = STATUS_SUCCESS;
    uint32_t freq = 0U;

    returnCode = CLOCK_SYS_CheckPCCClock(clockName);

    if (returnCode == STATUS_SUCCESS)
    {
        if ((peripheralFeaturesList[clockName] & HAS_INT_CLOCK_FROM_BUS_CLOCK) != 0U)
        {
            /* Check whether BUS CLOCK is clocked. */
            returnCode = (status_t)((CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_BUS) == 0U) ? STATUS_MCU_GATED_OFF : STATUS_SUCCESS);
        }
        else if ((peripheralFeaturesList[clockName] & HAS_INT_CLOCK_FROM_SYS_CLOCK) != 0U)
        {
            /* Check whether SYS CLOCK is clocked. */
            returnCode = (status_t)((CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_CORE) == 0U) ? STATUS_MCU_GATED_OFF : STATUS_SUCCESS);
        }
        else if ((peripheralFeaturesList[clockName] & HAS_INT_CLOCK_FROM_SLOW_CLOCK) != 0U)
        {
            /* Check whether SLOW CLOCK is clocked. */
            returnCode = (status_t)((CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_SLOW) == 0U) ? STATUS_MCU_GATED_OFF : STATUS_SUCCESS);
        }
        else
        {   /* It's an issue in peripheral features list, each peripheral must have one interface clock. */
            DEV_ASSERT(false);
        }

        if (returnCode == STATUS_SUCCESS)
        {
            /* Check whether peripheral has protocol clock (functional clock) */
            if ((peripheralFeaturesList[clockName] & (HAS_PROTOCOL_CLOCK_FROM_ASYNC1 | HAS_PROTOCOL_CLOCK_FROM_ASYNC2)) != 0U)
            {
                if ((peripheralFeaturesList[clockName] & HAS_PROTOCOL_CLOCK_FROM_ASYNC1) != 0U)
                {
                    /* Check whether the functional clock is clocked */
                    freq = CLOCK_SYS_GetPeripheralClock(clockName, SCG_ASYNC_CLOCK_DIV1);
                }

                if ((peripheralFeaturesList[clockName] & HAS_PROTOCOL_CLOCK_FROM_ASYNC2) != 0U)
                {
                    /* Check whether the functional clock is clocked */
                    freq = CLOCK_SYS_GetPeripheralClock(clockName, SCG_ASYNC_CLOCK_DIV2);
                }

                if (freq == 0U)
                {
                    returnCode = STATUS_MCU_GATED_OFF;
                }

            }
        }



    }

    /* If frequency reference is provided, write this value */
    if (frequency != NULL)
    {
        *frequency = freq;
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_DRV_GetFreq
 * Description   : This function returns the frequency of a given clock
 *
 * Implements CLOCK_DRV_GetFreq_Activity
 * END**************************************************************************/
status_t CLOCK_DRV_GetFreq(clock_names_t clockName,
                           uint32_t * frequency)
{
    status_t returnCode;

    /* Frequency of the clock name from SCG */
    if (clockName < SCG_END_OF_CLOCKS)
    {
        returnCode = CLOCK_SYS_GetScgClockFreq(clockName, frequency);
    }
    /* Frequency of the clock name from SIM */
    else if (clockName < SIM_END_OF_CLOCKS)
    {
        returnCode = CLOCK_SYS_GetSimClockFreq(clockName, frequency);
    }
    /* Frequency of the clock name from PCC */
    else if (clockName < PCC_END_OF_CLOCKS)
    {
        returnCode = CLOCK_SYS_GetPccClockFreq(clockName, frequency);
    }
    /* Invalid clock name */
    else
    {
        returnCode = STATUS_UNSUPPORTED;
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetPeripheralClock
 * Description   : Internal function used by CLOCK_SYS_GetFreq function
 * END**************************************************************************/

static uint32_t CLOCK_SYS_GetPeripheralClock(clock_names_t clockName,
                                             scg_async_clock_type_t divider)
{
    uint32_t frequency = 0;
    uint32_t fracValue = PCC_GetFracValueSel(PCC, clockName);
    uint32_t divValue = PCC_GetDividerSel(PCC, clockName);

    /* Check division factor */
    if (((uint32_t)fracValue) <= ((uint32_t)divValue))
    {
        /* Check clock gate */
        if (PCC_GetClockMode(PCC, clockName))
        {
            /* Check clock source */
            switch (PCC_GetClockSourceSel(PCC, clockName))
            {
                case (uint32_t)CLK_SRC_SOSC:
                    frequency = CLOCK_SYS_GetSysAsyncFreq(SOSC_CLK, divider);
                    break;
                case (uint32_t)CLK_SRC_SIRC:
                    frequency = CLOCK_SYS_GetSysAsyncFreq(SIRC_CLK, divider);
                    break;
                case (uint32_t)CLK_SRC_FIRC:
                    frequency = CLOCK_SYS_GetSysAsyncFreq(FIRC_CLK, divider);
                    break;
#if FEATURE_HAS_SPLL_CLK
                case (uint32_t)CLK_SRC_SPLL:
                    frequency = CLOCK_SYS_GetSysAsyncFreq(SPLL_CLK, divider);
                    break;
#endif
                default:
                    frequency = 0;
                    break;
            }

            frequency = frequency / (divValue + 1U);
            frequency = frequency * (fracValue + 1U);
        }
    }

    return frequency;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetCurrentRunMode
 * Description   : Internal function used by CLOCK_SYS_SetScgConfiguration function
 * END**************************************************************************/
static scg_system_clock_mode_t CLOCK_SYS_GetCurrentRunMode(void)
{
    scg_system_clock_mode_t mode;

    /* Get the current running mode */
    switch (SMC_GetCurrentRunningMode(SMC))
    {
        /* High speed run mode */
        case HIGH_SPEED_RUNNING_MODE:
            mode = SCG_SYSTEM_CLOCK_MODE_HSRUN;
            break;
        /* Run mode */
        case RUN_SPEED_RUNNING_MODE:
            mode = SCG_SYSTEM_CLOCK_MODE_RUN;
            break;
        /* Very low power run mode */
        case VLPR_SPEED_RUNNING_MODE:
            mode = SCG_SYSTEM_CLOCK_MODE_VLPR;
            break;
        /* This should never happen - core has to be in some run mode to execute code */
        default:
            mode = SCG_SYSTEM_CLOCK_MODE_NONE;
            break;
    }

    return mode;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_TransitionSystemClock
 * Description   : Internal function used by CLOCK_SYS_ConfigureTemporarySystemClock and
 * CLOCK_SYS_ConfigureModulesFromScg functions
 * END**************************************************************************/
static status_t CLOCK_SYS_TransitionSystemClock(const scg_system_clock_config_t * to_clk)
{
    scg_system_clock_mode_t run_mode;
    status_t retValue = STATUS_SUCCESS;
    uint32_t timeout;

    /* Check destination clock */
    DEV_ASSERT(to_clk != NULL);
    DEV_ASSERT(to_clk->src != SCG_SYSTEM_CLOCK_SRC_NONE);

    /* Get & Convert Run mode from SMC to SCG defines*/
    run_mode = CLOCK_SYS_GetCurrentRunMode();

    /* Check the current mode */
    DEV_ASSERT(run_mode != SCG_SYSTEM_CLOCK_MODE_NONE);

    /* Update run mode configuration */
    retValue = CLOCK_SYS_SetSystemClockConfig(run_mode, to_clk);

    if (retValue == STATUS_SUCCESS)
    {
        /* Wait for system clock to transition. */
#ifdef ERRATA_E10777
        timeout = 10U;
#else
        timeout = 1U;
#endif

        do
        {
            timeout--;
        }
        while ((SCG_GetCurrentSystemClockSource(SCG) != ((uint32_t)to_clk->src)) && (timeout > 0U));

        if (timeout == 0U)
        {
            retValue = STATUS_TIMEOUT;
        }
    }

    return retValue;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSimClkOutFreq
 * Description   : Internal function used by CLOCK_SYS_GetFreq function
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSimClkOutFreq(void)
{
    uint32_t frequency;

    if (SIM_GetClockoutStatus(SIM))
    {
        switch (SIM_GetClockoutSelectorValue(SIM))
        {
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_SCG_CLKOUT):
                frequency = CLOCK_SYS_GetScgClkOutFreq();
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_SOSC_DIV2_CLK):
                frequency = CLOCK_SYS_GetSysAsyncFreq(SOSC_CLK, SCG_ASYNC_CLOCK_DIV2);
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_SIRC_DIV2_CLK):
                frequency = CLOCK_SYS_GetSysAsyncFreq(SIRC_CLK, SCG_ASYNC_CLOCK_DIV2);
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_FIRC_DIV2_CLK):
                frequency = CLOCK_SYS_GetSysAsyncFreq(FIRC_CLK, SCG_ASYNC_CLOCK_DIV2);
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_HCLK):
                frequency = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_CORE);
                break;
#if FEATURE_HAS_SPLL_CLK
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_SPLL_DIV2_CLK):
                frequency = CLOCK_SYS_GetSysAsyncFreq(SPLL_CLK, SCG_ASYNC_CLOCK_DIV2);
                break;
#endif
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_BUS_CLK):
                frequency = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_BUS);
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_LPO_128K_CLK):
                frequency = LPO_128K_FREQUENCY;
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_LPO_CLK):
                frequency = CLOCK_SYS_GetLpoFreq();
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_RTC_CLK):
                frequency = CLOCK_SYS_GetSimRtcClkFreq();
                break;
#ifdef QuadSPI_INSTANCE_COUNT
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_SFIF_CLK_HYP):
                frequency = CLOCK_SYS_GetQspiSfifClkHyp();
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_IPG_CLK):
                frequency = CLOCK_SYS_GetQspiIpgClk();
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_IPG_CLK_SFIF):
                frequency = CLOCK_SYS_GetQspiIpgClkSfif();
                break;
            case ((uint32_t)SIM_CLKOUT_SEL_SYSTEM_IPG_CLK_2XSFIF):
                frequency = CLOCK_SYS_GetQspiIpgClk2Xsfif();
                break;
#endif
            default:
                /* Invalid SIM CLKOUT selection.*/
                frequency = 0U;
                break;
        }

        /* Apply Divide Ratio */
        frequency /= (SIM_GetClockoutDividerValue(SIM) + 1U);
    }
    else
    {
        /* Output disabled. */
        frequency = 0U;
    }

    return frequency;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetScgClkOutFreq
 * Description   : Internal function used by CLOCK_SYS_GetFreq function
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetScgClkOutFreq(void)
{
    uint32_t frequency;

    switch (SCG_GetClockoutSourceSel(SCG))
    {
        case ((uint32_t)SCG_CLOCKOUT_SRC_SCG_SLOW):
            frequency = CLOCK_SYS_GetSystemClockFreq(SCG_SYSTEM_CLOCK_SLOW);
            break;
        case ((uint32_t)SCG_CLOCKOUT_SRC_SOSC):
            frequency = CLOCK_SYS_GetSysOscFreq();
            break;
        case ((uint32_t)SCG_CLOCKOUT_SRC_SIRC):
            frequency = CLOCK_SYS_GetSircFreq();
            break;
        case ((uint32_t)SCG_CLOCKOUT_SRC_FIRC):
            frequency = CLOCK_SYS_GetFircFreq();
            break;
#if FEATURE_HAS_SPLL_CLK
        case ((uint32_t)SCG_CLOCKOUT_SRC_SPLL):
            frequency = CLOCK_SYS_GetSysPllFreq();
            break;
#endif
        default:
            /* Invalid SCG CLKOUT selection.*/
            frequency = 0U;
            break;
    }

    return frequency;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSimRtcClkFreq
 * Description   : Internal function used by CLOCK_SYS_GetFreq function
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSimRtcClkFreq()
{
    uint32_t frequency;

    /* Check RTCCLK Select */
    switch (SIM_GetRtcClkSrc(SIM))
    {
        case ((uint32_t)SIM_RTCCLK_SEL_SOSCDIV1_CLK):
            frequency = CLOCK_SYS_GetSysAsyncFreq(SOSC_CLK, SCG_ASYNC_CLOCK_DIV1);
            break;
        case ((uint32_t)SIM_RTCCLK_SEL_LPO_32K):
            frequency = SIM_GetLpo32KStatus(SIM) ? LPO_32K_FREQUENCY : 0UL;
            break;
        case ((uint32_t)SIM_RTCCLK_SEL_RTC_CLKIN):
            frequency = g_RtcClkInFreq;
            break;
        case ((uint32_t)SIM_RTCCLK_SEL_FIRCDIV1_CLK):
            frequency = CLOCK_SYS_GetSysAsyncFreq(FIRC_CLK, SCG_ASYNC_CLOCK_DIV1);
            break;
        default:
            /* Invalid RTCCLK selection.*/
            frequency = 0U;
            break;
    }

    return frequency;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConfigureSIRC
 * Description   : Configures SIRC module based on provided configuration.
 * END**************************************************************************/
static status_t CLOCK_SYS_ConfigureSIRC(bool enable, const scg_sirc_config_t * sircConfig)
{
    status_t status = STATUS_SUCCESS;
    scg_sirc_config_t sircDefaultConfig;
    const scg_sirc_config_t * sircCfg;
    uint32_t timeout;

    if (sircConfig == NULL)
    {
        sircDefaultConfig.enableInStop      = false;
        sircDefaultConfig.enableInLowPower  = true;
        sircDefaultConfig.locked            = false;

        sircDefaultConfig.div1              = SCG_ASYNC_CLOCK_DIV_BY_1;
        sircDefaultConfig.div2              = SCG_ASYNC_CLOCK_DIV_BY_1;

        sircDefaultConfig.range             = SCG_SIRC_RANGE_HIGH;

        sircCfg = &sircDefaultConfig;
    }
    else
    {
        sircCfg = sircConfig;
    }

    /* If clock is used by system, return error. */
    if (SCG_GetSircSystemClockMode(SCG))
    {
        status = STATUS_BUSY;
    }
    /* Disable SIRC */
    else
    {
        /* Clear LK bit field */
        SCG_ClearSircLock(SCG);

        /* Disable monitor, disable clock and clear error. */
        SCG_ClearSircControl(SCG);
    }

    /* Configure SIRC. */
    if (enable  && (status == STATUS_SUCCESS))
    {
        /* Now start to set up SIRC clock. */
        /* Step 1. Setup dividers. */
        SCG_SetSircAsyncConfig(SCG, sircCfg->div1, sircCfg->div2);

        /* Step 2. Set SIRC configuration: frequency range. */
        SCG_SetSircConfiguration(SCG, sircCfg->range);

        /* Step 3. Set SIRC control: enable clock, configure source in STOP and VLP modes, configure lock feature. */
        SCG_SetSircControl(SCG, sircCfg->enableInStop, sircCfg->enableInLowPower, sircCfg->locked);

        /* Wait for SIRC to initialize */
        timeout = SIRC_STABILIZATION_TIMEOUT;
        while ((CLOCK_SYS_GetSircFreq() == 0U) && (timeout > 0U))
        {
            timeout--;
        }

        if (timeout == 0U)
        {
            status = STATUS_TIMEOUT;
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConfigureFIRC
 * Description   : Configures FIRC module based on provided configuration.
 * END**************************************************************************/
static status_t CLOCK_SYS_ConfigureFIRC(bool enable, const scg_firc_config_t * fircConfig)
{
    status_t status = STATUS_SUCCESS;
    scg_firc_config_t fircDefaultConfig;
    const scg_firc_config_t * fircCfg;
    uint32_t timeout;

    if (fircConfig == NULL)
    {
        fircDefaultConfig.regulator         = true;
        fircDefaultConfig.locked            = false;

        fircDefaultConfig.div1              = SCG_ASYNC_CLOCK_DIV_BY_1;
        fircDefaultConfig.div2              = SCG_ASYNC_CLOCK_DIV_BY_1;

        fircDefaultConfig.range             = SCG_FIRC_RANGE_48M;

        fircCfg = &fircDefaultConfig;
    }
    else
    {
        fircCfg = fircConfig;
    }

    /* If clock is used by system, return error. */
    if (SCG_GetFircSystemClockMode(SCG))
    {
        status = STATUS_BUSY;
    }
    /* Disable FIRC */
    else
    {
        /* Clear LK bit field */
        SCG_ClearFircLock(SCG);

        /* Disable monitor, disable clock and clear error. */
        SCG_ClearFircControl(SCG);
    }

    /* Configure FIRC. */
    if (enable && (status == STATUS_SUCCESS))
    {
        /* Now start to set up FIRC clock. */
        /* Step 1. Setup dividers. */
        SCG_SetFircAsyncConfig(SCG, fircCfg->div1, fircCfg->div2);

        /* Step 2. Set FIRC configuration. */
        SCG_SetFircConfiguration(SCG, fircCfg->range);

        /* Step 3. Enable clock, config regulator and locking feature. */
        SCG_SetFircControl(SCG, fircCfg->regulator, fircCfg->locked);

        /* Wait for FIRC to initialize */
        timeout = FIRC_STABILIZATION_TIMEOUT;
        while ((CLOCK_SYS_GetFircFreq() == 0U) && (timeout > 0U))
        {
            timeout--;
        }

        if (timeout == 0U)
        {
            status = STATUS_TIMEOUT;
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConfigureSOSC
 * Description   : Configures SOSC module based on provided configuration.
 * END**************************************************************************/
static status_t CLOCK_SYS_ConfigureSOSC(bool enable, const scg_sosc_config_t * soscConfig)
{
    status_t status = STATUS_SUCCESS;
    scg_sosc_config_t soscDefaultConfig;
    const scg_sosc_config_t * soscCfg;
    uint32_t timeout;

    if (soscConfig == NULL)
    {
        soscDefaultConfig.monitorMode       = SCG_SOSC_MONITOR_DISABLE;
        soscDefaultConfig.locked            = false;

        soscDefaultConfig.div1              = SCG_ASYNC_CLOCK_DIV_BY_1;
        soscDefaultConfig.div2              = SCG_ASYNC_CLOCK_DIV_BY_1;

        soscDefaultConfig.extRef            = SCG_SOSC_REF_OSC;
        soscDefaultConfig.gain              = SCG_SOSC_GAIN_LOW;
        soscDefaultConfig.range             = SCG_SOSC_RANGE_MID;

        soscDefaultConfig.freq              = 8000000U;

        soscCfg = &soscDefaultConfig;
    }
    else
    {
        soscCfg = soscConfig;
    }

    /* If clock is used by system, return error. */
    if (SCG_GetSoscSystemClockMode(SCG))
    {
        status = STATUS_BUSY;
    }
    /* Disable SOSC */
    else
    {
        /* Clear LK bit field */
        SCG_ClearSoscLock(SCG);

        /* Disable monitor, disable clock and clear error. */
        SCG_ClearSoscControl(SCG);

        g_xtal0ClkFreq = 0U;
    }

    /* Configure SOSC. */
    if (enable && (status == STATUS_SUCCESS))
    {
        /* Now start to set up OSC clock. */
        /* Step 1. Setup dividers. */
        SCG_SetSoscAsyncConfig(SCG, soscCfg->div1, soscCfg->div2);

        /* Step 2. Set OSC configuration. */
        SCG_SetSoscConfiguration(SCG, soscCfg->range, soscCfg->gain, soscCfg->extRef);

        /* Step 3. Enable clock, configure monitor, lock register. */
        switch (soscCfg->monitorMode)
        {
            case SCG_SOSC_MONITOR_DISABLE:
            {
                SCG_SetSoscControl(SCG, false, false, soscCfg->locked);
            }
            break;
            case SCG_SOSC_MONITOR_INT:
            {
                SCG_SetSoscControl(SCG, true, false, soscCfg->locked);
            }
            break;
            case SCG_SOSC_MONITOR_RESET:
            {
                SCG_SetSoscControl(SCG, true, true, soscCfg->locked);
            }
            break;
            default:
                /* Invalid monitor mode */
                DEV_ASSERT(false);
                break;
        }

        g_xtal0ClkFreq = soscCfg->freq;

        /* Wait for System OSC to initialize */
        timeout = SOSC_STABILIZATION_TIMEOUT;
        while ((CLOCK_SYS_GetSysOscFreq() == 0U) && (timeout > 0U))
        {
            timeout--;
        }

        if (timeout == 0U)
        {
            status = STATUS_TIMEOUT;
        }
    }

    return status;
}

#if FEATURE_HAS_SPLL_CLK
/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConfigureSPLL
 * Description   : Configures SPLL module based on provided configuration.
 * END**************************************************************************/
static status_t CLOCK_SYS_ConfigureSPLL(bool enable, const scg_spll_config_t * spllConfig)
{
    status_t status = STATUS_SUCCESS;
    scg_spll_config_t spllDefaultConfig;
    const scg_spll_config_t * spllCfg;
    uint32_t srcFreq, timeout;

    if (spllConfig == NULL)
    {
        spllDefaultConfig.monitorMode    = SCG_SPLL_MONITOR_DISABLE;
        spllDefaultConfig.locked         = false;

        spllDefaultConfig.div1           = SCG_ASYNC_CLOCK_DIV_BY_1;
        spllDefaultConfig.div2           = SCG_ASYNC_CLOCK_DIV_BY_1;

        spllDefaultConfig.prediv         = 0;
        spllDefaultConfig.mult           = 7;
        spllDefaultConfig.src            = 0;

        spllCfg = &spllDefaultConfig;
    }
    else
    {
        spllCfg = spllConfig;
    }

    /* If clock is used by system, return error. */
    if (SCG_GetSpllSystemClockMode(SCG))
    {
        status = STATUS_BUSY;
    }
    /* Disable the SPLL. */
    else
    {
        /* Clear LK bit field */
        SCG_ClearSpllLock(SCG);

        /* Disable monitor, disable clock and clear error. */
        SCG_ClearSpllControl(SCG);
    }

    /* Configure SPLL. */
    if (enable && (status == STATUS_SUCCESS))
    {
        /* Get clock source frequency. */
        srcFreq = CLOCK_SYS_GetSysOscFreq();
        DEV_ASSERT(srcFreq != 0U);

        /* Pre-divider checking. */
        srcFreq /= (((uint32_t)spllCfg->prediv) + SCG_SPLL_PREDIV_BASE);
        DEV_ASSERT((srcFreq >= SCG_SPLL_REF_MIN) && (srcFreq <= SCG_SPLL_REF_MAX));

        /* Now start to set up PLL clock. */
        SCG_SetSpllAsyncConfig(SCG, spllCfg->div1, spllCfg->div2);

        /* Step 2. Set PLL configuration. */
        SCG_SetSpllConfiguration(SCG, spllCfg->prediv, spllCfg->mult);

        /* Step 3. Enable clock, configure monitor, lock register. */
        switch (spllCfg->monitorMode)
        {
            case SCG_SPLL_MONITOR_DISABLE:
            {
                SCG_SetSpllControl(SCG, false, false, spllCfg->locked);
            }
            break;
            case SCG_SPLL_MONITOR_INT:
            {
                SCG_SetSpllControl(SCG, true, false, spllCfg->locked);
            }
            break;
            case SCG_SPLL_MONITOR_RESET:
            {
                SCG_SetSpllControl(SCG, true, true, spllCfg->locked);
            }
            break;
            default:
                /* Invalid monitor mode */
                DEV_ASSERT(false);
                break;
        }

        /* Wait for System PLL to initialize */
        timeout = SPLL_STABILIZATION_TIMEOUT;
        while ((CLOCK_SYS_GetSysPllFreq() == 0U) && (timeout > 0U))
        {
            timeout--;
        }

        if (timeout == 0U)
        {
            status = STATUS_TIMEOUT;
        }
    }

    return status;
}
#endif

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConfigureTemporarySystemClock
 * Description   : Configures and transitions to a temporary system clock source: FIRC
 * END**************************************************************************/
static status_t CLOCK_SYS_ConfigureTemporarySystemClock(void)
{
    status_t status = STATUS_SUCCESS;
    scg_system_clock_config_t sysClockConfig;
    static const scg_system_clock_div_t tmpSysClk[TMP_SYS_CLK_NO][TMP_SYS_DIV_NO] = TMP_SYSTEM_CLOCK_CONFIGS;

    /* If the current system clock source is not FIRC:
     * 1. Enable FIRC (if it's not enabled)
     * 2. Switch to FIRC.
     */
    if (SCG_GetCurrentSystemClockSource(SCG) != ((uint32_t)SCG_SYSTEM_CLOCK_SRC_FIRC))
    {
        /* If FIRC is not on, then FIRC is configured
         * with the default configuration */
        if (CLOCK_SYS_GetFircFreq() == 0UL)
        {
            status = CLOCK_SYS_ConfigureFIRC(true, NULL);
        }

        /* FIRC is enabled, transition the system clock source to FIRC. */
        if (status == STATUS_SUCCESS)
        {
            sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_FIRC;
            sysClockConfig.divCore = tmpSysClk[TMP_FIRC_CLK][TMP_SYS_DIV];
            sysClockConfig.divBus  = tmpSysClk[TMP_FIRC_CLK][TMP_BUS_DIV];
            sysClockConfig.divSlow = tmpSysClk[TMP_FIRC_CLK][TMP_SLOW_DIV];
            status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConfigureModulesFromScg
 * Description   : Configures all modules from SCG (SIRC, FIRC, SOSC and SPLL)
 * END**************************************************************************/
static status_t CLOCK_SYS_ConfigureModulesFromScg(const scg_config_t * scgConfig)
{
    status_t status = STATUS_SUCCESS;
    scg_system_clock_config_t sysClockConfig;
    const scg_system_clock_config_t * nextSysClockConfig;
    static const scg_system_clock_div_t tmpSysClk[TMP_SYS_CLK_NO][TMP_SYS_DIV_NO] = TMP_SYSTEM_CLOCK_CONFIGS;

    /* Configure all clock sources that are different from the
     * current system clock source FIRC (SIRC, SOSC, SPLL). */
    status = CLOCK_SYS_ConfigureSIRC(scgConfig->sircConfig.initialize, &scgConfig->sircConfig);
    if (status == STATUS_SUCCESS)
    {
        status = CLOCK_SYS_ConfigureSOSC(scgConfig->soscConfig.initialize, &scgConfig->soscConfig);
#if FEATURE_HAS_SPLL_CLK
        if (status == STATUS_SUCCESS)
        {
            status = CLOCK_SYS_ConfigureSPLL(scgConfig->spllConfig.initialize,&scgConfig->spllConfig);
        }
#endif
    }

    /* Get the next system clock source */
    switch (CLOCK_SYS_GetCurrentRunMode())
    {
        case SCG_SYSTEM_CLOCK_MODE_RUN:
        {
            nextSysClockConfig = &scgConfig->clockModeConfig.rccrConfig;
        }
        break;
        case SCG_SYSTEM_CLOCK_MODE_VLPR:
        {
            nextSysClockConfig = &scgConfig->clockModeConfig.vccrConfig;
        }
        break;
#if FEATURE_HAS_HIGH_SPEED_RUN_MODE
        case SCG_SYSTEM_CLOCK_MODE_HSRUN:
        {
            nextSysClockConfig = &scgConfig->clockModeConfig.hccrConfig;
        }
        break;
#endif
        default:
            DEV_ASSERT(false);
            nextSysClockConfig = NULL;
            break;
    }

    if (status == STATUS_SUCCESS)
    {
        /* The current system clock source is FIRC.
         * Verify whether the next system clock source is FIRC. */
        if (nextSysClockConfig->src == SCG_SYSTEM_CLOCK_SRC_FIRC)
        {
            /* If they are the same, search for a temporary system clock source
             * (use one of the following sources: SPLL, SOSC, SIRC)
             * Assume that a temporary clock is not found status = ERROR. */
            status = STATUS_ERROR;

#if FEATURE_HAS_SPLL_CLK
            /* SPLL is enabled */
            if (scgConfig->spllConfig.initialize && (status == STATUS_ERROR))
            {
                sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SYS_PLL;
                sysClockConfig.divCore = tmpSysClk[TMP_SPLL_CLK][TMP_SYS_DIV];
                sysClockConfig.divBus  = tmpSysClk[TMP_SPLL_CLK][TMP_BUS_DIV];
                sysClockConfig.divSlow = tmpSysClk[TMP_SPLL_CLK][TMP_SLOW_DIV];
                status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
            }
#endif

            /* SOSC is enabled and SPLL configuration for system clock source is not valid */
            if (scgConfig->soscConfig.initialize && (status == STATUS_ERROR))
            {
                sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SYS_OSC;
                sysClockConfig.divCore = tmpSysClk[TMP_SOSC_CLK][TMP_SYS_DIV];
                sysClockConfig.divBus  = tmpSysClk[TMP_SOSC_CLK][TMP_BUS_DIV];
                sysClockConfig.divSlow = tmpSysClk[TMP_SOSC_CLK][TMP_SLOW_DIV];
                status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
            }


            /* SIRC is enabled and SOSC configuration for system clock source is not valid */
            if (scgConfig->sircConfig.initialize && (status == STATUS_ERROR))
            {
                sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SIRC;
                sysClockConfig.divCore = tmpSysClk[TMP_SIRC_CLK][TMP_SYS_DIV];
                sysClockConfig.divBus  = tmpSysClk[TMP_SIRC_CLK][TMP_BUS_DIV];
                sysClockConfig.divSlow = tmpSysClk[TMP_SIRC_CLK][TMP_SLOW_DIV];
                status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
            }

            /* Transitioned to a temporary system clock source. */
            if (status == STATUS_SUCCESS)
            {
                /* Configure the remaining clock source (FIRC). */
                status = CLOCK_SYS_ConfigureFIRC(scgConfig->fircConfig.initialize, &scgConfig->fircConfig);

                if (status == STATUS_SUCCESS)
                {
                    /* Transition to the next system clock source. */
                    sysClockConfig.src     = nextSysClockConfig->src;
                    sysClockConfig.divCore = nextSysClockConfig->divCore;
                    sysClockConfig.divBus  = nextSysClockConfig->divBus;
                    sysClockConfig.divSlow = nextSysClockConfig->divSlow;
                    status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
                }
            }
        }
        else
        {   /* Transition to the next system clock source. */
            sysClockConfig.src     = nextSysClockConfig->src;
            sysClockConfig.divCore = nextSysClockConfig->divCore;
            sysClockConfig.divBus  = nextSysClockConfig->divBus;
            sysClockConfig.divSlow = nextSysClockConfig->divSlow;
            status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);

            if (status == STATUS_SUCCESS)
            {
                /* Configure the remaining clock source (FIRC) */
                status = CLOCK_SYS_ConfigureFIRC(scgConfig->fircConfig.initialize, &scgConfig->fircConfig);
            }
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSystemClockFreq
 * Description   : This function gets the SCG system clock frequency, these
 * clocks are used for core, platform, external and bus clock domains.
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSystemClockFreq(scg_system_clock_type_t type)
{
    uint32_t freq;


    DEV_ASSERT(type < SCG_SYSTEM_CLOCK_MAX);

    switch (SCG_GetCurrentSystemClockSource(SCG))
    {
        case ((uint32_t)SCG_SYSTEM_CLOCK_SRC_SYS_OSC):
            freq = CLOCK_SYS_GetSysOscFreq();
            break;
        case ((uint32_t)SCG_SYSTEM_CLOCK_SRC_SIRC):
            freq = CLOCK_SYS_GetSircFreq();
            break;
        case ((uint32_t)SCG_SYSTEM_CLOCK_SRC_FIRC):
            freq = CLOCK_SYS_GetFircFreq();
            break;
#if FEATURE_HAS_SPLL_CLK
        case ((uint32_t)SCG_SYSTEM_CLOCK_SRC_SYS_PLL):
            freq = CLOCK_SYS_GetSysPllFreq();
            break;
#endif
        default:
            freq = 0U;
            break;
    }

    freq /= (SCG_GetCurrentCoreClockDividerRatio(SCG) + 1U);

    switch (type)
    {
        case SCG_SYSTEM_CLOCK_CORE:
            /* Intentionally left blank */
            break;
        case SCG_SYSTEM_CLOCK_BUS:
            freq /= (SCG_GetCurrentBusClockDividerRatio(SCG) + 1U);
            break;
        case SCG_SYSTEM_CLOCK_SLOW:
            freq /= (SCG_GetCurrentSlowClockDividerRatio(SCG) + 1U);
            break;
        default:
            freq = 0U;
            break;
    }

    return freq;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSrcFreq
 * Description   : Gets source frequency.
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSrcFreq(scg_system_clock_src_t src)
{
	uint32_t srcFreq = 0U;

	switch (src)
    {
        case SCG_SYSTEM_CLOCK_SRC_SYS_OSC:
            srcFreq = CLOCK_SYS_GetSysOscFreq();
            break;
        case SCG_SYSTEM_CLOCK_SRC_SIRC:
            srcFreq = CLOCK_SYS_GetSircFreq();
            break;
        case SCG_SYSTEM_CLOCK_SRC_FIRC:
            srcFreq = CLOCK_SYS_GetFircFreq();
            break;
#if FEATURE_HAS_SPLL_CLK
        case SCG_SYSTEM_CLOCK_SRC_SYS_PLL:
            srcFreq = CLOCK_SYS_GetSysPllFreq();
            break;
#endif
        default:
            srcFreq = 0U;
            break;
    }

	return srcFreq;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_SetSystemClockConfig
 * Description   : This function sets the system configuration for the specified mode.
 * END**************************************************************************/
static status_t CLOCK_SYS_SetSystemClockConfig(scg_system_clock_mode_t mode,
                                               scg_system_clock_config_t const * config)
{
    status_t status = STATUS_SUCCESS;
    uint32_t srcFreq = 0U;

    /* The maximum clock frequencies of system clocks in all power modes */
    static const uint32_t maxSysClksInVLPR[MODES_MAX_NO][SYS_CLK_MAX_NO]  = CLOCK_MAX_FREQUENCIES_VLPR_MODE;
    static const uint32_t maxSysClksInRUN[MODES_MAX_NO][SYS_CLK_MAX_NO]   = CLOCK_MAX_FREQUENCIES_RUN_MODE;
#if FEATURE_HAS_HIGH_SPEED_RUN_MODE
    static const uint32_t maxSysClksInHSRUN[MODES_MAX_NO][SYS_CLK_MAX_NO] = CLOCK_MAX_FREQUENCIES_HSRUN_MODE;
#endif
    const uint32_t sysFreqMul = ((uint32_t)config->divCore) + 1UL;
    const uint32_t busFreqMul = (((uint32_t)config->divCore) + 1UL) * (((uint32_t)config->divBus) + 1UL);
    const uint32_t slowFreqMul = (((uint32_t)config->divCore) + 1UL) * (((uint32_t)config->divSlow) + 1UL);

    DEV_ASSERT(mode != SCG_SYSTEM_CLOCK_MODE_CURRENT);

    srcFreq = CLOCK_SYS_GetSrcFreq(config->src);
    srcFreq >>= 4U;

    switch (mode)
    {
        case SCG_SYSTEM_CLOCK_MODE_RUN:       /*!< Run mode.                */
        	/* Verify the frequencies of sys, bus and slow clocks. */
            if ((srcFreq > (sysFreqMul  * (maxSysClksInRUN[(uint32_t)config->src][CORE_CLK_INDEX] >> 4U))) ||    /* Sys(core) clock */
                (srcFreq > (busFreqMul  * (maxSysClksInRUN[(uint32_t)config->src][BUS_CLK_INDEX] >> 4U)))  ||    /* Bus clock */
                (srcFreq > (slowFreqMul * (maxSysClksInRUN[(uint32_t)config->src][SLOW_CLK_INDEX] >> 4U))))      /* Slow clock */
            {
                /* Configuration for the next system clock source is not valid. */
                status = STATUS_ERROR;
            }
            else
            {
            	SCG_SetRunClockControl(SCG, (uint32_t)config->src, (uint32_t)config->divCore, (uint32_t)config->divBus, (uint32_t)config->divSlow);
            }
            break;
        case SCG_SYSTEM_CLOCK_MODE_VLPR:      /*!< Very Low Power Run mode. */
            DEV_ASSERT(SCG_SYSTEM_CLOCK_SRC_SIRC    == config->src);
        	/* Verify the frequencies of sys, bus and slow clocks. */
            if ((srcFreq > (sysFreqMul  * (maxSysClksInVLPR[(uint32_t)config->src][CORE_CLK_INDEX] >> 4U))) ||    /* Sys(core) clock */
                (srcFreq > (busFreqMul  * (maxSysClksInVLPR[(uint32_t)config->src][BUS_CLK_INDEX] >> 4U)))  ||    /* Bus clock */
                (srcFreq > (slowFreqMul * (maxSysClksInVLPR[(uint32_t)config->src][SLOW_CLK_INDEX] >> 4U))))      /* Slow clock */
            {
                /* Configuration for the next system clock source is not valid. */
                status = STATUS_ERROR;
            }
            else
            {
            	SCG_SetVlprClockControl(SCG, (uint32_t)config->src, (uint32_t)config->divCore, (uint32_t)config->divBus, (uint32_t)config->divSlow);
            }
            break;
#if FEATURE_HAS_HIGH_SPEED_RUN_MODE
        case SCG_SYSTEM_CLOCK_MODE_HSRUN:     /*!< High Speed Run mode.     */
            DEV_ASSERT((SCG_SYSTEM_CLOCK_SRC_FIRC == config->src) || ( SCG_SYSTEM_CLOCK_SRC_SYS_PLL == config->src));
        	/* Verify the frequencies of sys, bus and slow clocks. */
            if ((srcFreq > (sysFreqMul  * (maxSysClksInHSRUN[(uint32_t)config->src][CORE_CLK_INDEX] >> 4U))) ||    /* Sys(core) clock */
                (srcFreq > (busFreqMul * (maxSysClksInHSRUN[(uint32_t)config->src][BUS_CLK_INDEX] >> 4U)))  ||    /* Bus clock */
                (srcFreq > (slowFreqMul * (maxSysClksInHSRUN[(uint32_t)config->src][SLOW_CLK_INDEX] >> 4U))))      /* Slow clock */
            {
                /* Configuration for the next system clock source is not valid. */
                status = STATUS_ERROR;
            }
            else
            {
            	SCG_SetHsrunClockControl(SCG, (uint32_t)config->src, (uint32_t)config->divCore, (uint32_t)config->divBus, (uint32_t)config->divSlow);
            }
            break;
#endif
        default:
            /* Invalid mode */
            DEV_ASSERT(false);
            break;
    }
    return status;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSysAsyncFreq
 * Description   : Gets SCG asynchronous clock frequency from a clock source.
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSysAsyncFreq(clock_names_t clockSource,
                                          scg_async_clock_type_t type)
{
    uint32_t freq, div = 0U;

    switch (type)
    {
        case SCG_ASYNC_CLOCK_DIV1:
        {
            switch (clockSource)
            {
                case FIRC_CLK:
                {
                    freq = CLOCK_SYS_GetFircFreq();
                    div = SCG_GetFircFirstAsyncDivider(SCG);
                }
                break;
                case SIRC_CLK:
                {
                    freq = CLOCK_SYS_GetSircFreq();
                    div = SCG_GetSircFirstAsyncDivider(SCG);
                }
                break;
                case SOSC_CLK:
                {
                    freq = CLOCK_SYS_GetSysOscFreq();
                    div = SCG_GetSoscFirstAsyncDivider(SCG);
                }
                break;
#if FEATURE_HAS_SPLL_CLK
                case SPLL_CLK:
                {
                    freq = CLOCK_SYS_GetSysPllFreq();
                    div = SCG_GetSpllFirstAsyncDivider(SCG);
                }
                break;
#endif
                default:
                {
                    /* Invalid clock source type */
                    freq = 0U;
                    DEV_ASSERT(false);
                }
                break;
            }
        }
        break;
        case SCG_ASYNC_CLOCK_DIV2:
        {
            switch (clockSource)
            {
                case FIRC_CLK:
                {
                    freq = CLOCK_SYS_GetFircFreq();
                    div = SCG_GetFircSecondAsyncDivider(SCG);
                }
                break;
                case SIRC_CLK:
                {
                    freq = CLOCK_SYS_GetSircFreq();
                    div = SCG_GetSircSecondAsyncDivider(SCG);
                }
                break;
                case SOSC_CLK:
                {
                    freq = CLOCK_SYS_GetSysOscFreq();
                    div = SCG_GetSoscSecondAsyncDivider(SCG);
                }
                break;
#if FEATURE_HAS_SPLL_CLK
                case SPLL_CLK:
                {
                    freq = CLOCK_SYS_GetSysPllFreq();
                    div = SCG_GetSpllSecondAsyncDivider(SCG);
                }
                break;
#endif
                default:
                {
                    /* Invalid clock source type */
                    freq = 0U;
                    DEV_ASSERT(false);
                }
                break;
            }
        }
        break;
        default:
            /* Invalid async clock source */
            freq = 0U;
            DEV_ASSERT(false);
            break;
    }


    if (div != 0U)
    {
        freq = (freq >> (div - 1U));
    }
    else  /* Output disabled. */
    {
        freq = 0U;
    }

    return freq;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSysOscFreq
 * Description   : Gets SCG System OSC clock frequency (SYSOSC).
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSysOscFreq(void)
{
    uint32_t retValue;
    if (SCG_GetSoscStatus(SCG)) /* System OSC clock is valid. */
    {
        retValue = g_xtal0ClkFreq;
    }
    else
    {
        retValue = 0U;
    }

    return retValue;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSircFreq
 * Description   : Gets SCG Slow IRC clock frequency (SIRC).
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSircFreq(void)
{
    uint32_t retValue = 0U;

    if (SCG_GetSircStatus(SCG)) /* SIRC is valid. */
    {
		if (SCG_GetSircRange(SCG) != 0U)
		{
			retValue = FEATURE_SCG_SIRC_HIGH_RANGE_FREQ;
		}
    }

    return retValue;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetFircFreq
 * Description   : Gets SCG Fast IRC clock frequency (FIRC).
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetFircFreq(void)
{
    uint32_t retValue;

    static const uint32_t fircFreq[] = {
        FEATURE_SCG_FIRC_FREQ0,
    };

    if (SCG_GetFircStatus(SCG)) /* FIRC is valid. */
    {
        retValue = fircFreq[SCG_GetFircRange(SCG)];
    }
    else
    {
        retValue = 0U;
    }

    return retValue;
}

#if FEATURE_HAS_SPLL_CLK
/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSysPllFreq
 * Description   : Gets SCG System PLL clock frequency (SYSPLL).
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetSysPllFreq(void)
{
    uint32_t freq, retValue;

    if (SCG_GetSpllStatus(SCG)) /* System PLL is valid. */
    {
        /* Get System OSC. frequency. */
        freq = CLOCK_SYS_GetSysOscFreq();

        if (freq != 0U) /* If source is valid. */
        {
            freq /= (SCG_GetSpllPredivider(SCG) + SCG_SPLL_PREDIV_BASE);    /* Pre-divider. */
            freq *= (SCG_GetSpllMultiplier(SCG) + SCG_SPLL_MULT_BASE);      /* Multiplier. */
            freq = freq >> 1U;  /* Divide VCO by 2. */
        }

        retValue = freq;
    }
    else
    {
        retValue = 0U;
    }

    return retValue;
}
#endif

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetLpoFreq
 * Description   : Gets SIM LPO clock frequency (LPO).
 * END**************************************************************************/
static uint32_t CLOCK_SYS_GetLpoFreq(void)
{
    uint32_t freq = 0U;

    switch (SIM_GetLpoClkSelectorValue(SIM))
    {
        case 0U:  /* SIM_LPO_CLK_SEL_LPO_128K */
            freq = LPO_128K_FREQUENCY;
            break;
        case 1U:  /* SIM_LPO_CLK_SEL_NO_CLOCK: */
            freq = 0U;
            break;
        case 2U:  /* SIM_LPO_CLK_SEL_LPO_32K: */
            freq = SIM_GetLpo32KStatus(SIM) ? LPO_32K_FREQUENCY : 0UL;
            break;
        case 3U:  /* SIM_LPO_CLK_SEL_LPO_1K:  */
            freq = ((SIM_GetLpo32KStatus(SIM)) && (SIM_GetLpo1KStatus(SIM))) ? LPO_1K_FREQUENCY : 0UL;
            break;
        default:
            /* Invalid LPOCLKSEL selection.*/
            DEV_ASSERT(false);
            break;
    }

    return freq;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_TransitionToTmpSysClk
 * Description   : Transition to a temporary system clock
 * END**************************************************************************/
static status_t CLOCK_SYS_TransitionToTmpSysClk(scg_system_clock_src_t currentSysClkSrc)
{
    scg_system_clock_config_t sysClockConfig;

    static const scg_system_clock_div_t tmpSysClk[TMP_SYS_CLK_NO][TMP_SYS_DIV_NO] = TMP_SYSTEM_CLOCK_CONFIGS;

    status_t status = STATUS_ERROR;

#if FEATURE_HAS_SPLL_CLK
    /* SPLL is functional, and is not the current system clock source */
    if ((CLOCK_SYS_GetSysPllFreq() != 0U) && (currentSysClkSrc != SCG_SYSTEM_CLOCK_SRC_SYS_PLL) && (status == STATUS_ERROR))
    {
        sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SYS_PLL;
        sysClockConfig.divCore = tmpSysClk[TMP_SPLL_CLK][TMP_SYS_DIV];
        sysClockConfig.divBus  = tmpSysClk[TMP_SPLL_CLK][TMP_BUS_DIV];
        sysClockConfig.divSlow = tmpSysClk[TMP_SPLL_CLK][TMP_SLOW_DIV];
        status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
    }
#endif
    /* FIRC is functional, it is not the current system clock source, no valid source has been found yet. */
    if ((CLOCK_SYS_GetFircFreq() != 0U) && (currentSysClkSrc != SCG_SYSTEM_CLOCK_SRC_FIRC) && (status != STATUS_SUCCESS))
    {
        sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_FIRC;
        sysClockConfig.divCore = tmpSysClk[TMP_FIRC_CLK][TMP_SYS_DIV];
        sysClockConfig.divBus  = tmpSysClk[TMP_FIRC_CLK][TMP_BUS_DIV];
        sysClockConfig.divSlow = tmpSysClk[TMP_FIRC_CLK][TMP_SLOW_DIV];
        status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
    }


    /* SOSC is functional, it is not the current system clock source, no valid source has been found yet. */
    if ((CLOCK_SYS_GetSysOscFreq() != 0U) && (currentSysClkSrc != SCG_SYSTEM_CLOCK_SRC_SYS_OSC) && (status != STATUS_SUCCESS))
    {
        sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SYS_OSC;
        sysClockConfig.divCore = tmpSysClk[TMP_SOSC_CLK][TMP_SYS_DIV];
        sysClockConfig.divBus  = tmpSysClk[TMP_SOSC_CLK][TMP_BUS_DIV];
        sysClockConfig.divSlow = tmpSysClk[TMP_SOSC_CLK][TMP_SLOW_DIV];
        status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
    }

    /* SIRC is functional, it is not the current system clock source, no valid source has been found yet. */
    if ((CLOCK_SYS_GetSircFreq() != 0U) && (currentSysClkSrc != SCG_SYSTEM_CLOCK_SRC_SIRC) && (status != STATUS_SUCCESS))
    {
        sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SIRC;
        sysClockConfig.divCore = tmpSysClk[TMP_SIRC_CLK][TMP_SYS_DIV];
        sysClockConfig.divBus  = tmpSysClk[TMP_SIRC_CLK][TMP_BUS_DIV];
        sysClockConfig.divSlow = tmpSysClk[TMP_SIRC_CLK][TMP_SLOW_DIV];
        status = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
    }

    return status;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_TransitionToTmpSysClk
 * Description   : Transition to a temporary system clock
 * END**************************************************************************/
static void CLOCK_SYS_GetCurrentSysClkConfig(scg_system_clock_config_t * sysClockConfig)
{
    static const scg_system_clock_div_t sysClkDivMappings[((uint32_t) SCG_SYSTEM_CLOCK_DIV_BY_16)+ 1U] = {
            SCG_SYSTEM_CLOCK_DIV_BY_1, SCG_SYSTEM_CLOCK_DIV_BY_2,
            SCG_SYSTEM_CLOCK_DIV_BY_3, SCG_SYSTEM_CLOCK_DIV_BY_4,
            SCG_SYSTEM_CLOCK_DIV_BY_5, SCG_SYSTEM_CLOCK_DIV_BY_6,
            SCG_SYSTEM_CLOCK_DIV_BY_7, SCG_SYSTEM_CLOCK_DIV_BY_8,
            SCG_SYSTEM_CLOCK_DIV_BY_9, SCG_SYSTEM_CLOCK_DIV_BY_10,
            SCG_SYSTEM_CLOCK_DIV_BY_11, SCG_SYSTEM_CLOCK_DIV_BY_12,
            SCG_SYSTEM_CLOCK_DIV_BY_13, SCG_SYSTEM_CLOCK_DIV_BY_14,
            SCG_SYSTEM_CLOCK_DIV_BY_15, SCG_SYSTEM_CLOCK_DIV_BY_16 };

    /* Save the current system clock source configuration */
    switch(SCG_GetCurrentSystemClockSource(SCG))
    {
    case 1U:
    {
        sysClockConfig->src = SCG_SYSTEM_CLOCK_SRC_SYS_OSC;
        break;
    }
    case 2U:
    {
        sysClockConfig->src = SCG_SYSTEM_CLOCK_SRC_SIRC;
        break;
    }
    case 3U:
    {
        sysClockConfig->src = SCG_SYSTEM_CLOCK_SRC_FIRC;
        break;
    }
#if FEATURE_HAS_SPLL_CLK
    case 6U:
    {
        sysClockConfig->src = SCG_SYSTEM_CLOCK_SRC_SYS_PLL;
        break;
    }
#endif
    default:
    {
        /* Invalid system clock value */
        DEV_ASSERT(false);
        break;
    }
    }
    sysClockConfig->divBus  = sysClkDivMappings[SCG_GetCurrentBusClockDividerRatio(SCG)];
    sysClockConfig->divCore = sysClkDivMappings[SCG_GetCurrentCoreClockDividerRatio(SCG)];
    sysClockConfig->divSlow = sysClkDivMappings[SCG_GetCurrentSlowClockDividerRatio(SCG)];
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_ConvertAsyncDividerValue
 * Description   : Converts an integer value to asynchronous divider value type.
 * END**************************************************************************/
static scg_async_clock_div_t CLOCK_SYS_ConvertAsyncDividerValue(uint16_t divider)
{
    scg_async_clock_div_t retValue;

    switch(divider)
    {
        case (1U << 6U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_64;
            break;
        case (1U << 5U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_32;
            break;
        case (1U << 4U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_16;
            break;
        case (1U << 3U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_8;
            break;
        case (1U << 2U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_4;
            break;
        case (1U << 1U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_2;
            break;
        case (1U << 0U):
            retValue = SCG_ASYNC_CLOCK_DIV_BY_1;
            break;
        case 0U:
            /* Pass - through */
        default:
            retValue = SCG_ASYNC_CLOCK_DISABLE;
            break;
    }
    return retValue;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetDefaultModuleClkCfgSource
 * Description   : Gets default module clock source.
 * END**************************************************************************/
static clock_names_t CLOCK_SYS_GetDefaultModuleClkCfgSource(void)
{
	clock_names_t ret = CLOCK_NAME_COUNT;

	if (CLOCK_SYS_GetSircFreq() != 0U)
	{
		ret = SIRC_CLK;
	}
	else if (CLOCK_SYS_GetFircFreq() != 0U)
	{
		ret = FIRC_CLK;
	}
#if FEATURE_HAS_SPLL_CLK
	else if (CLOCK_SYS_GetSysPllFreq() != 0U)
	{
		ret = SPLL_CLK;
	}
#endif
	else if (CLOCK_SYS_GetSysOscFreq() != 0U)
	{
		ret = SOSC_CLK;
	}
	else
	{
		ret = CLOCK_NAME_COUNT;
	}

	return ret;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_SetClockGate
 * Description   : Sets clock gate.
 * END**************************************************************************/
static void CLOCK_SYS_SetClockGate(clock_names_t peripheralClock, bool gating)
{
	switch (peripheralClock)
	{
		case SIM_MSCM_CLK:
			SIM_SetMscmClockGate(SIM, gating);
			break;

		case SIM_MPU_CLK:
			SIM_SetMpuClockGate(SIM, gating);
			break;

		case SIM_DMA_CLK:
			SIM_SetDmaClockGate(SIM, gating);
			break;

		case SIM_ERM_CLK:
			SIM_SetErmClockGate(SIM, gating);
			break;

		case SIM_EIM_CLK:
			SIM_SetEimClockGate(SIM, gating);
			break;
		default:
			/* Invalid clock name */
			DEV_ASSERT(false);
			break;
	}
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_DRV_SetModuleClock
 * Description   : This function enables a peripheral clock
 *
 * Implements CLOCK_DRV_SetModuleClock_Activity
 * END**************************************************************************/
void CLOCK_DRV_SetModuleClock(clock_names_t peripheralClock, const module_clk_config_t * moduleClkConfig)
{
    uint32_t source = 0U, divider = 0U, multiplier = 0U;
    module_clk_config_t defaultModuleClkCfg;
    const module_clk_config_t * moduleClkCfg;

    /* Configuration is not provided, a default one will be used. */
    if (moduleClkConfig == NULL)
    {
        defaultModuleClkCfg.gating = true;
        defaultModuleClkCfg.source = CLOCK_SYS_GetDefaultModuleClkCfgSource();
        defaultModuleClkCfg.mul = 1U;
        defaultModuleClkCfg.div = 1U;
        moduleClkCfg = &defaultModuleClkCfg;
    }
    else
    {
        moduleClkCfg = moduleClkConfig;
    }

    /* Check that clock gating is configurable from PCC */
    if ((peripheralFeaturesList[peripheralClock] & HAS_CLOCK_GATING_IN_SIM) == 0U)
    {
        /* Check that protocol clock is supported by the peripheral corresponding to the clock name */
        if ((peripheralFeaturesList[peripheralClock] & (HAS_PROTOCOL_CLOCK_FROM_ASYNC1 | HAS_PROTOCOL_CLOCK_FROM_ASYNC2)) != 0U)
        {
            switch(moduleClkCfg->source)
            {
                case SIRC_CLK:
                    source = CLK_SRC_SIRC;
                    break;
                case FIRC_CLK:
                    source = CLK_SRC_FIRC;
                    break;
                case SOSC_CLK:
                    source = CLK_SRC_SOSC;
                    break;
#if FEATURE_HAS_SPLL_CLK
                case SPLL_CLK:
                    source = CLK_SRC_SPLL;
                    break;
#endif
                default:
                    /* Invalid name of the clock source. */
                    DEV_ASSERT(false);
                    break;
            }
        }

        /* Check that divider is supported by the peripheral corresponding to the clock name */
        if ((peripheralFeaturesList[peripheralClock] & HAS_DIVIDER) != 0U)
        {
            DEV_ASSERT((0U < ((uint32_t)moduleClkCfg->div)) && (((uint32_t)moduleClkCfg->div) <= (1UL << PCC_PCCn_PCD_WIDTH)));
            divider = ((uint32_t)moduleClkCfg->div) - 1U;
        }

        /* Check that multiplier is supported by the peripheral corresponding to the clock name */
        if ((peripheralFeaturesList[peripheralClock] & HAS_MULTIPLIER) != 0U)
        {
            DEV_ASSERT((0U < ((uint32_t)moduleClkCfg->mul)) && (((uint32_t)moduleClkCfg->mul) <= (1UL << PCC_PCCn_FRAC_WIDTH)));
            multiplier = ((uint32_t)moduleClkCfg->mul) - 1U;
        }

        /* Disable the peripheral clock */
        PCC_SetClockMode(PCC, peripheralClock, false);

        if (moduleClkCfg->gating)
        {
            /* Set peripheral clock control */
            PCC_SetPeripheralClockControl(PCC, peripheralClock, true, source, divider, multiplier);
        }
    }
    else
    {
    	CLOCK_SYS_SetClockGate(peripheralClock, moduleClkCfg->gating);
    }
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSysClockMode
 * Description   : Gets default system clock mode.
 * END**************************************************************************/
static scg_system_clock_mode_t CLOCK_SYS_GetSysClockMode(const pwr_modes_t mode)
{
	scg_system_clock_mode_t sysClockMode;

	switch(mode)
	{
		/* High speed run mode */
		case HSRUN_MODE:
			sysClockMode = SCG_SYSTEM_CLOCK_MODE_HSRUN;
			break;
		/* Run mode */
		case RUN_MODE:
			sysClockMode = SCG_SYSTEM_CLOCK_MODE_RUN;
			break;
		/* Very low power run mode */
		case VLPR_MODE:
			sysClockMode = SCG_SYSTEM_CLOCK_MODE_VLPR;
			break;
		/* This should never happen - input power mode is invalid */
		default:
			sysClockMode = SCG_SYSTEM_CLOCK_MODE_NONE;
			DEV_ASSERT(false);
			break;
	}

	return sysClockMode;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetSysClockSource
 * Description   : Gets default system clock source.
 * END**************************************************************************/
static scg_system_clock_src_t CLOCK_SYS_GetSysClockSource(clock_names_t src)
{
	scg_system_clock_src_t source;

	switch(src)
	{
		case SIRC_CLK:
			source = SCG_SYSTEM_CLOCK_SRC_SIRC;
			break;

		case FIRC_CLK:
			source = SCG_SYSTEM_CLOCK_SRC_FIRC;
			break;

		case SOSC_CLK:
			source = SCG_SYSTEM_CLOCK_SRC_SYS_OSC;
			break;

#if FEATURE_HAS_SPLL_CLK
		case SPLL_CLK:
			source = SCG_SYSTEM_CLOCK_SRC_SYS_PLL;
			break;
#endif

		default:
			source = SCG_SYSTEM_CLOCK_SRC_NONE;
			DEV_ASSERT(false);
			break;
	}

	return source;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_DRV_SetSystemClock
 * Description   : This function configures the system clocks (core, bus and flash clocks).
 *
 * Implements CLOCK_DRV_SetSystemClock_Activity
 * END**************************************************************************/
status_t CLOCK_DRV_SetSystemClock(const pwr_modes_t * mode,
                                  const sys_clk_config_t * sysClkConfig)
{
    status_t retCode = STATUS_SUCCESS;

    scg_system_clock_mode_t sysClockMode, currentSysClockMode = CLOCK_SYS_GetCurrentRunMode();
    scg_system_clock_config_t sysClockConfig;

    static const scg_system_clock_div_t sysClkDivMappings[] = {SCG_SYSTEM_CLOCK_DIV_BY_1,SCG_SYSTEM_CLOCK_DIV_BY_1,SCG_SYSTEM_CLOCK_DIV_BY_2,SCG_SYSTEM_CLOCK_DIV_BY_3,SCG_SYSTEM_CLOCK_DIV_BY_4,SCG_SYSTEM_CLOCK_DIV_BY_5,SCG_SYSTEM_CLOCK_DIV_BY_6,SCG_SYSTEM_CLOCK_DIV_BY_7,SCG_SYSTEM_CLOCK_DIV_BY_8,SCG_SYSTEM_CLOCK_DIV_BY_9,SCG_SYSTEM_CLOCK_DIV_BY_10,SCG_SYSTEM_CLOCK_DIV_BY_11,SCG_SYSTEM_CLOCK_DIV_BY_12,SCG_SYSTEM_CLOCK_DIV_BY_13,SCG_SYSTEM_CLOCK_DIV_BY_14,SCG_SYSTEM_CLOCK_DIV_BY_15,SCG_SYSTEM_CLOCK_DIV_BY_16};
    static const scg_system_clock_div_t defSysClk[TMP_SYS_CLK_NO][TMP_SYS_DIV_NO] = TMP_SYSTEM_CLOCK_CONFIGS;

    /* The power mode is not provided, the current one will be used by default. */
    if (mode == NULL)
    {
        sysClockMode = currentSysClockMode;
    }
    else
    {
    	sysClockMode = CLOCK_SYS_GetSysClockMode(*mode);
    }

    /* System clock configuration is not provided, a default one will be used. */
    if (sysClkConfig == NULL)
    {
        /* Find a valid clock source. */
        if (CLOCK_SYS_GetFircFreq() != 0U)
        {
            sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_FIRC;
            sysClockConfig.divCore = defSysClk[TMP_FIRC_CLK][TMP_SYS_DIV];
            sysClockConfig.divBus  = defSysClk[TMP_FIRC_CLK][TMP_BUS_DIV];
            sysClockConfig.divSlow = defSysClk[TMP_FIRC_CLK][TMP_SLOW_DIV];
        }
#if FEATURE_HAS_SPLL_CLK
        else if (CLOCK_SYS_GetSysPllFreq() != 0U)
        {
            sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SYS_PLL;
            sysClockConfig.divCore = defSysClk[TMP_SPLL_CLK][TMP_SYS_DIV];
            sysClockConfig.divBus  = defSysClk[TMP_SPLL_CLK][TMP_BUS_DIV];
            sysClockConfig.divSlow = defSysClk[TMP_SPLL_CLK][TMP_SLOW_DIV];
        }
#endif
        else if (CLOCK_SYS_GetSysOscFreq() != 0U)
        {
            sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SYS_OSC;
            sysClockConfig.divCore = defSysClk[TMP_SOSC_CLK][TMP_SYS_DIV];
            sysClockConfig.divBus  = defSysClk[TMP_SOSC_CLK][TMP_BUS_DIV];
            sysClockConfig.divSlow = defSysClk[TMP_SOSC_CLK][TMP_SLOW_DIV];
        }
        else if (CLOCK_SYS_GetSircFreq() != 0U)
        {
            sysClockConfig.src     = SCG_SYSTEM_CLOCK_SRC_SIRC;
            sysClockConfig.divCore = defSysClk[TMP_SIRC_CLK][TMP_SYS_DIV];
            sysClockConfig.divBus  = defSysClk[TMP_SIRC_CLK][TMP_BUS_DIV];
            sysClockConfig.divSlow = defSysClk[TMP_SIRC_CLK][TMP_SLOW_DIV];
        }
        else
        {
            /* Can't reach this point, at least one clock source is functional.
             * This code is written to avoid MISRA 15.7 (no 'else' at end of 'if ... else if' chain) */
            DEV_ASSERT(false);
        }
    }
    else
    {
        /* The system clock source from input configuration structure must be valid. */
        DEV_ASSERT((CLOCK_SYS_GetSircFreq() != 0U) || (sysClkConfig->src != SIRC_CLK));
        DEV_ASSERT((CLOCK_SYS_GetFircFreq() != 0U) || (sysClkConfig->src != FIRC_CLK));
        DEV_ASSERT((CLOCK_SYS_GetSysOscFreq() != 0U) || (sysClkConfig->src != SOSC_CLK));
#if FEATURE_HAS_SPLL_CLK
        DEV_ASSERT((CLOCK_SYS_GetSysPllFreq() != 0U) || (sysClkConfig->src != SPLL_CLK));
#endif

        sysClockConfig.src = CLOCK_SYS_GetSysClockSource(sysClkConfig->src);

        DEV_ASSERT(sysClkConfig->dividers[0U] != 0U);
        DEV_ASSERT(sysClkConfig->dividers[1U] != 0U);
        DEV_ASSERT(sysClkConfig->dividers[2U] != 0U);

        sysClockConfig.divCore = sysClkDivMappings[sysClkConfig->dividers[0U]];
        sysClockConfig.divBus  = sysClkDivMappings[sysClkConfig->dividers[1U]];
        sysClockConfig.divSlow = sysClkDivMappings[sysClkConfig->dividers[2U]];
    }

    /* System clock is configured in the current mode,
     * set configuration and wait until the system clock is changed. */
    if (sysClockMode == currentSysClockMode)
    {
        retCode = CLOCK_SYS_TransitionSystemClock(&sysClockConfig);
    }
    /* System clock is not configured in the current mode, just set configuration. */
    else
    {
        retCode = CLOCK_SYS_SetSystemClockConfig(sysClockMode,&sysClockConfig);
    }
    return retCode;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_DRV_GetSystemClockSource
 * Description   : This function gets the current system clock source
 *
 * Implements CLOCK_DRV_GetSystemClockSource_Activity
 * END**************************************************************************/
void CLOCK_DRV_GetSystemClockSource(sys_clk_config_t *sysClkConfig)
{
    switch(SCG_GetCurrentSystemClockSource(SCG))
    {
        case SCG_SYSTEM_CLOCK_SRC_SYS_OSC:
        {
            sysClkConfig->src = SOSC_CLK;
        }
        break;
        case SCG_SYSTEM_CLOCK_SRC_SIRC:
        {
            sysClkConfig->src = SIRC_CLK;
        }
        break;
        case SCG_SYSTEM_CLOCK_SRC_FIRC:
        {
            sysClkConfig->src = FIRC_CLK;
        }
        break;
#if FEATURE_HAS_SPLL_CLK
        case SCG_SYSTEM_CLOCK_SRC_SYS_PLL:
        {
            sysClkConfig->src = SPLL_CLK;
        }
        break;
#endif
        default:
        {
            /* Invalid system clock source. */
            DEV_ASSERT(false);
            sysClkConfig->src = SIRC_CLK;
        }
        break;
    }

    /* Core divider */
    sysClkConfig->dividers[0U] = (uint16_t) (SCG_GetCurrentCoreClockDividerRatio(SCG) + 1U);

    /* Bus divider */
    sysClkConfig->dividers[1U] = (uint16_t) (SCG_GetCurrentBusClockDividerRatio(SCG) + 1U);

    /* Slow divider */
    sysClkConfig->dividers[2U] = (uint16_t) (SCG_GetCurrentSlowClockDividerRatio(SCG) + 1U);
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_SetSircClockSource
 * Description   : Sets SIRC clock.
 * END**************************************************************************/
static status_t CLOCK_SYS_SetSircClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2)
{
	status_t retCode = STATUS_SUCCESS;
	scg_system_clock_config_t currentSysClockConfig;
	bool revertTmpSysClkTransition = false;
	scg_sirc_config_t scgSircConfig;

	/* Check whether the current system clock source is SIRC */
	if (SCG_GetCurrentSystemClockSource(SCG) == ((uint32_t)SCG_SYSTEM_CLOCK_SRC_SIRC))
	{
		if (clockSourceEnable == false)
		{
			/* Can't disable SIRC, it's used as system clock source */
			retCode = STATUS_ERROR;
		}
		else
		{
			/* Save the current system clock source configuration */
			CLOCK_SYS_GetCurrentSysClkConfig(&currentSysClockConfig);

			/* Transition to a temporary system clock source */
			retCode = CLOCK_SYS_TransitionToTmpSysClk(SCG_SYSTEM_CLOCK_SRC_SIRC);

			revertTmpSysClkTransition = (retCode == STATUS_SUCCESS) ? true : false;
		}
	}

	if (retCode == STATUS_SUCCESS)
	{
		if (clkSrcConfig == NULL)
		{
			retCode = CLOCK_SYS_ConfigureSIRC(true, NULL);
		}
		else
		{
			/* Check VLP modes */
			scgSircConfig.enableInLowPower  = true;
			scgSircConfig.enableInStop  = true;
			scgSircConfig.locked            = false;

			scgSircConfig.div1              = divider1;
			scgSircConfig.div2              = divider2;

			scgSircConfig.range             = SCG_SIRC_RANGE_HIGH;

			retCode = CLOCK_SYS_ConfigureSIRC(clockSourceEnable, &scgSircConfig);
		}

		/* If system clock source was SIRC and SIRC has been set successfully,
		 * then transition back to SIRC */
		if ((retCode == STATUS_SUCCESS) && (revertTmpSysClkTransition == true))
		{
			retCode = CLOCK_SYS_TransitionSystemClock(&currentSysClockConfig);
		}
	}

	return retCode;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_SetFircClockSource
 * Description   : Sets FIRC clock.
 * END**************************************************************************/
static status_t CLOCK_SYS_SetFircClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2)
{
	status_t retCode = STATUS_SUCCESS;
	scg_system_clock_config_t currentSysClockConfig;
	bool revertTmpSysClkTransition = false;
	scg_firc_config_t scgFircConfig;

	/* Check whether the current system clock source is FIRC */
	if (SCG_GetCurrentSystemClockSource(SCG) == ((uint32_t)SCG_SYSTEM_CLOCK_SRC_FIRC))
	{
		if (clockSourceEnable == false)
		{
			/* Can't disable FIRC, it's used as system clock source. */
			retCode = STATUS_ERROR;
		}
		else
		{
			/* Save the current system clock source configuration. */
			CLOCK_SYS_GetCurrentSysClkConfig(&currentSysClockConfig);

			/* Transition to a temporary system clock source. */
			retCode = CLOCK_SYS_TransitionToTmpSysClk(SCG_SYSTEM_CLOCK_SRC_FIRC);

			revertTmpSysClkTransition = (retCode == STATUS_SUCCESS) ? true : false;
		}
	}

	if (retCode == STATUS_SUCCESS)
	{
		if (clkSrcConfig == NULL)
		{
			retCode = CLOCK_SYS_ConfigureFIRC(clockSourceEnable, NULL);
		}
		else
		{
			scgFircConfig.locked            = false;

			scgFircConfig.div1              = divider1;
			scgFircConfig.div2              = divider2;

			scgFircConfig.range             = SCG_FIRC_RANGE_48M;
			scgFircConfig.regulator         = 0U;

			retCode = CLOCK_SYS_ConfigureFIRC(clockSourceEnable, &scgFircConfig);
		}

		/* If system clock source was FIRC and FIRC has been set successfully,
		 * then transition back to FIRC */
		if ((retCode == STATUS_SUCCESS) && (revertTmpSysClkTransition == true))
		{
			retCode = CLOCK_SYS_TransitionSystemClock(&currentSysClockConfig);
		}
	}

	return retCode;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_GetExtRef
 * Description   : Gets external reference value for SOSC clock.
 * END**************************************************************************/
static scg_sosc_ext_ref_t CLOCK_SYS_GetExtRef(xosc_ref_t refClk)
{
	scg_sosc_ext_ref_t extRef = SCG_SOSC_REF_EXT;

	switch (refClk)
	{
		case XOSC_EXT_REF:
			extRef            = SCG_SOSC_REF_EXT;
			break;

		case XOSC_INT_OSC:
			extRef            = SCG_SOSC_REF_OSC;
			break;

		default:
			/* Invalid input reference value */
			DEV_ASSERT(false);
			break;
	}

	return extRef;
}

/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_SetSoscClockSource
 * Description   : Sets SOSC clock.
 * END**************************************************************************/
static status_t CLOCK_SYS_SetSoscClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2)
{
	status_t retCode = STATUS_SUCCESS;
	scg_system_clock_config_t currentSysClockConfig;
	bool revertTmpSysClkTransition = false;
	scg_sosc_config_t scgSoscConfig;

	/* Check whether the current system clock source is SOSC */
	if (SCG_GetCurrentSystemClockSource(SCG) == ((uint32_t)SCG_SYSTEM_CLOCK_SRC_SYS_OSC))
	{
		if (clockSourceEnable == false)
		{
			/* Can't disable SOSC, it's used as system clock source. */
			retCode = STATUS_ERROR;
		}
		else
		{
			/* Save the current system clock source configuration. */
			CLOCK_SYS_GetCurrentSysClkConfig(&currentSysClockConfig);

			/* Transition to a temporary system clock source. */
			retCode = CLOCK_SYS_TransitionToTmpSysClk(SCG_SYSTEM_CLOCK_SRC_SYS_OSC);

			revertTmpSysClkTransition = (retCode == STATUS_SUCCESS) ? true : false;
		}
	}

	if (retCode == STATUS_SUCCESS)
	{
		if (clkSrcConfig == NULL)
		{
			retCode = CLOCK_SYS_ConfigureSOSC(clockSourceEnable, NULL);
		}
		else
		{
			scgSoscConfig.freq              = clkSrcConfig->refFreq;

			scgSoscConfig.monitorMode       = SCG_SOSC_MONITOR_DISABLE;
			scgSoscConfig.locked            = false;

			scgSoscConfig.div1              = divider1;
			scgSoscConfig.div2              = divider2;

			scgSoscConfig.extRef            = CLOCK_SYS_GetExtRef(clkSrcConfig->refClk);

			scgSoscConfig.gain              = SCG_SOSC_GAIN_LOW;

			if((scgSoscConfig.freq >= 4000000U) && (scgSoscConfig.freq <= 8000000U))
			{
				scgSoscConfig.range             = SCG_SOSC_RANGE_MID;
			}
			else if((scgSoscConfig.freq >= 8000000U) && (scgSoscConfig.freq <= 40000000U))
			{
				scgSoscConfig.range             = SCG_SOSC_RANGE_HIGH;
			}
			else
			{
				/* Frequency of the input reference clock is invalid. */
				DEV_ASSERT(false);
			}

			retCode = CLOCK_SYS_ConfigureSOSC(clockSourceEnable, &scgSoscConfig);
		}

		/* If system clock source was SOSC and SOSC has been set successfully,
		 * then transition back to SOSC */
		if ((retCode == STATUS_SUCCESS) && (revertTmpSysClkTransition == true))
		{
			retCode = CLOCK_SYS_TransitionSystemClock(&currentSysClockConfig);
		}
	}

	return retCode;
}

#if FEATURE_HAS_SPLL_CLK
/*FUNCTION**********************************************************************
 * Function Name : CLOCK_SYS_SetSpllClockSource
 * Description   : Sets SPLL clock.
 * END**************************************************************************/
static status_t CLOCK_SYS_SetSpllClockSource(bool clockSourceEnable, const clock_source_config_t * clkSrcConfig, scg_async_clock_div_t divider1, scg_async_clock_div_t divider2)
{
	status_t retCode = STATUS_SUCCESS;
	scg_system_clock_config_t currentSysClockConfig;
	bool revertTmpSysClkTransition = false;
	scg_spll_config_t scgSpllConfig;

	/* Check whether the current system clock source is PLL */
	if (SCG_GetCurrentSystemClockSource(SCG) == ((uint32_t)SCG_SYSTEM_CLOCK_SRC_SYS_PLL))
	{
		if (clockSourceEnable == false)
		{
			/* Can't disable PLL, it's used as system clock source. */
			retCode = STATUS_ERROR;
		}
		else
		{
			/* Save the current system clock source configuration. */
			CLOCK_SYS_GetCurrentSysClkConfig(&currentSysClockConfig);

			/* Transition to a temporary system clock source. */
			retCode = CLOCK_SYS_TransitionToTmpSysClk(SCG_SYSTEM_CLOCK_SRC_SYS_PLL);

			revertTmpSysClkTransition = (retCode == STATUS_SUCCESS) ? true : false;
		}
	}

	if (retCode == STATUS_SUCCESS)
	{
		if (clkSrcConfig == NULL)
		{
			retCode = CLOCK_SYS_ConfigureSPLL(clockSourceEnable, NULL);
		}
		else
		{
			scgSpllConfig.monitorMode       = SCG_SPLL_MONITOR_DISABLE;
			scgSpllConfig.locked            = false;

			scgSpllConfig.div1              = divider1;
			scgSpllConfig.div2              = divider2;

			DEV_ASSERT((SCG_SPLL_PREDIV_BASE <= clkSrcConfig->div) && (clkSrcConfig->div < (SCG_SPLL_PREDIV_BASE + (1U << SCG_SPLLCFG_PREDIV_WIDTH)) ));
			scgSpllConfig.prediv = (uint8_t)(clkSrcConfig->div - SCG_SPLL_PREDIV_BASE);

			DEV_ASSERT((SCG_SPLL_MULT_BASE <= clkSrcConfig->mul) && (clkSrcConfig->mul < (SCG_SPLL_MULT_BASE + (1U << SCG_SPLLCFG_MULT_WIDTH)) ));
			scgSpllConfig.mult = (uint8_t)(clkSrcConfig->mul - SCG_SPLL_MULT_BASE);

			if (retCode == STATUS_SUCCESS)
			{
				retCode = CLOCK_SYS_ConfigureSPLL(clockSourceEnable, &scgSpllConfig);
			}
		}

		/* If system clock source was PLL and PLL has been set successfully,
		 * then transition back to PLL */
		if ((retCode == STATUS_SUCCESS) && (revertTmpSysClkTransition == true))
		{
			retCode = CLOCK_SYS_TransitionSystemClock(&currentSysClockConfig);
		}
	}

	return retCode;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_DRV_SetClockSource
 * Description   : This function configures a clock source
 *
 * Implements CLOCK_DRV_SetClockSource_Activity
 * END**************************************************************************/
status_t CLOCK_DRV_SetClockSource(clock_names_t clockSource, const clock_source_config_t * clkSrcConfig)
{
    status_t retCode = STATUS_SUCCESS;
    bool clockSourceEnable = true;

    scg_async_clock_div_t divider1, divider2;

    if (clkSrcConfig == NULL)
    {
    	divider1 = SCG_ASYNC_CLOCK_DIV_BY_1;
    	divider2 = SCG_ASYNC_CLOCK_DIV_BY_1;
    }
    else
    {
    	divider1 = CLOCK_SYS_ConvertAsyncDividerValue(clkSrcConfig->outputDiv1);
    	divider2 = CLOCK_SYS_ConvertAsyncDividerValue(clkSrcConfig->outputDiv2);
    }

    /* Check whether the command enables and disables the clock source */
    if ((clkSrcConfig != NULL) && (clkSrcConfig->enable == false))
    {
        clockSourceEnable = false;
    }

    switch (clockSource)
    {
        case SIRC_CLK:
        	retCode = CLOCK_SYS_SetSircClockSource(clockSourceEnable, clkSrcConfig, divider1, divider2);
            break;

        case FIRC_CLK:
        	retCode = CLOCK_SYS_SetFircClockSource(clockSourceEnable, clkSrcConfig, divider1, divider2);
            break;

        case SOSC_CLK:
        	retCode = CLOCK_SYS_SetSoscClockSource(clockSourceEnable, clkSrcConfig, divider1, divider2);
            break;

#if FEATURE_HAS_SPLL_CLK
        case SPLL_CLK:
        	retCode = CLOCK_SYS_SetSpllClockSource(clockSourceEnable, clkSrcConfig, divider1, divider2);
            break;
#endif

        case SIM_LPO_CLK:

            if (clockSourceEnable)
            {
                SIM_SetLpoClocks(SIM,true,true,SIM_LPO_CLK_SEL_LPO_128K,SIM_RTCCLK_SEL_SOSCDIV1_CLK);
                PMC_SetLpoMode(PMC, true);
            }
            else
            {
                PMC_SetLpoMode(PMC, false);
            }
            break;

        default:
            /* Invalid name of the clock source */
            DEV_ASSERT(false);
            break;
    }

    return retCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_Init
 * Description   : Install pre-defined clock configurations.
 * This function installs the pre-defined clock configuration table to the
 * clock manager.
 *
 * Implements CLOCK_SYS_Init_Activity
 *END**************************************************************************/
status_t CLOCK_SYS_Init(clock_manager_user_config_t const **clockConfigsPtr,
                              uint8_t configsNumber,
                              clock_manager_callback_user_config_t **callbacksPtr,
                              uint8_t callbacksNumber)
{
    DEV_ASSERT(clockConfigsPtr != NULL);
    DEV_ASSERT(callbacksPtr != NULL);

    g_clockState.configTable     = clockConfigsPtr;
    g_clockState.clockConfigNum  = configsNumber;
    g_clockState.callbackConfig  = callbacksPtr;
    g_clockState.callbackNum     = callbacksNumber;

    /*
     * errorCallbackIndex is the index of the callback which returns error
     * during clock mode switch. If all callbacks return success, then the
     * errorCallbackIndex is callbacksNumber.
     */
    g_clockState.errorCallbackIndex = callbacksNumber;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_UpdateConfiguration
 * Description   : Send notification and change system clock configuration.
 * This function sends the notification to all callback functions, if all
 * callbacks return OK or forceful policy is used, this function will change
 * system clock configuration. The function should be called only on run mode.
 *
 * Implements CLOCK_SYS_UpdateConfiguration_Activity
 *END**************************************************************************/
status_t CLOCK_SYS_UpdateConfiguration(uint8_t targetConfigIndex,
                                                   clock_manager_policy_t policy)
{
    uint8_t callbackIdx;
    bool successfulSetConfig;           /* SetConfiguraiton status */
    status_t ret = STATUS_SUCCESS;
    const clock_manager_callback_user_config_t * callbackConfig;
    clock_notify_struct_t notifyStruct;

    DEV_ASSERT(targetConfigIndex < g_clockState.clockConfigNum);       /* Clock configuration index is out of range. */

    notifyStruct.targetClockConfigIndex = targetConfigIndex;
    notifyStruct.policy                 = policy;

    /* Disable interrupts */
    INT_SYS_DisableIRQGlobal();
    /* Set errorcallbackindex as callbackNum, which means no callback error now.*/
    g_clockState.errorCallbackIndex = g_clockState.callbackNum;

    /* First step: Send "BEFORE" notification. */
    notifyStruct.notifyType = CLOCK_MANAGER_NOTIFY_BEFORE;

    /* Send notification to all callback. */
    for (callbackIdx=0; callbackIdx<g_clockState.callbackNum; callbackIdx++)
    {
        callbackConfig = g_clockState.callbackConfig[callbackIdx];
        if ((callbackConfig) &&
            (callbackConfig->callbackType != CLOCK_MANAGER_CALLBACK_AFTER))
        {
            if (STATUS_SUCCESS !=
                    (*callbackConfig->callback)(&notifyStruct,
                        callbackConfig->callbackData))
            {
                g_clockState.errorCallbackIndex = callbackIdx;

                if (CLOCK_MANAGER_POLICY_AGREEMENT == policy)
                {
                    /* Save the error callback index. */
                    ret = STATUS_MCU_NOTIFY_BEFORE_ERROR;
                    break;
                }
            }
        }
    }

    /* If all callback success or forceful policy is used. */
    if ((STATUS_SUCCESS == ret) ||
        (policy == CLOCK_MANAGER_POLICY_FORCIBLE))
    {
        /* clock mode switch. */
        ret = CLOCK_SYS_SetConfiguration(g_clockState.configTable[targetConfigIndex]);
        successfulSetConfig = (STATUS_SUCCESS == ret);

        g_clockState.curConfigIndex = targetConfigIndex;
    }
    else
    {
        /* Unsuccessful setConfiguration */
        successfulSetConfig = false;
    }

    if(successfulSetConfig){
        notifyStruct.notifyType = CLOCK_MANAGER_NOTIFY_AFTER;

        for (callbackIdx=0; callbackIdx<g_clockState.callbackNum; callbackIdx++)
        {
            callbackConfig = g_clockState.callbackConfig[callbackIdx];
            if ((callbackConfig) &&
                (callbackConfig->callbackType != CLOCK_MANAGER_CALLBACK_BEFORE))
            {
                if (STATUS_SUCCESS !=
                        (*callbackConfig->callback)(&notifyStruct,
                            callbackConfig->callbackData))
                {
                    g_clockState.errorCallbackIndex = callbackIdx;

                    if (CLOCK_MANAGER_POLICY_AGREEMENT == policy)
                    {
                        /* Save the error callback index. */
                        ret = STATUS_MCU_NOTIFY_AFTER_ERROR;
                        break;
                    }
                }
            }
        }
    }
    else /* Error occurs, need to send "RECOVER" notification. */
    {
        notifyStruct.notifyType = CLOCK_MANAGER_NOTIFY_RECOVER;
        for(;;)
        {
            callbackConfig = g_clockState.callbackConfig[callbackIdx];
            if (callbackConfig != NULL)
            {
                (void)(*callbackConfig->callback)(&notifyStruct,
                        callbackConfig->callbackData);
            }
            if(callbackIdx == 0U)
            {
                break;
            }
            callbackIdx--;
        }
    }

    /* Enable interrupts */
    INT_SYS_EnableIRQGlobal();

    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetCurrentConfiguration
 * Description   : Get current clock configuration index.
 *
 * Implements CLOCK_SYS_GetCurrentConfiguration_Activity
 *END**************************************************************************/
uint8_t CLOCK_SYS_GetCurrentConfiguration(void)
{
    return g_clockState.curConfigIndex;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetErrorCallback
 * Description   : Get the callback which returns error in last clock switch.
 *
 * Implements CLOCK_SYS_GetErrorCallback_Activity
 *END**************************************************************************/
clock_manager_callback_user_config_t* CLOCK_SYS_GetErrorCallback(void)
{
    clock_manager_callback_user_config_t *retValue;

    /* If all callbacks return success. */
    if (g_clockState.errorCallbackIndex >= g_clockState.clockConfigNum)
    {
        retValue = NULL;
    }
    else
    {
        retValue = g_clockState.callbackConfig[g_clockState.errorCallbackIndex];
    }
    return retValue;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_GetFreq
 * Description   : Wrapper over CLOCK_DRV_GetFreq function. It's part of the old API.
 *
 * Implements CLOCK_SYS_GetFreq_Activity
 *END**************************************************************************/
status_t CLOCK_SYS_GetFreq(clock_names_t clockName, uint32_t *frequency)
{
    return CLOCK_DRV_GetFreq(clockName,frequency);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CLOCK_SYS_SetConfiguration
 * Description   : Wrapper over CLOCK_DRV_Init function. It's part of the old API.
 *
 * Implements CLOCK_SYS_SetConfiguration_Activity
 *END**************************************************************************/
status_t CLOCK_SYS_SetConfiguration(clock_manager_user_config_t const * config)
{
    return CLOCK_DRV_Init(config);
}



/*******************************************************************************
 * EOF
 ******************************************************************************/
