/*
 * @ 名称: CRC_hal.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef CRC_HAL_H_
#define CRC_HAL_H_

#include "includes.h"

#define CRC_SEED_INIT_VALUE 0xFFFF
typedef uint32 tCrc;

boolean CRC_HAL_Init(void);
/*!
 * @brief To create CRC.
 *
 * This function returns the state of the initial.
 *
 * @param[in] instance instance number
 * @param[in] data buffer
 * @param[in] data len
 * @param[out] CRC value
 * @return the initial state.
 */
void CRC_HAL_CreatHardwareCrc(const uint8 *i_pucDataBuf, const uint32 i_ulDataLen, uint32 *m_pCurCrc);

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_HAL_CreatHardwareCrc
 * Description   : This function use software lookup table or calculate  to create CRC..
 *
 * Implements : CreatCrc_Activity
 *END**************************************************************************/
void CRC_HAL_CreatSoftwareCrc(const uint8_t *i_pucDataBuf, const uint32_t i_ulDataLen, uint32_t *m_pCurCrc);

void CRC_HAL_Deinit(void);

#endif /* CRC_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
