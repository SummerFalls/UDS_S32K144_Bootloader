/*
 * @ 名称: bootloader_debug.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "bootloader_debug.h"

#ifdef EN_DEBUG_PRINT
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : BOOTLOADER_DEBUG_Init
 * Description   : This function init this module.
 *
 * Implements : BOOTLOADER_DEBUG_Init_Activity
 *END**************************************************************************/
void BOOTLOADER_DEBUG_Init(void)
{
    InitDebugIO();
    InitDebugTimer();
    InitDebugPrint();
}

#ifdef EN_DEBUG_PRINT
void Bootloader_DebugPrintInit(void)
{
    LPUART_DRV_Init(INST_LPUART1, &lpuart1_State, &lpuart1_InitConfig0);
}
void Bootloader_DebugPrint(const char *fmt, ...)
{
    static char buffer [DEBUG_LOG_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, DEBUG_LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    LPUART_DRV_SendDataBlocking(INST_LPUART1, (uint8 *)buffer, strlen(buffer), 1000);
}
#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
