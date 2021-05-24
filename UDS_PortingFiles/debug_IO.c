/*
 * @ 名称: debug_IO.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "user_config.h"

/*******************************************************************************
 * User Include
 ******************************************************************************/
#ifdef EN_DEBUG_IO


#define BLUE_PORT       (PTD)
#define BLUE_PIN        (1u << 0u)

#define RED_PORT        (PTD)
#define RED_PIN         (1u << 15u)

#define GREEN_PORT      (PTD)
#define GREEN_PIN       (1u << 16u)

#ifdef UDS_PROJECT_FOR_BOOTLOADER
#define DBG_LED_PORT    (BLUE_PORT)
#define DBG_LED_PIN     (BLUE_PIN)
#endif

#ifdef UDS_PROJECT_FOR_APP
#define DBG_LED_PORT    (GREEN_PORT)
#define DBG_LED_PIN     (GREEN_PIN)
#endif

/*****************************************/
#endif
#include "debug_IO.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

#ifdef EN_DEBUG_IO
/*FUNCTION**********************************************************************
 *
 * Function Name : DEBUG_IO_Init
 * Description   : This function initial this module.
 *
 * Implements : DEBUG_IO_Init_Activity
 *END**************************************************************************/
void DEBUG_IO_Init(void)
{
}

/*FUNCTION**********************************************************************
 *
 * Function Name : DEBUG_IO_Deinit
 * Description   : This function initial this module.
 *
 * Implements : DEBUG_IO_Deinit_Activity
 *END**************************************************************************/
void DEBUG_IO_Deinit(void)
{
}

void DEBUG_IO_SetDebugIOLow(void)
{
    PINS_DRV_ClearPins(DBG_LED_PORT, DBG_LED_PIN);
}

void DEBUG_IO_SetDebugIOHigh(void)
{
    PINS_DRV_SetPins(DBG_LED_PORT, DBG_LED_PIN);
}

void DEBUG_IO_ToggleDebugIO(void)
{
    PINS_DRV_TogglePins(DBG_LED_PORT, DBG_LED_PIN);
}

#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
