/*
 * @ 名称: toolchain.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef TOOLCHAIN_H_
#define TOOLCHAIN_H_

/*==================================================================================================
                                         MISRA VIOLATIONS
==================================================================================================*/

/**
* @page misra_violations MISRA-C:2004 violations
*
* @section toolchain_h_REF_1
* Violates MISRA 2004 Advisory Rule 19.7, A function should be used in preference to a function-like macro.
* It is not possible to use function to abstract data attribute definition.
*/

#if defined __ghs__ || defined (__GNUC__)
#define ASM_KEYWORD         __asm
#define INTERRUPT_FUNC      __interrupt
#define INLINE              inline
#define PACKED_STRUCT_BEGIN
#define PACKED_STRUCT_FIELD(x) x __attribute__((packed))
#define PACKED_STRUCT_END   __attribute__((packed))
/** @violates @ref toolchain_h_REF_1 MISRA rule 19.7 */
#define ALIGNEDXB(x)        __attribute__((aligned(x)))
#elif defined __DCC__
#define ASM_KEYWORD         __asm volatile
#define INTERRUPT_FUNC      __interrupt__
#define INLINE              __inline__
#define PACKED_STRUCT_BEGIN __packed__
#define PACKED_STRUCT_FIELD(x) x
#define PACKED_STRUCT_END
/** @violates @ref toolchain_h_REF_1 MISRA rule 19.7 */
#define ALIGNEDXB(x)        __attribute__((aligned(x)))
#elif defined __MWERKS__
#define INLINE              inline
// #error "For this compiler it must be updated here"
#elif defined __CC_ARM
#define ASM_KEYWORD         __asm
#define INTERRUPT_FUNC      __irq
/* NOTE: __inline must be used (with __) because we use inlining in C90 */
#if 1 /* NOTE: inlining is not working correctly between modules in DS-5,
             so it must be disabled. Otherwise the code would not compile or
             the INLINEs would have to be removed at problematic locations */
#define INLINE          __inline
#else
#define INLINE
#endif
#define PACKED_STRUCT_BEGIN
#define PACKED_STRUCT_FIELD(x) x __attribute__((packed))
#define PACKED_STRUCT_END   __attribute__((packed))
/** @violates @ref toolchain_h_REF_1 MISRA rule 19.7 */
#define ALIGNEDXB(x)        __attribute__((aligned(x)))
#elif defined __IAR_SYSTEMS_ICC__
#define ASM_KEYWORD         __asm volatile
#define INTERRUPT_FUNC      __interrupt
#define INLINE              inline
#if 1 /* Applicable to version 7.50.3 */
#define STRINGIZE(X) #X
#define PACKED_STRUCT_BEGIN     __packed
#define PACKED_STRUCT_FIELD(x)  x
#define PACKED_STRUCT_END
/** @violates @ref toolchain_h_REF_1 MISRA rule 19.7 */
#define ALIGNEDXB(n)            _Pragma(STRINGIZE(data_alignment=n))
#else /* Applicable to version 8.30.2 */
#define PACKED_STRUCT_BEGIN
#define PACKED_STRUCT_FIELD(x)  x
#define PACKED_STRUCT_END       __attribute__((packed))
/** @violates @ref toolchain_h_REF_1 MISRA rule 19.7 */
#define ALIGNEDXB(n)            __attribute__((aligned(n)))
#endif
#else
#error "Unknown toolchain"
#endif

#endif /* TOOLCHAIN_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
