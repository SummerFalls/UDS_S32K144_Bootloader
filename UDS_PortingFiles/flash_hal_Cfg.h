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

typedef struct {
    tLogicalAddr xBlockStartLogicalAddr; /*block start logical addr*/
    tLogicalAddr xBlockEndLogicalAddr;   /*block end logical addr*/
} BlockInfo_t;

/*program data buf max length*/
#define MAX_FLASH_DATA_LEN (200u)

#if 0
/*startup address*/
#define STARTUP_ADDR (0xFFFEu)
#endif

/*check flash config valid or not?*/
extern boolean FLASH_HAL_APPAddrCheck(void);

/*Get configure flash information*/
extern  boolean FLASH_HAL_GetFlashConfigInfo(const tAPPType i_appType, BlockInfo_t **o_ppBlockInfo, uint32 *o_pItemLen);

/*get application information(storage start address, block size)*/
extern  boolean FLASH_HAL_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize);

/*get secotr length*/
extern uint32 FLASH_HAL_GetFlashLengthToSectors(const uint32 i_startFlashAddr, const uint32 i_len);

/*get flash driver start and length*/
extern boolean FLASH_HAL_GetFlashDriverInfo(uint32 *o_pFlashDriverAddrStart, uint32 *o_pFlashDriverEndAddr);

/*get reset handler information*/
extern void FLASH_HAL_GetRestHanlderInfo(boolean *o_pIsEnableWriteResetHandlerInFlash, uint32 *o_pResetHanderOffset, uint32 *o_pResetHandlerLength);

/*Get storage reset handler infomation*/
extern uint32 FLASH_HAL_GetStorageRestHandlerAddr(void);

/*Is enable write reset handler in flash?*/
extern boolean FLASH_HAL_IsEnableStorageResetHandlerInFlash(void);

/*get reset handler addr length*/
extern uint32 FLASH_HAL_GetResetHandlerLen(void);

/*Get config core no*/
extern uint32 FLASH_HAL_GetConfigCoreNo(void);

/*Get config core application mirror address*/
extern boolean FLASH_HAL_GetMultiCoreMirrorAddr(const tAPPType i_appType, const uint32 i_coreNo, uint32 *o_pMirrorAddr);

/*Get core remap address*/
extern boolean FLASH_HAL_GetMultiCoreRemapAddr(const tAPPType i_appType, const uint32 i_coreNo, uint32 *o_pReampAddr);

/*logical address to physical address*/
extern uint32 FLASH_HAL_LogicalToPhysicalAddr(const uint32 i_logicalAddr);

/*global address to logical address*/
extern uint32 FLASH_HAL_PhysicalToLogicalAddr(const uint32 i_globalAddr);

/*get erase flash sector max time*/
extern uint32 FLASH_HAL_GetEraseFlashASectorMaxTimeMs(void);

/*get total how much sectors in flash*/
extern uint32 FLASH_HAL_GetTotalSectors(const tAPPType i_appType);

/*sector number to flash address*/
extern boolean FLASH_HAL_SectorNumberToFlashAddress(const tAPPType i_appType, const uint32 i_secotrNo, uint32 *o_pFlashAddr);

/*get 1 sector = bytes*/
extern uint32 FLASH_HAL_Get1SectorBytes(void);

#endif /* FLASH_HAL_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
