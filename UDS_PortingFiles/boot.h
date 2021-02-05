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

extern void Boot_JumpToAppOrNot(void);

/*request bootloader mode check*/
extern boolean Boot_CheckReqBootloaderMode(void);

#endif /* BOOT_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
