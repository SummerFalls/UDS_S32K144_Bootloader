/* ###################################################################
**     Filename    : main.c
**     Processor   : S32K1xx
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.00
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including necessary module. Cpu.h contains other modules needed for compiling.*/
#include "Cpu.h"

  volatile int exit_code = 0;

/* User includes (#include below this line is not maintained by Processor Expert) */
#include "pin_mux.h"
#include "bootloader_main.h"
#include "TP.h"
#include "can_driver.h"
#include "flash.h"

#define MAX_WAIT_TIME_MS (5000u)
unsigned short g_usMaxDelayUdsMsgTime = MAX_WAIT_TIME_MS;
unsigned char g_ucIsRxUdsMsg = FALSE;

static void BSP_init(void)
{
    /*clock init*/
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    /*GPIO init*/
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

    /*CAN init*/
    InitCAN();

    /*init flash*/
    InitFlash();
}

void SendMsgMainFun(void)
{
    uint8 aucMsgBuf[8u];
    uint32 msgId = 0u;
    uint32 msgLength = 0u;

    /*get message from TP*/
    if(TRUE == TP_DriverReadDataFromTP(8u,&aucMsgBuf[0u],&msgId, &msgLength))
    {
        TransmiteCANMsg(msgId, msgLength, aucMsgBuf, &TP_DoTxMsgSuccesfulCallback, 0u);
    }
}

/*! 
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
    BOOTLOADER_MAIN_Init(BSP_init, NULL_PTR);

    g_usMaxDelayUdsMsgTime = MAX_WAIT_TIME_MS;

    APPDebugPrintf("-->Enter CAN Bootloader S32K144--<\n");

    for(;;)
    {
        BOOTLOADER_MAIN_Demo();

        SendMsgMainFun();

    } /* loop forever */

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the NXP S32K series of microcontrollers.
**
** ###################################################################
*/
