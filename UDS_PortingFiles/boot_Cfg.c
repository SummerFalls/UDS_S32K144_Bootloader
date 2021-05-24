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

typedef struct
{
    uint8 infoDataLen;                  /* Exchange information length must be N * 4 */
    uint8 requestEnterBootloader;       /* Request enter bootloader mode flag */
    uint8 downloadAPPSuccessful;        /* Download APP successful flag */
    uint32 infoStartAddr;               /* Exchange information start address */
    uint32 requestEnterBootloaderAddr;  /* Request enter bootloader flag address */
    uint32 downloadAppSuccessfulAddr;   /* Download APP successful flag address */
} tBootInfo;

static const tBootInfo gs_stBootInfo =
{
    16u,
    0x5Au,
    0xA5u,
    INFO_START_ADDR,
    REQUEST_ENTER_BOOTLOADER_ADDR,
    DOWNLOAD_APP_SUCCESSFUL_ADDR,
};

/* Get information storage CRC */
#define GetInfoStorageCRC() (*(uint16 *)(gs_stBootInfo.infoStartAddr + 14))

/* Set information CRC */
#define SetInforCRC(xCrc) ((*(uint16 *)(gs_stBootInfo.infoStartAddr + 14)) = (uint16)(xCrc))

/* Is information valid? */
static boolean Boot_IsInfoValid(void);

/* Calculate information CRC */
static uint16 Boot_CalculateInfoCRC(void);


#ifdef UDS_PROJECT_FOR_BOOTLOADER
/* Set download APP successful */
void SetDownloadAppSuccessful(void)
{
    uint16 infoCrc = 0u;
    *((uint8 *)gs_stBootInfo.downloadAppSuccessfulAddr) = gs_stBootInfo.downloadAPPSuccessful;
    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/* Is request enter bootloader? */
boolean IsRequestEnterBootloader(void)
{
    boolean result = FALSE;

    if (TRUE == Boot_IsInfoValid())
    {
        if (gs_stBootInfo.requestEnterBootloader == *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr))
        {
            result = TRUE;
        }
    }

    return result;
}

/* Clear request enter bootloader flag */
void ClearRequestEnterBootloaderFlag(void)
{
    uint16 infoCrc = 0u;
    *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr) = 0u;
    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/* Is power on trigger reset?(POR) */
boolean Boot_IsPowerOnTriggerReset(void)
{
    boolean result = TRUE;
    /* TODO Bootloader: #06 必须执行，否则在刷入一次APP后无法再次刷入 */
    result = POWER_SYS_GetResetSrcStatusCmd(RCM, RCM_POWER_ON);
    return result;
}

/* When power on, clear all flag in RAM for ECC */
void Boot_PowerONClearAllFlag(void)
{
    uint16 infoCrc = 0u;
    uint8 index = 0u;

    /* Clear RAM with 4 bytes for ECC */
    for (index = 0u; index < (gs_stBootInfo.infoDataLen >> 2u); index++)
    {
        *(((uint32 *)gs_stBootInfo.infoStartAddr) + index) = 0u;
    }

    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/* Remap multi-core application */
void Boot_RemapApplication(void)
{
    uint32 totalCoreNo = 0u;
    uint32 index = 0u;
    tAPPType appType = APP_A_TYPE;
    uint32 appMirrorAddr = 0u;
    uint32 appRemapAddr = 0u;
    totalCoreNo = FLASH_HAL_GetConfigCoreNo();

    if (totalCoreNo > 0u)
    {
        appType = Flash_GetNewestAPPType();

        for (index = 0u; index < totalCoreNo; index++)
        {
            if ((TRUE == FLASH_HAL_GetMultiCoreMirrorAddr(appType, index, &appMirrorAddr)) &&
                    (TRUE == FLASH_HAL_GetMultiCoreRemapAddr(appType, index, &appRemapAddr)))
            {
                /* Do remap multi-core application */
            }
            else
            {
                /* Trigger MCU reset */
            }
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Boot_JumpToApp
 * Description   : This function is jump to APP.
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
    /* Example for CodeWarroir - MagniV */
    AppAddr appAddr;
    __asm
    {
        LD D6, i_AppAddr
        TFR D6, X
        ST X, appAddr
        JSR [appAddr]
    }
#endif
}
#endif

#ifdef UDS_PROJECT_FOR_APP
void RequestEnterBootloader(void)
{
    uint16 infoCrc = 0u;
    *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr) = gs_stBootInfo.requestEnterBootloader;
    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/* Is download APP successful? */
boolean IsDownloadAPPSccessful(void)
{
    boolean result = FALSE;

    if (TRUE == Boot_IsInfoValid())
    {
        if (gs_stBootInfo.downloadAPPSuccessful == *((uint8 *)gs_stBootInfo.downloadAppSuccessfulAddr))
        {
            result = TRUE;
        }
    }

    return result;
}

/* Clear download APP successful flag */
void ClearDownloadAPPSuccessfulFlag(void)
{
    uint16 infoCrc = 0u;
    *((uint8 *)gs_stBootInfo.downloadAppSuccessfulAddr) = 0u;
    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}
#endif

/* Is information valid? */
static boolean Boot_IsInfoValid(void)
{
    uint16 infoCrc = 0u;
    uint16 storageCrc = 0u;
    boolean result = FALSE;
    infoCrc = Boot_CalculateInfoCRC();
    storageCrc = GetInfoStorageCRC();

    if (storageCrc == infoCrc)
    {
        result = TRUE;
    }

    return result;
}

/* Calculate information CRC */
static uint16 Boot_CalculateInfoCRC(void)
{
    uint32 infoCrc = 0u;
    CRC_HAL_CreatSoftwareCrc((const uint8 *)gs_stBootInfo.infoStartAddr, gs_stBootInfo.infoDataLen - 2u, &infoCrc);
    return (uint16)infoCrc;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
