/*
 * @ ����: bootloader_main.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
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
