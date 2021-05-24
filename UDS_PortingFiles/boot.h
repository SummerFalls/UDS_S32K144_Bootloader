/*
 * @ 名称: boot.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
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
