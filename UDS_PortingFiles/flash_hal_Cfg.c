/*
 * @ 名称: flash_hal_Cfg.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "flash_hal_Cfg.h"

/*define a sector = bytes*/
#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K14x)
#define SECTOR_LEN (4096u)
#elif (defined MCU_TYPE) && (MCU_TYPE == MCU_S12Z)
#define SECTOR_LEN (512u)
#elif (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K11x)
#define SECTOR_LEN (2048u)
#else
#error "Please config the MCU Type"
#endif

/*reset handler information*/
#define EN_WRITE_RESET_HANDLER_IN_FLASH (FALSE)  /*enable write reset handler in flash or not*/


typedef struct {
    uint32 imageAStartAddr;
    uint32 imageBStartAddr;
    uint32 imageAMirrorAddr;
    uint32 imageBMirrorAddr;
    uint32 remapApplicationAddr;
} CoreInfo_t;

/*
define eras flash sector max time
**************************
S32K142/144/146/148     -- max erase time 130ms
MagniV S12ZVL/S32ZVM   --                       21ms
*/
#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K14x)
#define MAX_ERASE_SECTOR_FLASH_MS (130u)
#endif

#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K11x)
#define MAX_ERASE_SECTOR_FLASH_MS (130u)
#endif


#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S12Z)
#define MAX_ERASE_SECTOR_FLASH_MS (21u)
#endif

/*define vector table offset and reset handle offset*/
#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K14x)
#define APP_VECTOR_TABLE_OFFSET (0x200u) /*vector table offset from gs_astBlockNumA/B*/
#define RESET_HANDLE_OFFSET (4u)        /*from top vector table to reset handle*/
#define RESET_HANDLER_ADDR_LEN (4u)     /*pointer length or reset hanlder length*/
#endif

/*define vector table offset and reset handle offset*/
#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S32K11x)
#define APP_VECTOR_TABLE_OFFSET (0x200u) /*vector table offset from gs_astBlockNumA/B*/
#define RESET_HANDLE_OFFSET (4u)        /*from top vector table to reset handle*/
#define RESET_HANDLER_ADDR_LEN (4u)     /*pointer length or reset hanlder length*/
#endif

/*define vector table offset and reset handle offset*/
#if  (defined MCU_TYPE) && (MCU_TYPE == MCU_S12Z)
#define APP_VECTOR_TABLE_OFFSET (512u) /*vector table offset from gs_astBlockNumA/B*/
#define RESET_HANDLE_OFFSET (508u)     /*from top vector table to reset handle 508 = 127 * 4*/
#define RESET_HANDLER_ADDR_LEN (3u)    /*pointer length or reset hanlder length*/
#endif


/*flash driver config*/
const BlockInfo_t gs_astFlashDriverBlock[] = {
    {0x1FFF8000u, 0x1FFF8800u},
};

/*application can used space*/
const BlockInfo_t gs_astBlockNumA[] = {
    {0x00014000u, 0x00080000u},    /*block logical 0*/
};

/*logical num*/
const uint32 gs_blockNumA = sizeof(gs_astBlockNumA) / sizeof(gs_astBlockNumA[0u]);

#ifdef EN_SUPPORT_APP_B
/*application can used space*/
const BlockInfo_t gs_astBlockNumB[] = {
    0x00080000u, 0x000EF000u
};

/*logical num*/
const uint32 gs_blockNumB = sizeof(gs_astBlockNumB) / sizeof(gs_astBlockNumB[0u]);
#endif

/*multi-core configure*/
#if (CORE_NO >= 1u)
static const CoreInfo_t gs_astMultiCoreAPPRemapInfo[CORE_NO] = {
    {
        /*imageAStartAddr, imageBStartAddr, imageAMirrorAddr, imageBMirrorAddr, remapApplicationAddr*/
        0x1000000u, 0x1200000u, 0xA000000u, 0xA200000u, 0x2000000u
    },

};
#endif

/*check app flash config valid or not?*/
boolean FLASH_HAL_APPAddrCheck(void)
{
    const uint32 flashAddrLowByte = (SECTOR_LEN) - 1u;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32 item = 0u;


    if (TRUE == FLASH_HAL_GetFlashConfigInfo(APP_A_TYPE, &pBlockInfo, &item)) {
        while (item) {
            if ((0u != (pBlockInfo->xBlockStartLogicalAddr & flashAddrLowByte)) ||
                    (0u != (pBlockInfo->xBlockEndLogicalAddr & flashAddrLowByte))) {
                return FALSE;
            }

            item--;
            pBlockInfo++;
        }
    }

#ifdef EN_SUPPORT_APP_B

    if (TRUE == FLASH_HAL_GetFlashConfigInfo(APP_B_TYPE, &pBlockInfo, &item)) {
        while (item) {
            if ((0u != (pBlockInfo->xBlockStartLogicalAddr & flashAddrLowByte)) ||
                    (0u != (pBlockInfo->xBlockEndLogicalAddr & flashAddrLowByte))) {
                return FALSE;
            }

            item--;
            pBlockInfo++;
        }
    }

#endif


    return TRUE;
}


