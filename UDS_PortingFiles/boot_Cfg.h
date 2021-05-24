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


#ifdef UDS_PROJECT_FOR_BOOTLOADER

void SetDownloadAppSuccessful(void);

boolean IsRequestEnterBootloader(void);

void ClearRequestEnterBootloaderFlag(void);

void Boot_JumpToApp(const uint32 i_AppAddr);

void Boot_RemapApplication(void);

void Boot_PowerONClearAllFlag(void);

boolean Boot_IsPowerOnTriggerReset(void);
#endif

#ifdef UDS_PROJECT_FOR_APP

void RequestEnterBootloader(void);

boolean IsDownloadAPPSccessful(void);

void ClearDownloadAPPSuccessfulFlag(void);
#endif

#endif /* BOOT_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
