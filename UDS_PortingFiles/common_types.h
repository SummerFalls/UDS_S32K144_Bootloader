/*
 * @ 名称: common_types.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

/*==================================================================================================
                                         MISRA VIOLATIONS
==================================================================================================*/

/**
* @page misra_violations MISRA-C:2004 violations
*
* @section common_types_h_REF_1
* Violates MISRA 2004 Advisory Rule 19.7, A function should be used in preference to a function-like macro.
* Function-like macros does not support type control of parameters but inline functions usage causes
* problems with some kind of compilers. Was also checked that macros are used correctly!
*/

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef TRUE
#define TRUE 1U
#endif

#ifndef LITTLE
#define LITTLE 1
#endif

#ifndef BIG
#define BIG 2
#endif

#ifndef MACHINE_ENDIAN
#define MACHINE_ENDIAN LITTLE
#endif


#if(LITTLE == MACHINE_ENDIAN)
#ifndef ntohs
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohs(a) (((((uint16_t)(a)) >> 8) & 0x00ffU) | \
                  (((uint16_t)(((uint16_t)(a)) << 8)) & 0xff00U))
#endif

#ifndef ntohl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohl(a) (((((uint32_t)(a)) >> 24) & 0x000000ffU) | \
                  ((((uint32_t)(a)) >> 8 ) & 0x0000ff00U) | \
                  ((uint32_t)(((uint32_t)(a)) << 8 ) & 0x00ff0000U) | \
                  ((uint32_t)(((uint32_t)(a)) << 24) & 0xff000000U))
#endif

#ifndef ntohll
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohll(a) (((((uint64_t)(a)) & 0xFF00000000000000ULL) >> 56U) | \
                   ((((uint64_t)(a)) & 0x00FF000000000000ULL) >> 40U) | \
                   ((((uint64_t)(a)) & 0x0000FF0000000000ULL) >> 24U) | \
                   ((((uint64_t)(a)) & 0x000000FF00000000ULL) >> 8U)  | \
                   ((((uint64_t)(a)) & 0x00000000FF000000ULL) << 8U)  | \
                   ((((uint64_t)(a)) & 0x0000000000FF0000ULL) << 24U) | \
                   ((((uint64_t)(a)) & 0x000000000000FF00ULL) << 40U) | \
                   ((((uint64_t)(a)) & 0x00000000000000FFULL) << 56U))
#endif

#ifndef htons
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htons(a) ntohs(a)
#endif

#ifndef htonl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htonl(a) ntohl(a)
#endif

#ifndef htonll
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htonll(a) ntohll(a)
#endif

#elif (BIG == MACHINE_ENDIAN)
#ifndef ntohs
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohs(a) (a)
#endif

#ifndef ntohl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define ntohl(a) (a)
#endif

#ifndef htons
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htons(a) (a)
#endif

#ifndef htonl
/** @violates @ref common_types_h_REF_1 MISRA rule 19.7 */
#define htonl(a) (a)
#endif
#else
#error "Setup MACHINE_ENDIAN! Options: BIG or LITTLE."
#endif

/* Definition of the platform specific types */
typedef signed char sint8_t;
typedef signed short sint16_t;
typedef signed int sint32_t;
typedef unsigned char boolean;
/* Needed by the PTPDRV for timestamps storage and calculations */
typedef unsigned long long uint64;
typedef signed long long sint64_t;

#if (!defined TYPEDEFS_H) && (! defined _STDINT) && (! defined _SYS__STDINT_H ) && (!defined _EWL_CSTDINT)/* ewl_library & S32K SDK - workaround for typedefs collisions */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef volatile unsigned char vuint8_t;
typedef volatile unsigned short vuint16_t;
typedef volatile unsigned int vuint32_t;
typedef volatile char vint8_t;
typedef volatile short vint16_t;
typedef volatile int vint32_t;
#endif  /* _TYPEDEFS_H_ */

/* Old type definitions used by FECLLD */
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
#if(!defined TYPEDEFS_H) && (! defined _STDINT) && (! defined _SYS__STDINT_H) && (!defined _EWL_CSTDINT)/* eCos & S32K SDK- workaround for typedefs collisions */
typedef sint64_t int64_t;
typedef sint32_t int32_t;
typedef sint16_t int16_t;
typedef sint8_t int8_t;
#endif  /* _TYPEDEFS_H_ */
typedef sint8_t sint8;
typedef sint16_t sint16;
typedef sint32_t sint32;

//#if(!defined TYPEDEFS_H) && (!defined _STDINT) /* eCos & S32K SDK - workaround for typedefs collisions */
//    typedef unsigned long long uint64_t;
//#endif

typedef int int_t;
typedef char char_t;
typedef unsigned int uint_t;
typedef double float64_t;
typedef unsigned int uaddr_t;

#endif /* COMMON_TYPES_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