/*Get config core no*/
uint32 FLASH_HAL_GetConfigCoreNo(void)
{
    return CORE_NO;
}

/*Get config core application mirror address*/
boolean FLASH_HAL_GetMultiCoreMirrorAddr(const tAPPType i_appType, const uint32 i_coreNo, uint32 *o_pMirrorAddr)
{
    boolean result = FALSE;

#if (CORE_NO >= 1)

    if ((APP_A_TYPE == i_appType) && (i_coreNo < CORE_NO)) {
        *o_pMirrorAddr = gs_astMultiCoreAPPRemapInfo[i_coreNo].imageAMirrorAddr;

        result = TRUE;
    } else {
#ifdef EN_SUPPORT_APP_B

        if ((APP_B_TYPE == i_appType) && (i_coreNo < CORE_NO)) {
            *o_pMirrorAddr = gs_astMultiCoreAPPRemapInfo[i_coreNo].imageBMirrorAddr;

            result = TRUE;
        }

#endif /*#ifdef EN_SUPPORT_APP_B*/
    }

#endif /*#if (CORE_NO >= 1)*/

    return result;
}

/*Get core remap address*/
boolean FLASH_HAL_GetMultiCoreRemapAddr(const tAPPType i_appType, const uint32 i_coreNo, uint32 *o_pReampAddr)
{
    boolean result = FALSE;

#if (CORE_NO >= 1)

    if ((APP_A_TYPE == i_appType) && (i_coreNo < CORE_NO)) {
        *o_pReampAddr = gs_astMultiCoreAPPRemapInfo[i_coreNo].remapApplicationAddr;

        result = TRUE;
    } else {
#ifdef EN_SUPPORT_APP_B

        if ((APP_B_TYPE == i_appType) && (i_coreNo < CORE_NO)) {
            *o_pReampAddr = gs_astMultiCoreAPPRemapInfo[i_coreNo].remapApplicationAddr;

            result = TRUE;
        }

#endif /*#ifdef EN_SUPPORT_APP_B*/
    }

#endif /*#if (CORE_NO >= 1)*/

    return result;
}

boolean FLASH_HAL_GetFlashConfigInfo(const tAPPType i_appType,
                                     BlockInfo_t **o_ppBlockInfo,
                                     uint32 *o_pItemLen)
{
    boolean result = FALSE;

    if (APP_A_TYPE == i_appType) {
        *o_ppBlockInfo = (BlockInfo_t *)gs_astBlockNumA;
        *o_pItemLen = gs_blockNumA;

        result = TRUE;
    } else {
#ifdef EN_SUPPORT_APP_B

        if (APP_B_TYPE == i_appType) {
            *o_ppBlockInfo = (BlockInfo_t *)gs_astBlockNumB;
            *o_pItemLen = gs_blockNumB;

            result = TRUE;
        }

#endif
    }


    return result;
}


boolean FLASH_HAL_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize)
{
    boolean result = FALSE;

    if (APP_A_TYPE == i_appType) {
        *o_pAppInfoStartAddr = gs_astBlockNumA[0u].xBlockStartLogicalAddr;
        *o_pBlockSize = gs_astBlockNumA[0u].xBlockEndLogicalAddr - gs_astBlockNumA[0u].xBlockStartLogicalAddr;

        result = TRUE;
    } else {
#ifdef EN_SUPPORT_APP_B

        if (APP_B_TYPE == i_appType) {
            *o_pAppInfoStartAddr = gs_astBlockNumB[0u].xBlockStartLogicalAddr;
            *o_pBlockSize = gs_astBlockNumB[0u].xBlockEndLogicalAddr - gs_astBlockNumB[0u].xBlockStartLogicalAddr;

            result = TRUE;
        }

#endif
    }

    return result;
}

/*get 1 sector = bytes*/
uint32 FLASH_HAL_Get1SectorBytes(void)
{
    return SECTOR_LEN;
}


/*get flash length to sectors*/
uint32 FLASH_HAL_GetFlashLengthToSectors(const uint32 i_startFlashAddr, const uint32 i_len)
{
    uint32 sectorNo = 0u;
    const uint32 flashAddrLowByte = (SECTOR_LEN) - 1u;
    uint32 flashAddrTmp = 0u;

    flashAddrTmp = (i_startFlashAddr & flashAddrLowByte);

    if (i_len <= SECTOR_LEN) {
        flashAddrTmp += i_len;

        if (flashAddrTmp <= SECTOR_LEN) {
            sectorNo = 1u;
        } else {
            sectorNo = 2u;
        }
    } else {
        sectorNo = i_len / SECTOR_LEN;

        if (0u != (i_len & flashAddrLowByte)) {
            sectorNo += 1u;
        }

        if ((0u != flashAddrTmp) && (flashAddrTmp != ((flashAddrTmp + i_len) & flashAddrLowByte))) {
            sectorNo += 1u;
        }
    }

    return sectorNo;
}

