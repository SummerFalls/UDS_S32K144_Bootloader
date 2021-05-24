/*
 * @ 名称: UDS_alg_hal.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef UDS_ALG_HAL_H_
#define UDS_ALG_HAL_H_

#include "includes.h"

void UDS_ALG_HAL_Init(void);

/*!
 * @brief To UDS encrypt data.
 *
 * This function returns encrypt data status.
 *
 * @param[in] i_pPlainText point plaintext
 * @param[in] i_dataLen point plaintext data lenght
 * @param[out]  o_pCipherText point ciphertext
 * @return encrypt data status.
 */
boolean UDS_ALG_HAL_EncryptData(const uint8 *i_pPlainText, const uint32 i_dataLen, uint8 *o_pCipherText);

/*!
 * @brief To UDS decrypt data.
 *
 * This function returns decrypt data status.
 *
 * @param[in] i_pCipherText point ciphertext
 * @param[in] i_dataLen point ciphertext data lenght
 * @param[out]  o_pPlainText point plaintext
 * @return decrypt data status.
 */
boolean UDS_ALG_HAL_DecryptData(const uint8 *i_pCipherText, const uint32 i_dataLen, uint8 *o_pPlainText);

/*!
 * @brief To UDS get random data.
 *
 * This function returns get random data status.
 *
 * @param[in] i_needRandomLen need random data len
 * @param[out]  o_pRandomBuf point random data buff
 * @return get random data status.
 */
boolean UDS_ALG_HAL_GetRandom(const uint32 i_needRandomDataLen, uint8 *o_pRandomDataBuf);

/* UDS software timer tick */
void UDS_ALG_HAL_AddSWTimerTickCnt(void);

void UDS_ALG_HAL_Deinit(void);

#endif /* UDS_ALG_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
