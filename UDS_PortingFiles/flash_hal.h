/*
 * @ 名称: flash_hal.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef FLASH_HAL_H_
#define FLASH_HAL_H_

#include "flash_hal_Cfg.h"

typedef boolean (*tpfFlashInit)(void);
typedef void (*tpfFlashDeInit)(void);
typedef boolean (*tpfEraseSecotr)(const uint32, const uint32);
typedef boolean (*tpfProgramData)(const uint32, const uint8 *, const uint32);
typedef boolean (*tpfReadFlashData)(const uint32, const uint32, uint8 *);

typedef struct
{
    tpfFlashInit pfFlashInit;
    tpfEraseSecotr pfEraserSecotr;    /* erase sector */
    tpfProgramData pfProgramData;     /* program data */
    tpfReadFlashData pfReadFlashData; /* read flash data */
    tpfFlashDeInit pfFlashDeinit;
} tFlashOperateAPI;

/*!
 * @brief To Register operate API.
 *
 * This function returns the state of the initial.
 *
 * @param[out] operate flash API
 * @return register status.
 */
boolean FLASH_HAL_RegisterFlashAPI(tFlashOperateAPI *o_pstFlashOperateAPI);

#endif /* FLASH_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