/*get flash driver start and length*/
boolean FLASH_HAL_GetFlashDriverInfo(uint32 *o_pFlashDriverAddrStart, uint32 *o_pFlashDriverEndAddr)
{
    ASSERT(NULL_PTR == o_pFlashDriverAddrStart);
    ASSERT(NULL_PTR == o_pFlashDriverEndAddr);


    *o_pFlashDriverAddrStart = gs_astFlashDriverBlock[0u].xBlockStartLogicalAddr;
    *o_pFlashDriverEndAddr = gs_astFlashDriverBlock[0u].xBlockEndLogicalAddr;

    return TRUE;
}

/*get reset handler information*/
void FLASH_HAL_GetRestHanlderInfo(boolean *o_pIsEnableWriteResetHandlerInFlash, uint32 *o_pResetHanderOffset, uint32 *o_pResetHandlerLength)
{
    ASSERT(NULL_PTR == o_pIsEnableWriteResetHandlerInFlash);
    ASSERT(NULL_PTR == o_pResetHanderOffset);
    ASSERT(NULL_PTR == o_pResetHandlerLength);

    *o_pIsEnableWriteResetHandlerInFlash = EN_WRITE_RESET_HANDLER_IN_FLASH;
    *o_pResetHanderOffset = APP_VECTOR_TABLE_OFFSET + RESET_HANDLE_OFFSET;
    *o_pResetHandlerLength = RESET_HANDLER_ADDR_LEN;
}

/*Get storage reset handler infomation*/
uint32 FLASH_HAL_GetStorageRestHandlerAddr(void)
{
    return APP_VECTOR_TABLE_OFFSET + RESET_HANDLE_OFFSET;
}

/*Is enable write reset handler in flash?*/
boolean FLASH_HAL_IsEnableStorageResetHandlerInFlash(void)
{
    return EN_WRITE_RESET_HANDLER_IN_FLASH;
}

/*get reset handler addr length*/
uint32 FLASH_HAL_GetResetHandlerLen(void)
{
    return RESET_HANDLER_ADDR_LEN;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_LogicalToPhysicalAddr
 * Description   : This function is logical address to physical address.
 *
 *END**************************************************************************/
uint32 FLASH_HAL_LogicalToPhysicalAddr(const uint32 i_logicalAddr)
{
    uint32 globalAddr = 0u;


    globalAddr = (uint32)i_logicalAddr;

    return globalAddr;

}

/*physical address to logical address*/
uint32 FLASH_HAL_PhysicalToLogicalAddr(const uint32 i_globalAddr)
{
    uint32 logicalAddr = 0u;


    logicalAddr = (uint32)i_globalAddr;

    return logicalAddr;
}

/*get erase flash sector max time*/
uint32 FLASH_HAL_GetEraseFlashASectorMaxTimeMs(void)
{
    return MAX_ERASE_SECTOR_FLASH_MS;
}

/*get total how much sectors in flash*/
uint32 FLASH_HAL_GetTotalSectors(const tAPPType i_appType)
{
    uint32 sectors = 0u;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32 itemNo = 0u;
    uint32 flashLength = 0u;
    uint32 index = 0u;

    if (TRUE == FLASH_HAL_GetFlashConfigInfo(i_appType, &pBlockInfo, &itemNo)) {
        for (index = 0u; index < itemNo; index++) {
            flashLength = pBlockInfo[index].xBlockEndLogicalAddr - pBlockInfo[index].xBlockStartLogicalAddr;
            sectors += FLASH_HAL_GetFlashLengthToSectors(pBlockInfo[index].xBlockEndLogicalAddr, flashLength);
        }
    }

    return sectors;
}

/*sector number to flash address*/
boolean FLASH_HAL_SectorNumberToFlashAddress(const tAPPType i_appType, const uint32 i_secotrNo, uint32 *o_pFlashAddr)
{
    boolean result = FALSE;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32 itemNo = 0u;
    uint32 totalSectors = 0u;
    uint32 index = 0u;

    uint32 sectorsTmp = 0u;
    const uint32 flashAddrLowByte = (SECTOR_LEN) - 1u;
    uint32 flashAddrTmp = 0u;

    if (TRUE == FLASH_HAL_GetFlashConfigInfo(i_appType, &pBlockInfo, &itemNo)) {
        totalSectors = FLASH_HAL_GetTotalSectors(i_appType);

        if (i_secotrNo < totalSectors) {
            sectorsTmp = 0u;

            while (index < itemNo) {
                flashAddrTmp = pBlockInfo[index].xBlockStartLogicalAddr & flashAddrLowByte;

                if (flashAddrTmp) {
                    sectorsTmp += 1u;
                }

                flashAddrTmp = pBlockInfo[index].xBlockStartLogicalAddr - flashAddrTmp;

                while (flashAddrTmp < pBlockInfo[index].xBlockEndLogicalAddr) {
                    if (sectorsTmp == i_secotrNo) {
                        *o_pFlashAddr = flashAddrTmp;

                        result = TRUE;

                        break;
                    }

                    flashAddrTmp += SECTOR_LEN;

                    sectorsTmp++;
                }

                if (TRUE == result) {
                    break;
                }

                index++;
            }

        } else {
            result = FALSE;
        }
    }

    return result;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
