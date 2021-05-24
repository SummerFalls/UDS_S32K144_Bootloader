/*
 * @ 名称: flash.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "flash_cfg.h"

/*******************************************************************************
* Callback function prototype
*******************************************************************************/
/*! @brief Call back function pointer data type
 *
 *   If using callback in the application, any code reachable from this function
 *   must not be placed in a Flash block targeted for a program/erase operation.
 *   Functions can be placed in RAM section by using the START/END_FUNCTION_DEFINITION/DECLARATION_RAMSECTION macros.
 */
typedef void (* flash_callback_t)(void);

/*! @brief Null callback */
#define NULL_CALLBACK      ((flash_callback_t)0xFFFFFFFFU)

#ifndef FLASH_SDK_USING

/* Word size 2 bytes */
#define FTFx_WORD_SIZE     0x0002U
/* Long word size 4 bytes */
#define FTFx_LONGWORD_SIZE 0x0004U
/* Phrase size 8 bytes */
#define FTFx_PHRASE_SIZE   0x0008U
/* Double-phrase size 16 bytes */
#define FTFx_DPHRASE_SIZE  0x0010U

/*!
 * @brief Flash User Configuration Structure
 *
 * Implements : flash_user_config_t_Class
 */
typedef struct
{
    uint32_t PFlashBase;            /*!< The base address of P-Flash memory */
    uint32_t PFlashSize;            /*!< The size in byte of P-Flash memory */
    uint32_t DFlashBase;            /*!< For FlexNVM device, this is the base address of D-Flash memory
                                     *    (FlexNVM memory); For non-FlexNVM device, this field is unused */
    uint32_t EERAMBase;             /*!< The base address of FlexRAM (for FlexNVM device)
                                     *    or acceleration RAM memory (for non-FlexNVM device) */
    flash_callback_t CallBack;      /*!< Call back function to service the time critical events. Any code reachable from this function
                                     *   must not be placed in a Flash block targeted for a program/erase operation */
} flash_user_config_t;

/*!
 * @brief Flash SSD Configuration Structure
 *
 * The structure includes the static parameters for C90TFS/FTFx which are
 * device-dependent. The fields including
 * PFlashBlockBase, PFlashBlockSize, DFlashBlockBase, EERAMBlockBase,
 * and CallBack are passed via flash_user_config_t.
 * The rest of parameters such as DFlashBlockSize, and EEEBlockSize will be
 * initialized in FLASH_DRV_Init() automatically.
 *
 * Implements : flash_ssd_config_t_Class
 */
typedef struct
{
    uint32_t PFlashBase;          /*!< The base address of P-Flash memory */
    uint32_t PFlashSize;          /*!< The size in byte of P-Flash memory */
    uint32_t DFlashBase;          /*!< For FlexNVM device, this is the base address of D-Flash memory (FlexNVM memory);
                                   *    For non-FlexNVM device, this field is unused */
    uint32_t DFlashSize;          /*!< For FlexNVM device, this is the size in byte of area
                                   *    which is used as D-Flash from FlexNVM memory;
                                   *    For non-FlexNVM device, this field is unused */
    uint32_t EERAMBase;           /*!< The base address of FlexRAM (for FlexNVM device)
                                   *    or acceleration RAM memory (for non-FlexNVM device) */
    uint32_t EEESize;             /*!< For FlexNVM device, this is the size in byte of EEPROM area which was partitioned
                                   *    from FlexRAM; For non-FlexNVM device, this field is unused */
    flash_callback_t CallBack;    /*!< Call back function to service the time critical events. Any code reachable from this function
                                   *   must not be placed in a Flash block targeted for a program/erase operation */
} flash_ssd_config_t;
#endif /* FLASH_SDK_USING */

/* 本结构体内各函数指针成员顺序必须与 Flash Driver 工程一一对应 */
typedef struct
{
    status_t (*FLASH_EraseSector)   (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size);
    status_t (*FLASH_VerifySection) (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint16_t number, uint8_t marginLevel);
    status_t (*FLASH_Program)       (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pData);
    status_t (*FLASH_ProgramCheck)  (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pExpectedData, uint32_t *pFailAddr, uint8_t marginLevel);
} tFlashOptInfo;

#ifdef FLASH_API_DEBUG
void Flash_Test(void);
#endif /* FLASH_API_DEBUG */


//#ifdef USE_FLASH_DRIVER
unsigned char EraseFlashSector(const unsigned long i_ulLogicalAddr,
                               const unsigned long i_ulEraseLen);

unsigned char WriteFlash(const uint32_t i_xStartAddr,
                         const void *i_pvDataBuf,
                         const unsigned short i_usDataLen);
//#endif /* USE_FLASH_DRIVER */

void InitFlash(void);

void InitFlashAPI(void);

unsigned char ReadFlashByte(const unsigned long i_ulGloabalAddress);

void ReadFlashMemory(const unsigned long i_ulLogicalAddr,
                     const unsigned long i_ulLength,
                     unsigned char *o_pucDataBuf);

#endif /* FLASH_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
