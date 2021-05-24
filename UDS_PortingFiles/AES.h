/*
 * @ 名称: AES.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef AES_H_
#define AES_H_

#include "includes.h"

#ifdef EN_AES_SA_ALGORITHM_SW

/**
**  p:      plain text
**  plen:   plain text length
**  key:    AES key
**  cipher: cipher text
**/
void aes(sint8 *p, sint32 plen, sint8 *key, sint8 *cipher);

/**
** c         : cipher text
** clen      : cipher text length
** key       : AES key
** pPlainText: plain text
**/
void deAes(sint8 *c, sint32 clen, sint8 *key, sint8 *pPlainText);

#endif

#endif /* AES_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
