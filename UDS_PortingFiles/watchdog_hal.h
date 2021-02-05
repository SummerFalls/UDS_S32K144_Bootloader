/*
 * @ ����: watchdog_hal.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef WATCHDOG_HAL_H_
#define WATCHDOG_HAL_H_

#include "includes.h"

extern void WATCHDOG_HAL_Init(void);

extern void WATCHDOG_HAL_Fed(void);

extern void WATCHDOG_HAL_SystemRest(void);

extern void WATCHDOG_HAL_Deinit(void);

#endif /* WATCHDOG_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
