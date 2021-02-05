/*
 * @ ����: boot_Cfg.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef BOOT_CFG_H_
#define BOOT_CFG_H_

#include "includes.h"

/*set download app successful */
extern void SetDownloadAppSuccessful(void);

/*Is request enter bootloader?*/
extern boolean IsRequestEnterBootloader(void);

/*clear request enter bootloader flag*/
extern void ClearRequestEnterBootloaderFlag(void);

/*Jump to APP.*/
extern void Boot_JumpToApp(const uint32 i_AppAddr);

/*remap multi-core application*/
extern void Boot_RemapApplication(void);

/*when power on, clear all flag in RAM for ECC.*/
extern void Boot_PowerONClearAllFlag(void);

/*Is power on trigger reset?*/
extern boolean Boot_IsPowerOnTriggerReset(void);

#endif /* BOOT_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
