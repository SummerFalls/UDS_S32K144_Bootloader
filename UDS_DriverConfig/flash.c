/*
 * @ 名称: flash.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "flash.h"
#include "flash_hal_Cfg.h"

/*! @brief Configuration structure flashCfg_0 */
static const flash_user_config_t Flash_InitConfig0 =
{
    .PFlashBase  = 0x00000000U,                        /* Base address of Program Flash block */
    .PFlashSize  = FEATURE_FLS_PF_BLOCK_SIZE,          /* Size of Program Flash block         */
    .DFlashBase  = FEATURE_FLS_DF_START_ADDRESS,       /* Base address of Data Flash block    */
    .EERAMBase   = FEATURE_FLS_FLEX_RAM_START_ADDRESS, /* Base address of FlexRAM block       */
    /* If using callback, any code reachable from this function must not be placed in a Flash block targeted for a program/erase operation. */
    .CallBack    = NULL_CALLBACK
};

/* Declare a FLASH configuration struct which initialized by InitFlash, and will be used by all flash operations */
static flash_ssd_config_t flashSSDConfig;

#ifdef USE_FLASH_DRIVER
//#pragma CONST_SEG FLASH_HEADER
const tFlashOptInfo g_stFlashOptInfo =
{
    FALSH_DRIVER_START,
    FALSH_DRIVER_END,
    NULL_PTR,//&InitFlash,
    &EraseFlashSector,
    &WriteFlash,
    NULL_PTR//&ReadFlashMemory
};
//#pragma CODE_SEG DEFAULT

const tFlashOptInfo *g_pstFlashOptInfo = &g_stFlashOptInfo;
#else
//#pragma CODE_SEG DEFAULT
/* Declare a Flash configuration struct which initialized by InitFlashAPI, and will be used all flash operation */
static tFlashOptInfo *g_pstFlashOptInfo = (void *)0;
#endif /* USE_FLASH_DRIVER */

#ifndef FLASH_SDK_USING
/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_DRV_GetDEPartitionCode
 * Description   : Gets DFlash size from FlexNVM Partition Code.
 *
 *END**************************************************************************/
