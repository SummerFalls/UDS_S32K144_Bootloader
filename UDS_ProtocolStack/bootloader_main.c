/*
 * @ 名称: bootloader_main.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "bootloader_main.h"

/*******************************************************************************
 * User Include
 ******************************************************************************/
#include "includes.h"
#include "uds_app.h"
#include "TP.h"
#include "fls_app.h"
#include "timer_hal.h"
#include "watchdog_hal.h"
#include "boot.h"
#include "CRC_HAL.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*FUNCTION**********************************************************************
 *
 * Function Name : BOOTLOADER_MAIN_Init
 * Description   : This function initial this module.
 *
 * Implements : BOOTLOADER_MAIN_Init_Activity
 *END**************************************************************************/
void BOOTLOADER_MAIN_Init(void (*pfBSP_Init)(void), void (*pfAbortTxMsg)(void))
{
    /*Is power on ?*/
    if (TRUE == Boot_IsPowerOnTriggerReset()) {
        Boot_PowerONClearAllFlag();
    }

    /*Check jump to APP or not.*/
    Boot_JumpToAppOrNot();

    /*User Init: clock CAN Lin etc..*/
    if (NULL_PTR != pfBSP_Init) {
        /*do BSP init*/
        (*pfBSP_Init)();
    }

    BOOTLOADER_DEBUG_Init();

    if (TRUE != CRC_HAL_Init()) {
        APPDebugPrintf("CRC_HAL_Init failed!\n");
    }

    BOOTLOADER_DEBUG_Init();

    WATCHDOG_HAL_Init();

    TIMER_HAL_Init();

    TP_Init();

    if (TRUE != FLASH_HAL_APPAddrCheck()) {
        APPDebugPrintf("\n FLASH_HAL_APPAddrCheck check error!\n");
    }

    UDS_Init();

    Boot_CheckReqBootloaderMode();

    TP_RegisterAbortTxMsg(pfAbortTxMsg);

    FLASH_APP_Init();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : BOOTLOADER_MAIN_Demo
 * Description   : This function initial this module.
 *
 * Implements : BOOTLOADER_MAIN_Demo_Activity
 *END**************************************************************************/
void BOOTLOADER_MAIN_Demo(void)
{
#ifdef EN_DEBUG_IO
    static uint16 timerCnt1Ms = 0u;
#endif

    if (TRUE == TIMER_HAL_Is1msTickTimeout()) {
        TP_SystemTickCtl();

        UDS_SystemTickCtl();


#ifdef EN_DEBUG_IO
        /*tigger LED*/
        timerCnt1Ms++;

        if (250u == timerCnt1Ms) {
            timerCnt1Ms = 0u;

            ToggleDebugIO();
        }

#endif
    }

    /*fed watchdog every 100ms*/
    if (TRUE == TIMER_HAL_Is100msTickTimeout()) {
        WATCHDOG_HAL_Fed();
    }

    TP_MainFun();

    UDS_MainFun();

    Flash_OperateMainFunction();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : BOOTLOADER_MAIN_Deinit
 * Description   : This function initial this module.
 *
 * Implements : BOOTLOADER_MAIN_Deinit_Activity
 *END**************************************************************************/
void BOOTLOADER_MAIN_Deinit(void)
{

}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
