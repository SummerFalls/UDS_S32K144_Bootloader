/*
 * @ 名称: boot.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "boot.h"
#include "includes.h"
#include "fls_app.h"
#include "uds_app.h"

#ifdef UDS_PROJECT_FOR_BOOTLOADER

static boolean Boot_IsAPPValid(void)
{
    boolean bResult = FALSE;

    /* Check APP code flash status */
    bResult = Flash_IsReadAppInfoFromFlashValid();

    if (TRUE == bResult)
    {
        bResult = Flash_IsAppInFlashValid();
    }

    return bResult;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Boot_JumpToAppOrNot
 * Description   : This function is jump to APP or not. If APP valid and APP not request enter bootloader, then jump to APP.
 *                 Some MCU call this function in startup, because 'JumpToAPP' is need to be called before peripheral initialization (clock...).
 *
 * Implements :
 *END*************************************************************************/
void Boot_JumpToAppOrNot(void)
{
    uint32 resetHandlerAddr = 0u;

    if ((TRUE == Boot_IsAPPValid()) && (TRUE != IsRequestEnterBootloader()))
    {
        Boot_RemapApplication();
        resetHandlerAddr = Flash_GetResetHandlerAddr();
        Boot_JumpToApp(resetHandlerAddr);
    }
}

/* Request bootloader mode check */
boolean Boot_CheckReqBootloaderMode(void)
{
    boolean ret = FALSE;

    if (TRUE == IsRequestEnterBootloader())
    {
        ClearRequestEnterBootloaderFlag();

        /* Write a message to host based on TP */
        if (TRUE == UDS_TxMsgToHost())
        {
            ret = TRUE;
            APPDebugPrintf("\nEnter bootloader mode\n");
        }
        else
        {
            APPDebugPrintf("\nEnter bootloader mode and transmit confirm message failed!\n");
        }
    }

    return ret;
}

#endif

#ifdef UDS_PROJECT_FOR_APP

boolean Boot_CheckDownlaodAPPStatus(void)
{
    boolean ret = FALSE;

    if (TRUE == IsDownloadAPPSccessful())
    {
        ClearDownloadAPPSuccessfulFlag();

        /* Write a message to host based on TP */
        if (TRUE == UDS_TxMsgToHost())
        {
            ret = TRUE;
            APPDebugPrintf("\nDownlaod APP successful!\n");
        }
        else
        {
            APPDebugPrintf("\nEnter APP mode and transmit confirm message failed!\n");
        }
    }

    return ret;
}

#endif
/* -------------------------------------------- END OF FILE -------------------------------------------- */
