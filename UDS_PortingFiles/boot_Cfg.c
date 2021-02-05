/*
 * @ 名称: boot_Cfg.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "boot_Cfg.h"
#include "flash_hal_cfg.h"
#include "fls_app.h"
#include "CRC_hal.h"

typedef struct {
    uint8 infoDataLen;             /*Exchange inforamtion length must N * 4.*/
    uint8 requestEnterBootloader;  /*Request enter bootloader mode flag*/
    uint8 downloadAPPSuccessful;   /*downlaod APP successful flag*/
    uint32 infoStartAddr;          /*exchange information start address*/
    uint32 requestEnterBootloaderAddr; /*Request enter bootloader flag address */
    uint32 downloadAppSuccessfulAddr;  /*download APP successful flag address*/
} tBootInfo;

static const tBootInfo gs_stBootInfo = {
    16u,    /*Exchange inforamtion length must N * 4.*/
    0x5Au,
    0xA5u,
    0x20006FF0u,
    0x20006FF1u,
    0x20006FF0u,
};

/*get information storage CRC*/
#define GetInfoStorageCRC() (*(uint16 *)(gs_stBootInfo.infoStartAddr + 14))

/*set information CRC */
#define SetInforCRC(xCrc) ((*(uint16 *)(gs_stBootInfo.infoStartAddr + 14)) = (uint16)(xCrc))

/*Is information valid?*/
static boolean Boot_IsInfoValid(void);

/*calculate information CRC*/
static uint16 Boot_CalculateInfoCRC(void);

/*set download app successful */
void SetDownloadAppSuccessful(void)
{
    uint16 infoCrc = 0u;

    *((uint8 *)gs_stBootInfo.downloadAppSuccessfulAddr) = gs_stBootInfo.downloadAPPSuccessful;

    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/*Is request enter bootloader?*/
boolean IsRequestEnterBootloader(void)
{
    boolean result = FALSE;

    if (TRUE == Boot_IsInfoValid()) {
        if (gs_stBootInfo.requestEnterBootloader == *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr)) {
            result = TRUE;
        }
    }

    return result;
}

/*clear request enter bootloader flag*/
void ClearRequestEnterBootloaderFlag(void)
{
    uint16 infoCrc = 0u;

    *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr) = 0u;

    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/*Is power on trigger reset?*/
boolean Boot_IsPowerOnTriggerReset(void)
{
    boolean result = FALSE;


    return result;
}

/*when power on, clear all flag in RAM for ECC.*/
void Boot_PowerONClearAllFlag(void)
{
    uint16 infoCrc = 0u;
    uint8 index = 0u;

    /*clear RAM with 4 bytes for ECC*/
    for (index = 0u; index < (gs_stBootInfo.infoDataLen >> 2u); index++) {
        *((uint32 *)gs_stBootInfo.infoStartAddr + index) = 0u;
    }

    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/*remap multi-core application*/
void Boot_RemapApplication(void)
{
    uint32 totalCoreNo = 0u;
    uint32 index = 0u;
    tAPPType appType = APP_A_TYPE;
    uint32 appMirrorAddr = 0u;
    uint32 appRemapAddr = 0u;

    totalCoreNo = FLASH_HAL_GetConfigCoreNo();

    if (totalCoreNo > 0u) {
        appType = Flash_GetNewestAPPType();

        for (index = 0u; index < totalCoreNo; index++) {
            if ((TRUE == FLASH_HAL_GetMultiCoreMirrorAddr(appType, index, &appMirrorAddr)) &&
                    (TRUE == FLASH_HAL_GetMultiCoreRemapAddr(appType, index, &appRemapAddr))) {
                /*do remap multi core application*/
            } else {
                /*trigger MCU reset*/
            }
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Boot_JumpToApp
 * Description   : This function is jump to app.
 *
 * Implements :
 *END**************************************************************************/
typedef void (*AppAddr)(void);
AppAddr JumpAppAddr = NULL;
void Boot_JumpToApp(const uint32 i_AppAddr)
{
    AppAddr resetHandle = (AppAddr)(i_AppAddr);

    (resetHandle)();

#if 0
    /*example for CodeWarroir -- MagniV*/
    AppAddr appAddr;

    __asm{
        LD D6, i_AppAddr
        TFR D6, X
        ST X, appAddr
        JSR [appAddr]
    }
#endif
}

/*Is information valid?*/
static boolean Boot_IsInfoValid(void)
{
    uint16 infoCrc = 0u;
    uint16 storageCrc = 0u;
    boolean result = FALSE;

    infoCrc = Boot_CalculateInfoCRC();

    storageCrc = GetInfoStorageCRC();

    if (storageCrc == infoCrc) {
        result = TRUE;
    }

    return result;
}

/*calculate information CRC*/
static uint16 Boot_CalculateInfoCRC(void)
{
    uint32 infoCrc = 0u;

    CRC_HAL_CreatSoftwareCrc((const uint8 *)gs_stBootInfo.infoStartAddr, gs_stBootInfo.infoDataLen - 2u, &infoCrc);

    return (uint16)infoCrc;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
