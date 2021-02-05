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

extern void Boot_JumpToAppOrNot(void);

/*request bootloader mode check*/
extern boolean Boot_CheckReqBootloaderMode(void);

#endif /* BOOT_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
