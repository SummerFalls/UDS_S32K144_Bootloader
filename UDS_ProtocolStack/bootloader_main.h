/*
 * @ 名称: bootloader_main.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef BOOTLOADER_MAIN_H_
#define BOOTLOADER_MAIN_H_

#include "includes.h"

extern void BOOTLOADER_MAIN_Init(void (*pfBSP_Init)(void), void (*pfAbortTxMsg)(void));

/*Bootloader main demo*/
extern void BOOTLOADER_MAIN_Demo(void);

void BOOTLOADER_MAIN_Deinit(void);

#endif /* BOOTLOADER_MAIN_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
