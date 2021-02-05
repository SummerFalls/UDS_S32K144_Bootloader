/*
 * @ 名称: CRC_hal.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "CRC_hal.h"

#ifdef EN_CRC_HARDWARE
#include "cpu.h"
#include "crc_cfg.h"
#endif

/*******************************************************************************
 * User Include
 ******************************************************************************/

#ifdef EN_CRC_SOFTWARE
/*crc table*/
#if 0
/*
  *CRC-16/CCITT crc table
  *Poly:   0x1021
  *Init:   0xFFFF
  *RefIn:  false
  *RefOut: false
  *XorOut: 0x0000
*/
static const uint16 gs_aCrc16Tab[] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
#endif

/*
  *CRC-16/DNP crc table
  *Poly: 0x3D65
  *Init:  0x0000
  *RefIn:true
  *RefOut:true
  *XorOut:0xFFFF
*/
static const uint16 g_dnpCrcTable[256u] = {
    0x0000, 0x365e, 0x6cbc, 0x5ae2, 0xd978, 0xef26, 0xb5c4, 0x839a,
    0xff89, 0xc9d7, 0x9335, 0xa56b, 0x26f1, 0x10af, 0x4a4d, 0x7c13,
    0xb26b, 0x8435, 0xded7, 0xe889, 0x6b13, 0x5d4d, 0x07af, 0x31f1,
    0x4de2, 0x7bbc, 0x215e, 0x1700, 0x949a, 0xa2c4, 0xf826, 0xce78,
    0x29af, 0x1ff1, 0x4513, 0x734d, 0xf0d7, 0xc689, 0x9c6b, 0xaa35,
    0xd626, 0xe078, 0xba9a, 0x8cc4, 0x0f5e, 0x3900, 0x63e2, 0x55bc,
    0x9bc4, 0xad9a, 0xf778, 0xc126, 0x42bc, 0x74e2, 0x2e00, 0x185e,
    0x644d, 0x5213, 0x08f1, 0x3eaf, 0xbd35, 0x8b6b, 0xd189, 0xe7d7,
    0x535e, 0x6500, 0x3fe2, 0x09bc, 0x8a26, 0xbc78, 0xe69a, 0xd0c4,
    0xacd7, 0x9a89, 0xc06b, 0xf635, 0x75af, 0x43f1, 0x1913, 0x2f4d,
    0xe135, 0xd76b, 0x8d89, 0xbbd7, 0x384d, 0x0e13, 0x54f1, 0x62af,
    0x1ebc, 0x28e2, 0x7200, 0x445e, 0xc7c4, 0xf19a, 0xab78, 0x9d26,
    0x7af1, 0x4caf, 0x164d, 0x2013, 0xa389, 0x95d7, 0xcf35, 0xf96b,
    0x8578, 0xb326, 0xe9c4, 0xdf9a, 0x5c00, 0x6a5e, 0x30bc, 0x06e2,
    0xc89a, 0xfec4, 0xa426, 0x9278, 0x11e2, 0x27bc, 0x7d5e, 0x4b00,
    0x3713, 0x014d, 0x5baf, 0x6df1, 0xee6b, 0xd835, 0x82d7, 0xb489,
    0xa6bc, 0x90e2, 0xca00, 0xfc5e, 0x7fc4, 0x499a, 0x1378, 0x2526,
    0x5935, 0x6f6b, 0x3589, 0x03d7, 0x804d, 0xb613, 0xecf1, 0xdaaf,
    0x14d7, 0x2289, 0x786b, 0x4e35, 0xcdaf, 0xfbf1, 0xa113, 0x974d,
    0xeb5e, 0xdd00, 0x87e2, 0xb1bc, 0x3226, 0x0478, 0x5e9a, 0x68c4,
    0x8f13, 0xb94d, 0xe3af, 0xd5f1, 0x566b, 0x6035, 0x3ad7, 0x0c89,
    0x709a, 0x46c4, 0x1c26, 0x2a78, 0xa9e2, 0x9fbc, 0xc55e, 0xf300,
    0x3d78, 0x0b26, 0x51c4, 0x679a, 0xe400, 0xd25e, 0x88bc, 0xbee2,
    0xc2f1, 0xf4af, 0xae4d, 0x9813, 0x1b89, 0x2dd7, 0x7735, 0x416b,
    0xf5e2, 0xc3bc, 0x995e, 0xaf00, 0x2c9a, 0x1ac4, 0x4026, 0x7678,
    0x0a6b, 0x3c35, 0x66d7, 0x5089, 0xd313, 0xe54d, 0xbfaf, 0x89f1,
    0x4789, 0x71d7, 0x2b35, 0x1d6b, 0x9ef1, 0xa8af, 0xf24d, 0xc413,
    0xb800, 0x8e5e, 0xd4bc, 0xe2e2, 0x6178, 0x5726, 0x0dc4, 0x3b9a,
    0xdc4d, 0xea13, 0xb0f1, 0x86af, 0x0535, 0x336b, 0x6989, 0x5fd7,
    0x23c4, 0x159a, 0x4f78, 0x7926, 0xfabc, 0xcce2, 0x9600, 0xa05e,
    0x6e26, 0x5878, 0x029a, 0x34c4, 0xb75e, 0x8100, 0xdbe2, 0xedbc,
    0x91af, 0xa7f1, 0xfd13, 0xcb4d, 0x48d7, 0x7e89, 0x246b, 0x1235,
};

