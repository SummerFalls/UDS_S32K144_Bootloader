/*
 * @ 名称: LIN_tp.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef LIN_TP_H_
#define LIN_TP_H_

#include "LIN_tp_cfg.h"

#ifdef EN_LIN_TP

#include "multi_cyc_fifo.h"

void LINTP_MainFun(void);

void LINTP_SytstemTickControl(void);

void LINTP_Init(void);

#endif /* EN_LIN_TP */

#endif /* LIN_TP_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
