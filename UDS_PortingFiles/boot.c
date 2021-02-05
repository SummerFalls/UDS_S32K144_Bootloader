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

static boolean Boot_IsAPPValid(void)
{
    boolean bResult = FALSE;

    /*check app code flash status. If app code update successfull, this api return TRUE, else return FALSE.*/
    bResult = Flash_IsReadAppInfoFromFlashValid();

    if(TRUE == bResult)
    {
        bResult = Flash_IsAppInFlashValid();
    }

    return bResult;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : Boot_JumpToAppOrNot
 * Description   : This function is jump to app or not. If app valid and APP not request enter bootloader, then jump to APP.
 *               Some MCU call this function need in startup, because jump to APP need to before init Peripheral (clock...).
 *
 * Implements :
 *END*************************************************************************/
void Boot_JumpToAppOrNot(void)
{
    uint32 resetHandlerAddr = 0u;

    if((TRUE == Boot_IsAPPValid()) && (TRUE != IsRequestEnterBootloader()))
    {
        Boot_RemapApplication();

        resetHandlerAddr = Flash_GetResetHandlerAddr();

        Boot_JumpToApp(resetHandlerAddr);
    }
}

/*request bootloader mode check*/
boolean Boot_CheckReqBootloaderMode(void)
{
    boolean ret = FALSE;

    if(TRUE == IsRequestEnterBootloader())
    {
        ClearRequestEnterBootloaderFlag();

        /*write a message to host based on TP*/
        if(TRUE == UDS_TxMsgToHost())
        {
            ret = TRUE;
            APPDebugPrintf("\n Enter bootloader mode\n");
        }
        else
        {
            APPDebugPrintf("\n Enter bootloader mode and transmit confirm message failed!\n");
        }
    }

    return ret;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
