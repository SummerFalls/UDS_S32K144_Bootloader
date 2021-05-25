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
#include "includes.h"
#include "uds_app.h"
#include "TP.h"
#include "fls_app.h"
#include "timer_hal.h"
#include "watchdog_hal.h"
#include "boot.h"
#include "CRC_HAL.h"


void UDS_MAIN_Init(void (*pfBSP_Init)(void), void (*pfAbortTxMsg)(void))
{
#ifdef UDS_PROJECT_FOR_BOOTLOADER

    if (TRUE == Boot_IsPowerOnTriggerReset())
    {
        Boot_PowerONClearAllFlag();
    }

#ifdef EN_DELAY_TIME
    if (TRUE == POWER_SYS_GetResetSrcStatusCmd(RCM, RCM_WATCH_DOG))
#endif
    {
        Boot_JumpToAppOrNot();
    }

#endif

    if (NULL_PTR != pfBSP_Init)
    {
        (*pfBSP_Init)();
    }

    BOOTLOADER_DEBUG_Init();

    if (TRUE != CRC_HAL_Init())
    {
        APPDebugPrintf("\nCRC_HAL_Init failed!\n");
    }

    WATCHDOG_HAL_Init();
    TIMER_HAL_Init();
    TP_Init();

#ifdef UDS_PROJECT_FOR_BOOTLOADER

    if (TRUE != FLASH_HAL_APPAddrCheck())
    {
        APPDebugPrintf("\nFLASH_HAL_APPAddrCheck check error!\n");
    }

#endif

    UDS_Init();

#ifdef UDS_PROJECT_FOR_BOOTLOADER
    Boot_CheckReqBootloaderMode();
#endif

#ifdef UDS_PROJECT_FOR_APP
    Boot_CheckDownlaodAPPStatus();
#endif

    TP_RegisterAbortTxMsg(pfAbortTxMsg);
    FLASH_APP_Init();
}

void UDS_MAIN_Process(void)
{
#ifdef EN_DEBUG_IO
    static uint16 timerCnt1Ms = 0u;
#endif

    if (TRUE == TIMER_HAL_Is1msTickTimeout())
    {
        TP_SystemTickCtl();
        UDS_SystemTickCtl();
#ifdef EN_DEBUG_IO

        timerCnt1Ms++;

        if (250u == timerCnt1Ms)
        {
            timerCnt1Ms = 0u;

            /* Blink LED */
            ToggleDebugIO();
        }

#endif
    }

    if (TRUE == TIMER_HAL_Is100msTickTimeout())
    {
        /* Feed watch dog every 100ms */
        WATCHDOG_HAL_Feed();
    }

    TP_MainFun();
    UDS_MainFun();
    Flash_OperateMainFunction();
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
