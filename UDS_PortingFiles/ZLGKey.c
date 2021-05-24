/*
 * @ 名称: ZLGKey.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "ZLGKey.h"
#ifdef EN_ZLG_SA_ALGORITHM

/*******************************************************************************
* @name   : deZLGKey
* @brief  : 简单的用于ZCANPRO安全访问验证解密算法，对应于ZCANPRO的安全算法链接库
* @param  : void
* @retval : void
*******************************************************************************/
void deZLGKey(sint8 *c, sint32 clen, sint8 *pPlainText)
{
    sint32 i;

    if (clen == 0 || clen % 16 != 0)
    {
        return;
    }

    for (i = 0; i < clen; i++)
    {
        pPlainText[i] = c[i] - 1;
    }
}
#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
