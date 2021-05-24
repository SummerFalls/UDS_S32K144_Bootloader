/*
 * @ 名称: flash_hal.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "flash_hal.h"
#include "flash.h"

static boolean FLASH_HAL_Init(void);

static boolean FLASH_HAL_EraseSector(const uint32 i_startAddr, const uint32 i_noEraseSectors);

static boolean FLASH_HAL_WriteData(const uint32 i_startAddr,
                                   const uint8 *i_pDataBuf,
                                   const uint32 i_dataLen);

static boolean FLASH_HAL_ReadData(const uint32 i_startAddr,
                                  const uint32 i_readLen,
                                  uint8 *o_pDataBuf);

static void FLASH_HAL_Deinit(void);


/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_Init
 * Description   : This function initial this module.
 *
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean FLASH_HAL_Init(void)
{
    InitFlashAPI();
    return TRUE;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_EraseSector
 * Description   : This function is erase flash sectors.
 * Parameters    : i_startAddr input for start flash address
                   i_noEraseSectors input number of erase sectors
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean FLASH_HAL_EraseSector(const uint32 i_startAddr, const uint32 i_noEraseSectors)
{
    boolean retstates = FALSE; /* Store the driver APIs return code */
    uint8 ret = 0u;
    uint32 length = 0u;
    length = i_noEraseSectors * FLASH_HAL_Get1SectorBytes();
    ret = EraseFlashSector(i_startAddr, length);

    if (0u == ret)
    {
        retstates = TRUE;
    }

    return retstates;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_WriteData
 * Description   : This function is write data in flash. if write write data successfully return TRUE, else return FALSE.
 * Parameters    : i_startAddr input for start flash address
                   i_pDataBuf write data buffer
                   i_dataLen write data len
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean FLASH_HAL_WriteData(const uint32 i_startAddr,
                                   const uint8 *i_pDataBuf,
                                   const uint32 i_dataLen)
{
    boolean retstates = FALSE;
    uint8 lessWriteLen = 8u;
    uint8 aDataBuf[8u] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
    uint32 writeDataLen = 0u;
    uint8 index = 0u;
    DisableAllInterrupts();

    if (i_dataLen  & (lessWriteLen - 1))
    {
        /*if write data more than 8 bytes*/
        if (i_dataLen > lessWriteLen)
        {
            writeDataLen = i_dataLen - (i_dataLen  & (lessWriteLen - 1));

            if (0u == WriteFlash(i_startAddr, i_pDataBuf, writeDataLen))
            {
                retstates = TRUE;
            }
            else
            {
                retstates = FALSE;
            }

            if ((TRUE == retstates))
            {
                for (index = 0u; index < (i_dataLen  & (lessWriteLen - 1)); index++)
                {
                    aDataBuf[index] = i_pDataBuf[writeDataLen + index];
                }

                if (0u == WriteFlash(i_startAddr + writeDataLen, aDataBuf, 8u))
                {
                    retstates = TRUE;
                }
            }
        }
        else
        {
            for (index = 0u; index < i_dataLen; index++)
            {
                aDataBuf[index] = i_pDataBuf[writeDataLen + index];
            }

            if (0u == WriteFlash(i_startAddr + writeDataLen, aDataBuf, 8u))
            {
                retstates = TRUE;
            }
        }
    }
    else
    {
        if (0u == WriteFlash(i_startAddr, i_pDataBuf, i_dataLen))
        {
            retstates = TRUE;
        }
    }

    EnableAllInterrupts();
    return retstates;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_ReadData
 * Description   : This function is read data in RAM. if read data successfully return TRUE, else return FALSE.
 * Parameters    : i_startAddr input for start flash address
                   i_readLen read data length
                   o_pDataBuf read data buffer
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean FLASH_HAL_ReadData(const uint32 i_startAddr,
                                  const uint32 i_readLen,
                                  uint8 *o_pDataBuf)
{
    FLSDebugPrintf("\n %s\n", __func__);
    //ReadFlashMemory(i_startAddr, i_readLen, o_pDataBuf);
    return TRUE;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_Deinit
 * Description   : This function init this module.
 *
 * Implements : FLASH_HAL_Deinit_Activity
 *END**************************************************************************/
static void FLASH_HAL_Deinit(void)
{
    FLSDebugPrintf("\n %s\n", __func__);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_RegisterFlashAPI
 * Description   : This function is register flash API. The API maybe download from host and storage in RAM.
 *
 *END**************************************************************************/
boolean FLASH_HAL_RegisterFlashAPI(tFlashOperateAPI *o_pstFlashOperateAPI)
{
    boolean result = FALSE;

    if (NULL_PTR != o_pstFlashOperateAPI)
    {
        o_pstFlashOperateAPI->pfFlashInit = FLASH_HAL_Init;
        o_pstFlashOperateAPI->pfEraserSecotr = FLASH_HAL_EraseSector;
        o_pstFlashOperateAPI->pfProgramData = FLASH_HAL_WriteData;
        o_pstFlashOperateAPI->pfReadFlashData = FLASH_HAL_ReadData;
        o_pstFlashOperateAPI->pfFlashDeinit = FLASH_HAL_Deinit;
        result = TRUE;
    }

    return result;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
