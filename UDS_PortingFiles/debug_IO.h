/*
 * @ 名称: debug_IO.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef DEBUG_IO_H_
#define DEBUG_IO_H_

#include "includes.h"

#ifdef EN_DEBUG_IO

void DEBUG_IO_Init(void);
void DEBUG_IO_Deinit(void);
void DEBUG_IO_SetDebugIOLow(void);
void DEBUG_IO_SetDebugIOHigh(void);
void DEBUG_IO_ToggleDebugIO(void);

#endif

#endif /* DEBUG_IO_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
