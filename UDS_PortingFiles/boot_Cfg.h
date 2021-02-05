/*
 * @ 名称: boot_Cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
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
