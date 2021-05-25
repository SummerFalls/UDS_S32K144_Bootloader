/*
 * @ 名称: flash_hal_Cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef FLASH_HAL_CFG_H_
#define FLASH_HAL_CFG_H_

#include "includes.h"

typedef uint32 tLogicalAddr;

typedef struct
{
    tLogicalAddr xBlockStartLogicalAddr; /* block start logical addr */
    tLogicalAddr xBlockEndLogicalAddr;   /* block end logical addr */
} BlockInfo_t;

/* Program data buffer max length */
#define MAX_FLASH_DATA_LEN (200u)

/* Check flash config valid or not? */
boolean FLASH_HAL_APPAddrCheck(void);

/* Get configure flash information */
boolean FLASH_HAL_GetFlashConfigInfo(const tAPPType i_appType, BlockInfo_t **o_ppBlockInfo, uint32 *o_pItemLen);

/* Get application information(storage start address, block size) */
boolean FLASH_HAL_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize);

/* Get sector length */
uint32 FLASH_HAL_GetFlashLengthToSectors(const uint32 i_startFlashAddr, const uint32 i_len);

/* Get flash driver start and length */
boolean FLASH_HAL_GetFlashDriverInfo(uint32 *o_pFlashDriverAddrStart, uint32 *o_pFlashDriverEndAddr);

/* Get reset handler information */
void FLASH_HAL_GetResetHandlerInfo(boolean *o_pIsEnableWriteResetHandlerInFlash, uint32 *o_pResetHandlerOffset, uint32 *o_pResetHandlerLength);

/* Is enable write reset handler in flash? */
boolean FLASH_HAL_IsEnableStorageResetHandlerInFlash(void);

/* Get storage reset handler information */
uint32 FLASH_HAL_GetStorageRestHandlerAddr(void);

/* Get reset handler addr length */
uint32 FLASH_HAL_GetResetHandlerLen(void);

/* Get config core no */
uint32 FLASH_HAL_GetConfigCoreNo(void);

/* Get config core application mirror address */
boolean FLASH_HAL_GetMultiCoreMirrorAddr(const tAPPType i_appType, const uint32 i_coreNo, uint32 *o_pMirrorAddr);

/* Get core remap address */
boolean FLASH_HAL_GetMultiCoreRemapAddr(const tAPPType i_appType, const uint32 i_coreNo, uint32 *o_pReampAddr);

/* Logical address to physical address */
uint32 FLASH_HAL_LogicalToPhysicalAddr(const uint32 i_logicalAddr);

/* Global address to logical address */
uint32 FLASH_HAL_PhysicalToLogicalAddr(const uint32 i_globalAddr);

/* Get erase flash sector max time */
uint32 FLASH_HAL_GetEraseFlashASectorMaxTimeMs(void);

/* Get total how much sectors in flash */
uint32 FLASH_HAL_GetTotalSectors(const tAPPType i_appType);

/* Sector number to flash address */
boolean FLASH_HAL_SectorNumberToFlashAddress(const tAPPType i_appType, const uint32 i_secotrNo, uint32 *o_pFlashAddr);

/* Get 1 sector = bytes */
uint32 FLASH_HAL_Get1SectorBytes(void);

#endif /* FLASH_HAL_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
