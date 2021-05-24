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
