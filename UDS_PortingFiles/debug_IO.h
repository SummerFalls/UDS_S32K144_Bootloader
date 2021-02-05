/*
 * @ ����: debug_IO.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef DEBUG_IO_H_
#define DEBUG_IO_H_

#include "includes.h"

#ifdef EN_DEBUG_IO

extern void DEBUG_IO_Init(void);
extern void DEBUG_IO_Deinit(void);
extern void DEBUG_IO_SetDebugIOLow(void);
extern void DEBUG_IO_SetDebugIOHigh(void);
extern void DEBUG_IO_ToggleDebugIO(void);

#endif

#endif /* DEBUG_IO_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