#if 0
static unsigned short calcDnpCrc(unsigned char *p, unsigned int count )
{
unsigned int crc = 0 ;

while ( count-- ) crc = ( crc >> 8 ) ^ dnpCrcTable[( crc ^ *p++) & 0x00ff] ;

return( (unsigned short) ( ~crc ) ) ;
}
#endif
/************************************************************
**  Description :   using software lookup table to create crc. Input data in @ i_pucDataBuf
**  and data len in @ i_ulDataLen. When create crc successful
**  return CRC.
************************************************************/
static void CreatSoftwareCrc16(const uint8 *i_pDataBuf, const uint32 i_dataLen, uint32 *m_pCurCrc);
#endif

#ifdef EN_CRC_HARDWARE
/************************************************************
**  Description :   init hardware crc
**  data            NULL
**  return          Null
************************************************************/
static void Crc_Init(void);

/************************************************************
**  Description :   using MCU hardware to create crc. Input data in @ i_pucDataBuf
**  and data len in @ i_ulDataLen. When create crc successful
**  return CRC.
************************************************************/
static void CreatHardwareCrc16(const uint8_t *i_pucDataBuf, const uint32_t i_ulDataLen, uint32_t *m_pCurCrc);
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_Init
 * Description   : This function initial this module.
 *
 * Implements : CRC_Init_Activity
 *END**************************************************************************/
boolean CRC_HAL_Init(void)
{
#ifdef EN_CRC_HARDWARE
    Crc_Init();
#endif


    return TRUE;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_HAL_CreatHardwareCrc
 * Description   : This function use MCU hardware to create crc.
 *
 * Implements : CreatCrc_Activity
 *END**************************************************************************/
void CRC_HAL_CreatHardwareCrc(const uint8 *i_pucDataBuf, const uint32 i_ulDataLen, uint32 *m_pCurCrc)
{
#ifdef EN_CRC_HARDWARE
    CreatHardwareCrc16(i_pucDataBuf, i_ulDataLen, m_pCurCrc);
#elif (defined EN_CRC_SOFTWARE)
    CreatSoftwareCrc16(i_pucDataBuf, i_ulDataLen, m_pCurCrc);
#else
    #error "Non CRC module enabled!"
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_HAL_CreatHardwareCrc
 * Description   : This function use software lookup table or calculate  to create crc..
 *
 * Implements : CreatCrc_Activity
 *END**************************************************************************/
void CRC_HAL_CreatSoftwareCrc(const uint8_t *i_pucDataBuf, const uint32_t i_ulDataLen, uint32_t *m_pCurCrc)
{
#ifdef EN_CRC_SOFTWARE
    CreatSoftwareCrc16(i_pucDataBuf, i_ulDataLen, m_pCurCrc);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_Deinit
 * Description   : This function initial this module.
 *
 * Implements : CRC_Deinit_Activity
 *END**************************************************************************/
void CRC_HAL_Deinit(void)
{


}

#ifdef EN_CRC_SOFTWARE
/************************************************************
**  Description :   using software lookup table to create crc. Input data in @ i_pucDataBuf
**  and data len in @ i_ulDataLen. When create crc successful
**  return CRC.
************************************************************/
static void CreatSoftwareCrc16(const uint8 *i_pDataBuf, const uint32 i_dataLen, uint32 *m_pCurCrc)
{
    uint16 crc = 0u;
    uint32 index = 0u;

#if (defined FALSH_ADDRESS_CONTINUE) && (FALSH_ADDRESS_CONTINUE == TRUE)
    crc = *m_pCurCrc;
#endif
    for(index = 0u; index < i_dataLen; index++)
    {
        crc = ( crc >> 8 ) ^ g_dnpCrcTable[( crc ^ i_pDataBuf[index]) & 0x00ff] ;
    }

    *m_pCurCrc = (uint32)((~crc) & 0xFFFFu);
}
#endif

#ifdef EN_CRC_HARDWARE
/************************************************************
**  Description :   using MCU hardware to create crc. Input data in @ i_pucDataBuf
**  and data len in @ i_ulDataLen. When create crc successful
**  return CRC.
************************************************************/
static void CreatHardwareCrc16(const uint8_t *i_pucDataBuf, const uint32_t i_ulDataLen, uint32_t *m_pCurCrc)
{
    /* Write your local variable definition here */
    uint32_t result = 0u;

#if (defined FALSH_ADDRESS_CONTINUE) && (FALSH_ADDRESS_CONTINUE == TRUE)
    crc1_UserConfig0.seed = *m_pCurCrc;
#endif  //end of MCU_USE_PAGING

    /* Init crc hardware */
    CRC_DRV_Init(INST_CRC1, &crc1_UserConfig0);


    /* Calculate CRC value for CRC_data with configuration of 16 of 32bit wide result */
    CRC_DRV_WriteData(INST_CRC1, i_pucDataBuf, i_ulDataLen);
    result = CRC_DRV_GetCrcResult(INST_CRC1);

    *m_pCurCrc = result;
}

/************************************************************
**  Description :   init hardware crc
**  data            NULL
**  return          Null
************************************************************/
static void Crc_Init(void)
{
#ifdef USING_HARDWARE_CRC
    /* call crc init before system clock init */
    PCC->PCCn[PCC_CRC_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Init crc hardware */
    CRC_DRV_Init(INST_CRC1, &crc1_InitConfig0);

    /* Config user crcWidth, seed, polynomial, writeTranspose, readTranspose, complementChecksum */
    //CRC_DRV_Configure(INST_CRC1, &crc1_UserConfig0);

#endif //end of USING_HARDWARE_CRC
}
#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
