/*
 * @ 名称: AES.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "AES.h"
#ifdef EN_AES_SA_ALGORITHM_SW
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const sint32 S[16][16] =
{
    { 0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76 },
    { 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0 },
    { 0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15 },
    { 0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75 },
    { 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84 },
    { 0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF },
    { 0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8 },
    { 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2 },
    { 0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73 },
    { 0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB },
    { 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79 },
    { 0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08 },
    { 0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A },
    { 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E },
    { 0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF },
    { 0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16 }
};

static const sint32 S2[16][16] =
{
    { 0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB },
    { 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB },
    { 0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E },
    { 0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25 },
    { 0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92 },
    { 0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84 },
    { 0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06 },
    { 0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B },
    { 0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73 },
    { 0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E },
    { 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B },
    { 0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4 },
    { 0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F },
    { 0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF },
    { 0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61 },
    { 0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D }
};

static sint32 getLeft4Bit(sint32 num)
{
    sint32 left = num & 0x000000F0;
    return left >> 4;
}

static sint32 getRight4Bit(sint32 num)
{
    return num & 0x0000000F;
}

static sint32 getNumFromSBox(sint32 index)
{
    sint32 row = getLeft4Bit(index);
    sint32 col = getRight4Bit(index);
    return S[row][col];
}

static sint32 getIntFromChar(sint8 c)
{
    sint32 result = (sint32) c;
    return result & 0x000000FF;
}

static void convertToIntArray(sint8 *str, sint32 pa[4][4])
{
    sint32 k = 0;
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            pa[j][i] = getIntFromChar(str[k]);
            k++;
        }
}

static sint32 getWordFromStr(sint8 *str)
{
    sint32 one, two, three, four;
    one = getIntFromChar(str[0]);
    one = one << 24;
    two = getIntFromChar(str[1]);
    two = two << 16;
    three = getIntFromChar(str[2]);
    three = three << 8;
    four = getIntFromChar(str[3]);
    return one | two | three | four;
}

static void splitIntToArray(sint32 num, sint32 array[4])
{
    sint32 one, two, three;
    one = num >> 24;
    array[0] = one & 0x000000FF;
    two = num >> 16;
    array[1] = two & 0x000000FF;
    three = num >> 8;
    array[2] = three & 0x000000FF;
    array[3] = num & 0x000000FF;
}

static void leftLoop4int(sint32 array[4], sint32 step)
{
    sint32 temp[4];
    sint32 i;
    sint32 index;

    for (i = 0; i < 4; i++)
    {
        temp[i] = array[i];
    }

    index = step % 4 == 0 ? 0 : step % 4;

    for (i = 0; i < 4; i++)
    {
        array[i] = temp[index];
        index++;
        index = index % 4;
    }
}

static sint32 mergeArrayToInt(sint32 array[4])
{
    sint32 one = array[0] << 24;
    sint32 two = array[1] << 16;
    sint32 three = array[2] << 8;
    sint32 four = array[3];
    return one | two | three | four;
}

static const sint32 Rcon[10] = { 0x01000000, 0x02000000,
                                 0x04000000, 0x08000000,
                                 0x10000000, 0x20000000,
                                 0x40000000, 0x80000000,
                                 0x1B000000, 0x36000000
                               };

static sint32 T(sint32 num, sint32 round)
{
    sint32 numArray[4];
    sint32 i;
    sint32 result;
    splitIntToArray(num, numArray);
    leftLoop4int(numArray, 1);

    for (i = 0; i < 4; i++)
    {
        numArray[i] = getNumFromSBox(numArray[i]);
    }

    result = mergeArrayToInt(numArray);
    return result ^ Rcon[round];
}

static sint32 w[44];

static void extendKey(sint8 *key)
{
    sint32 i;
    sint32 j;

    for (i = 0; i < 4; i++)
    {
        w[i] = getWordFromStr(key + i * 4);
    }

    for (i = 4, j = 0; i < 44; i++)
    {
        if (i % 4 == 0)
        {
            w[i] = w[i - 4] ^ T(w[i - 1], j);
            j++;
        }
        else
        {
            w[i] = w[i - 4] ^ w[i - 1];
        }
    }
}

static void addRoundKey(sint32 array[4][4], sint32 round)
{
    sint32 warray[4];
    sint32 i, j;

    for (i = 0; i < 4; i++)
    {
        splitIntToArray(w[round * 4 + i], warray);

        for (j = 0; j < 4; j++)
        {
            array[j][i] = array[j][i] ^ warray[j];
        }
    }
}

static void subBytes(sint32 array[4][4])
{
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            array[i][j] = getNumFromSBox(array[i][j]);
        }
}

static void shiftRows(sint32 array[4][4])
{
    sint32 rowTwo[4], rowThree[4], rowFour[4];
    sint32 i;

    for (i = 0; i < 4; i++)
    {
        rowTwo[i] = array[1][i];
        rowThree[i] = array[2][i];
        rowFour[i] = array[3][i];
    }

    leftLoop4int(rowTwo, 1);
    leftLoop4int(rowThree, 2);
    leftLoop4int(rowFour, 3);

    for (i = 0; i < 4; i++)
    {
        array[1][i] = rowTwo[i];
        array[2][i] = rowThree[i];
        array[3][i] = rowFour[i];
    }
}

static const sint32 colM[4][4] =
{
    { 2, 3, 1, 1 },
    { 1, 2, 3, 1 },
    { 1, 1, 2, 3 },
    { 3, 1, 1, 2 }
};

static sint32 GFMul2(sint32 s)
{
    sint32 result = s << 1;
    sint32 a7 = result & 0x00000100;

    if (a7 != 0)
    {
        result = result & 0x000000FF;
        result = result ^ 0x1B;
    }

    return result;
}

static sint32 GFMul3(sint32 s)
{
    return GFMul2(s) ^ s;
}

static sint32 GFMul4(sint32 s)
{
    return GFMul2(GFMul2(s));
}

static sint32 GFMul8(sint32 s)
{
    return GFMul2(GFMul4(s));
}

static sint32 GFMul9(sint32 s)
{
    return GFMul8(s) ^ s;
}

static sint32 GFMul11(sint32 s)
{
    return GFMul9(s) ^ GFMul2(s);
}

static sint32 GFMul12(sint32 s)
{
    return GFMul8(s) ^ GFMul4(s);
}

static sint32 GFMul13(sint32 s)
{
    return GFMul12(s) ^ s;
}

static sint32 GFMul14(sint32 s)
{
    return GFMul12(s) ^ GFMul2(s);
}

static sint32 GFMul(sint32 n, sint32 s)
{
    sint32 result;

    switch (n)
    {
        case 1:
            result = s;
            break;

        case 2:
            result = GFMul2(s);
            break;

        case 3:
            result = GFMul3(s);
            break;

        case 0x9:
            result = GFMul9(s);
            break;

        case 0xB:
            result = GFMul11(s);
            break;

        case 0xD:
            result = GFMul13(s);
            break;

        case 0xE:
            result = GFMul14(s);
            break;

        default:
            break;
    }

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    return result;
}

static void mixColumns(sint32 array[4][4])
{
    sint32 tempArray[4][4];
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            tempArray[i][j] = array[i][j];
        }

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            array[i][j] = GFMul(colM[i][0], tempArray[0][j]) ^ GFMul(colM[i][1], tempArray[1][j])
                          ^ GFMul(colM[i][2], tempArray[2][j]) ^ GFMul(colM[i][3], tempArray[3][j]);
        }
}

static void convertArrayToStr(sint32 array[4][4], sint8 *str)
{
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            *str++ = (sint8) array[j][i];
        }
}

static sint32 checkKeyLen(sint32 len)
{
    if (len == 16)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void aes(sint8 *p, sint32 plen, sint8 *key, sint8 *cipher)
{
    sint32 pArray[4][4];
    sint32 i, k;
    sint32 keylen = 16;

    if (plen == 0 || plen % 16 != 0)
    {
        return;
    }

    if (!checkKeyLen(keylen))
    {
        return;
    }

    extendKey(key);

    for (k = 0; k < plen; k += 16)
    {
        convertToIntArray(p + k, pArray);
        addRoundKey(pArray, 0);

        for (i = 1; i < 10; i++)
        {
            subBytes(pArray);
            shiftRows(pArray);
            mixColumns(pArray);
            addRoundKey(pArray, i);
        }

        subBytes(pArray);
        shiftRows(pArray);
        addRoundKey(pArray, 10);
        convertArrayToStr(pArray, cipher + k);
    }
}

static sint32 getNumFromS1Box(sint32 index)
{
    sint32 row = getLeft4Bit(index);
    sint32 col = getRight4Bit(index);
    return S2[row][col];
}

static void deSubBytes(sint32 array[4][4])
{
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            array[i][j] = getNumFromS1Box(array[i][j]);
        }
}

static void rightLoop4int(sint32 array[4], sint32 step)
{
    sint32 temp[4];
    sint32 i;
    sint32 index;

    for (i = 0; i < 4; i++)
    {
        temp[i] = array[i];
    }

    index = step % 4 == 0 ? 0 : step % 4;
    index = 3 - index;

    for (i = 3; i >= 0; i--)
    {
        array[i] = temp[index];
        index--;
        index = index == -1 ? 3 : index;
    }
}

static void deShiftRows(sint32 array[4][4])
{
    sint32 rowTwo[4], rowThree[4], rowFour[4];
    sint32 i;

    for (i = 0; i < 4; i++)
    {
        rowTwo[i] = array[1][i];
        rowThree[i] = array[2][i];
        rowFour[i] = array[3][i];
    }

    rightLoop4int(rowTwo, 1);
    rightLoop4int(rowThree, 2);
    rightLoop4int(rowFour, 3);

    for (i = 0; i < 4; i++)
    {
        array[1][i] = rowTwo[i];
        array[2][i] = rowThree[i];
        array[3][i] = rowFour[i];
    }
}

static const sint32 deColM[4][4] =
{
    {0x0E, 0x0B, 0x0D, 0x09},
    {0x09, 0x0E, 0x0B, 0x0D},
    {0x0D, 0x09, 0x0E, 0x0B},
    {0x0B, 0x0D, 0x09, 0x0E}
};

static void deMixColumns(sint32 array[4][4])
{
    sint32 tempArray[4][4];
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            tempArray[i][j] = array[i][j];
        }

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            array[i][j] = GFMul(deColM[i][0], tempArray[0][j]) ^ GFMul(deColM[i][1], tempArray[1][j])
                          ^ GFMul(deColM[i][2], tempArray[2][j]) ^ GFMul(deColM[i][3], tempArray[3][j]);
        }
}

static void addRoundTowArray(sint32 aArray[4][4], sint32 bArray[4][4])
{
    sint32 i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        {
            aArray[i][j] = aArray[i][j] ^ bArray[i][j];
        }
}

static void getArrayFrom4W(sint32 i, sint32 array[4][4])
{
    sint32 index = i * 4;
    sint32 colOne[4], colTwo[4], colThree[4], colFour[4];
    sint32 j;
    splitIntToArray(w[index], colOne);
    splitIntToArray(w[index + 1], colTwo);
    splitIntToArray(w[index + 2], colThree);
    splitIntToArray(w[index + 3], colFour);

    for (j = 0; j < 4; j++)
    {
        array[j][0] = colOne[j];
        array[j][1] = colTwo[j];
        array[j][2] = colThree[j];
        array[j][3] = colFour[j];
    }
}

void deAes(sint8 *c, sint32 clen, sint8 *key, sint8 *pPlainText)
{
    sint32 keylen = 16;
    sint32 k, i;
    sint32 cArray[4][4];
    sint32 wArray[4][4];

    if (clen == 0 || clen % 16 != 0)
    {
        return;
    }

    if (!checkKeyLen(keylen))
    {
        return;
    }

    extendKey(key);

    for (k = 0; k < clen; k += 16)
    {
        convertToIntArray(c + k, cArray);
        addRoundKey(cArray, 10);

        for (i = 9; i >= 1; i--)
        {
            deSubBytes(cArray);
            deShiftRows(cArray);
            deMixColumns(cArray);
            getArrayFrom4W(i, wArray);
            deMixColumns(wArray);
            addRoundTowArray(cArray, wArray);
        }

        deSubBytes(cArray);
        deShiftRows(cArray);
        addRoundKey(cArray, 0);
        convertArrayToStr(cArray, pPlainText + k);
    }
}
#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
