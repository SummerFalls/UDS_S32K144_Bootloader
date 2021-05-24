/*
 * @ ����: boot.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef BOOT_H_
#define BOOT_H_

#include "boot_Cfg.h"

#ifdef UDS_PROJECT_FOR_BOOTLOADER

void Boot_JumpToAppOrNot(void);

boolean Boot_CheckReqBootloaderMode(void);

#endif

#ifdef UDS_PROJECT_FOR_APP

boolean Boot_CheckDownlaodAPPStatus(void);

#endif

#endif /* BOOT_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
