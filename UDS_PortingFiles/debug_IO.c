/*
 * @ ����: debug_IO.c
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "user_config.h"

/*******************************************************************************
 * User Include
 ******************************************************************************/
#ifdef EN_DEBUG_IO
/*here include platform headers for driver*/


#define BLUE_PORT (PTD)
#define BLUE_PIN (1u << 0u)

#define RED_PORT (PTD)
#define RED_PIN (1u << 15u)

#define GREEN_PORT (PTD)
#define GREEN_PIN (1u << 16u)

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
    PINS_DRV_ClearPins(PTD, (1 << 0));

}

void DEBUG_IO_SetDebugIOHigh(void)
{
    PINS_DRV_SetPins(PTD, (1 << 0));
}

void DEBUG_IO_ToggleDebugIO(void)
{
    PINS_DRV_TogglePins(PTD, (1 << 0));
}

#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
