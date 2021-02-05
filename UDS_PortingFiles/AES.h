/*
 * @ ����: AES.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef AES_H_
#define AES_H_

#include "includes.h"

#ifdef EN_ALG_SW

/**
**  p:    plaintxt
**  plen: plaintxt length
**  key:  AES key
**  ciper:cipertext
**/
void aes(sint8 *p, sint32 plen, sint8 *key, sint8 * cipher);

/**
** c         : ciphertext
** clen      : ciphertext length
** key       : AES key
** pPlainText: plaintxt
**/
void deAes(sint8 *c, sint32 clen, sint8 *key, sint8 * pPlainText);
#endif

#endif /* AES_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */