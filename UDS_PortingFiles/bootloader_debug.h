/*
 * @ 名称: bootloader_debug.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef BOOTLOADER_DEBUG_H_
#define BOOTLOADER_DEBUG_H_

#include "includes.h"

#ifdef EN_DEBUG_IO
#include "debug_IO.h"
#endif

#ifdef EN_DEBUG_TIMER
#include "debug_timer.h"
#endif

#ifdef EN_DEBUG_IO
#define InitDebugIO() DEBUG_IO_Init()
#define SetDebugIOHigh() DEBUG_IO_SetDebugIOHigh()
#define SetDebugIOLow() DEBUG_IO_SetDebugIOLow()
#define ToggleDebugIO() DEBUG_IO_ToggleDebugIO()
#else
#define InitDebugIO()
#define SetDebugIOHigh()
#define SetDebugIOLow()
#define ToggleDebugIO()
#endif

#ifdef EN_DEBUG_TIMER
#define InitDebugTimer() DEBUG_TIMER_Init()
#define GetDegbuTimerValue() DEBUG_TIMER_GetTimerValue()
#else
#define InitDebugTimer()
#define GetDegbuTimerValue()
#endif

#ifdef EN_DEBUG_PRINT
#define DEBUG_LOG_BUF_SIZE (200u)
void Bootloader_DebugPrintInit(void);
void Bootloader_DebugPrint(const char *fmt, ...);

#define InitDebugPrint() Bootloader_DebugPrintInit()
//LPUART_DRV_Init(INST_LPUART1, &lpuart1_State, &lpuart1_InitConfig0);
//#define DebugPrintf(fmt, args...) Bootloader_DebugPrint(fmt, ##args)
#define DebugPrintf Bootloader_DebugPrint
#else
/* Some MCUs can't call printf lib. E.g., S12Gxxx */
#define InitDebugPrint()
#define DebugPrintf(...)
#endif

void BOOTLOADER_DEBUG_Init(void);

#endif /* BOOTLOADER_DEBUG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
