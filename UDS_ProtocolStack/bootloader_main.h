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

void UDS_MAIN_Init(void (*pfBSP_Init)(void), void (*pfAbortTxMsg)(void));
void UDS_MAIN_Process(void);

#endif /* BOOTLOADER_MAIN_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
