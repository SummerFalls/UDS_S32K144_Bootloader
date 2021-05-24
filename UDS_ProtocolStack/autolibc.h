/*
 * @ 名称: autolibc.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef AUTOLIBC_H_
#define AUTOLIBC_H_

/*==============================================================================
                             DEFINES AND MACROS
==============================================================================*/
/* labs() is identical to abs(), because int and long are same on 32bit PPC */
#define fsl_labs(a) fsl_abs(a)

/*==============================================================================
                             FUNCTION PROTOTYPES
==============================================================================*/
void *fsl_memcpy(void *pavDest2, const void *pcoavSource2, uint32_t u32Length2);
void *fsl_memset(void *pavDest3, uint8_t u8Fill3, uint32_t u32Length3);
void *fsl_memmove(void *pavDest4, const void *pcoavSource4, uint32_t u32Length4);
int32_t fsl_memcmp(const void *pcoavMemA5, const void *pcoavMemB5, uint32_t u32Size5);
int32_t fsl_strcmp(const char_t *pcoszStrA6, const char_t *pcoszStrB6);
char_t *fsl_strncpy(char_t *pszDest7, const char_t *pcoszSrc7, uint32_t u32Length7);
uint32_t fsl_strlen(const char_t pcozsStr8[]);
void fsl_srand(uint32_t u32Seed9);
uint32_t fsl_rand(void);
int32_t fsl_abs(int32_t s32Number);
int64_t fsl_abs64(int64_t s64Number);

#endif /* AUTOLIBC_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