static void FLASH_DRV_GetDEPartitionCode(flash_ssd_config_t *const pSSDConfig,
                                         uint8_t DEPartitionCode)
{
    /* Select D-Flash size */
    switch (DEPartitionCode)
    {
        case 0x00U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0000;
            break;

        case 0x01U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0001;
            break;

        case 0x02U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0010;
            break;

        case 0x03U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0011;
            break;

        case 0x04U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0100;
            break;

        case 0x05U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0101;
            break;

        case 0x06U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0110;
            break;

        case 0x07U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_0111;
            break;

        case 0x08U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1000;
            break;

        case 0x09U:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1001;
            break;

        case 0x0AU:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1010;
            break;

        case 0x0BU:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1011;
            break;

        case 0x0CU:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1100;
            break;

        case 0x0DU:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1101;
            break;

        case 0x0EU:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1110;
            break;

        case 0x0FU:
            pSSDConfig->DFlashSize = (uint32_t)FEATURE_FLS_DF_SIZE_1111;
            break;

        default:
            /* Undefined value */
            break;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_DRV_Init
 * Description   : Initializes Flash module by clearing status error bit
 * and reporting the memory configuration via SSD configuration structure.
 *
 * Implements    : FLASH_DRV_Init_Activity
 *END**************************************************************************/
static status_t FLASH_DRV_Init(const flash_user_config_t *const pUserConf,
                               flash_ssd_config_t *const pSSDConfig)
{
    DEV_ASSERT(pUserConf != NULL);
    DEV_ASSERT(pSSDConfig != NULL);
    status_t ret = STATUS_SUCCESS;
#if FEATURE_FLS_HAS_FLEX_NVM
    uint8_t DEPartitionCode;    /* store D/E-Flash Partition Code */
#endif
    pSSDConfig->PFlashBase = pUserConf->PFlashBase;
    pSSDConfig->PFlashSize = pUserConf->PFlashSize;
    pSSDConfig->DFlashBase = pUserConf->DFlashBase;
    pSSDConfig->EERAMBase = pUserConf->EERAMBase;
    pSSDConfig->CallBack = pUserConf->CallBack;
#if FEATURE_FLS_HAS_FLEX_NVM
    /* Temporary solution for FTFC and S32K144 CSEc part */
    /* Get DEPART from Flash Configuration Register 1 */
    DEPartitionCode = (uint8_t)((SIM->FCFG1 & SIM_FCFG1_DEPART_MASK) >> SIM_FCFG1_DEPART_SHIFT);
    /* Get data flash size */
    FLASH_DRV_GetDEPartitionCode(pSSDConfig, DEPartitionCode);

    if (pSSDConfig->DFlashSize < FEATURE_FLS_DF_BLOCK_SIZE)
    {
        pSSDConfig->EEESize = FEATURE_FLS_FLEX_RAM_SIZE;
    }
    else
    {
        pSSDConfig->EEESize = 0U;
    }

#else /* FEATURE_FLS_HAS_FLEX_NVM == 0 */
    /* If size of D/E-Flash = 0 */
    pSSDConfig->DFlashSize = 0U;
    pSSDConfig->EEESize = 0U;
#endif /* End of FEATURE_FLS_HAS_FLEX_NVM */
    return ret;
}
#endif /* FLASH_SDK_USING */

void InitFlash(void)
{
    FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);
}

/* Init Flash API g_pstFlashOptInfo pointer */
void InitFlashAPI(void)
{
    uint32_t *tmp = NULL;
    uint32_t flashDriverStartAdd = 0;
    uint32_t flashDriverEndAdd = 0;
    FLASH_HAL_GetFlashDriverInfo(&flashDriverStartAdd, &flashDriverEndAdd);
    tmp = (uint32 *)flashDriverStartAdd;

    for (uint32_t i = 0; i < sizeof(tFlashOptInfo) / 4; i++)
    {
        /* XXX Bootloader: #01 此处 tmp[i] 用来给 Flash Driver 头部的函数偏移量偏移到目前 Bootloader 中实际的 RAM 地址
         * 官方 Bootloader 例程配合官方编译好的 12 个 Flash Driver 函数，为何要减去（向左偏移） 0x410 原因不明，
         * 自己编译出的 4 个 Flash Driver 函数不需要减 0x410
         */
        //        tmp[i] -= 0x410;
        tmp[i] += (uint32_t) flashDriverStartAdd;
    }

    g_pstFlashOptInfo = (tFlashOptInfo *)flashDriverStartAdd;
    /* Disable cache to ensure that all flash operations will take effect instantly,
       this is device dependent */
#if defined (S32K144_SERIES) || defined (S32K118_SERIES)
    //    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM1(0x3u);
    //    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM1(0x3u);
#endif /* defined (S32K144_SERIES) || defined (S32K118_SERIES) */
}

//#ifdef USE_FLASH_DRIVER
unsigned char EraseFlashSector(const unsigned long i_ulLogicalAddr,
                               const unsigned long i_ulEraseLen)
{
    status_t ret; /* Store the driver APIs return code */
    unsigned long i_ulStartVerifyAddr = i_ulLogicalAddr;
    unsigned long i_ulVerifyLen = i_ulEraseLen;

    ret = g_pstFlashOptInfo->FLASH_EraseSector(&flashSSDConfig, i_ulLogicalAddr, i_ulEraseLen);
    DEV_ASSERT(STATUS_SUCCESS == ret);

    ret = g_pstFlashOptInfo->FLASH_VerifySection(&flashSSDConfig, i_ulStartVerifyAddr, i_ulVerifyLen >> 4, 1u);
    DEV_ASSERT(STATUS_SUCCESS == ret);

    return ret;
}

unsigned char WriteFlash(const uint32_t i_xStartAddr,
                         const void *i_pvDataBuf,
                         const unsigned short i_usDataLen)
{
    uint32_t failAddr;
    status_t ret; /* Store the driver APIs return code */
    ret = g_pstFlashOptInfo->FLASH_Program(&flashSSDConfig, i_xStartAddr, i_usDataLen, i_pvDataBuf);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Verify the program operation at margin level value of 1, user margin */
    ret = g_pstFlashOptInfo->FLASH_ProgramCheck(&flashSSDConfig, i_xStartAddr, i_usDataLen, i_pvDataBuf, &failAddr, 1u);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    return ret;
}
//#endif /* USE_FLASH_DRIVER */

/* read a byte from flash. Read data address must be global address. */
unsigned char ReadFlashByte(const unsigned long i_ulGloabalAddress)
{
    unsigned char  ucReadvalue;
    /* From global address get values */
    ucReadvalue = (*((unsigned long *)i_ulGloabalAddress));
    return ucReadvalue;
}

/********************************************************
**  read data from current page flash.
**  Parameter :
**      @   i_ulLogicalAddr : Local address
**      @   i_ulLength : Data length need to be read
**      @   o_pucDataBuf : Data buffer used to store the read buffer
*********************************************************/
void ReadFlashMemory(const unsigned long i_ulLogicalAddr,
                     const unsigned long i_ulLength,
                     unsigned char *o_pucDataBuf)
{
    unsigned long ulGlobalAddr;
    unsigned long ulIndex = 0u;
    ulGlobalAddr = i_ulLogicalAddr;

    for (ulIndex = 0u; ulIndex < i_ulLength; ulIndex++)
    {
        o_pucDataBuf[ulIndex] = ReadFlashByte(ulGlobalAddr);
        ulGlobalAddr++;
    }
}

#ifdef FLASH_API_DEBUG
/* Data source for program operation */
#define BUFFER_SIZE         0x100u /* Size of data source */
uint8_t sourceBuffer[BUFFER_SIZE];

void Flash_Test(void)
{
    status_t ret; /* Store the driver APIs return code */
    uint32_t address;
    uint32_t size;
    uint32_t failAddr;

    /* Init source data */
    for (uint32_t i = 0u; i < BUFFER_SIZE; i++)
    {
        sourceBuffer[i] = i;
    }

    /* Erase the sixth PFlash sector */
    address = 20u * FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
    size = FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
    DISABLE_INTERRUPTS();
#ifndef FLASH_SDK_USING
    ret = g_pstFlashOptInfo->FLASH_EraseSector(&flashSSDConfig, address, size);
#else
    ret = FLASH_DRV_EraseSector(&flashSSDConfig, address, size);
#endif /* FLASH_SDK_USING */
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Disable Callback */
    flashSSDConfig.CallBack = NULL_CALLBACK;
    /* Verify the erase operation at margin level value of 1, user read */
    ret = g_pstFlashOptInfo->FLASH_VerifySection(&flashSSDConfig, address, size / FTFx_DPHRASE_SIZE, 1u);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Write some data to the erased PFlash sector */
    size = BUFFER_SIZE;
    ret = g_pstFlashOptInfo->FLASH_Program(&flashSSDConfig, address, size, sourceBuffer);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Verify the program operation at margin level value of 1, user margin */
    ret = g_pstFlashOptInfo->FLASH_ProgramCheck(&flashSSDConfig, address, size, sourceBuffer, &failAddr, 1u);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    ENABLE_INTERRUPTS();
#if (FEATURE_FLS_HAS_FLEX_NVM == 1u)
    /* Erase a sector in DFlash */
    address = flashSSDConfig.DFlashBase;
    size = FEATURE_FLS_DF_BLOCK_SECTOR_SIZE;
    ret = g_pstFlashOptInfo->FLASH_EraseSector(&flashSSDConfig, address, size);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Verify the erase operation at margin level value of 1, user read */
    ret = g_pstFlashOptInfo->FLASH_VerifySection(&flashSSDConfig, address, size / FTFx_PHRASE_SIZE, 1u);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Write some data to the erased DFlash sector */
    address = flashSSDConfig.DFlashBase;
    size = BUFFER_SIZE;
    ret = g_pstFlashOptInfo->FLASH_Program(&flashSSDConfig, address, size, sourceBuffer);
    DEV_ASSERT(STATUS_SUCCESS == ret);
    /* Verify the program operation at margin level value of 1, user margin */
    ret = g_pstFlashOptInfo->FLASH_ProgramCheck(&flashSSDConfig, address, size, sourceBuffer, &failAddr, 1u);
    DEV_ASSERT(STATUS_SUCCESS == ret);
#endif /* FEATURE_FLS_HAS_FLEX_NVM */
}
#endif /* FLASH_API_DEBUG */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
