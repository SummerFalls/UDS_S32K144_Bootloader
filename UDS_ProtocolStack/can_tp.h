/*
 * @ ����: can_tp.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef CAN_TP_H_
#define CAN_TP_H_

#include "can_tp_cfg.h"

#ifdef EN_CAN_TP

#include "multi_cyc_fifo.h"

/*uds network man function*/
extern void CANTP_MainFun(void);

/*can tp system tick control*/
extern void CANTP_SytstemTickControl(void);

/*Init CAN TP list*/
extern void CANTP_Init(void);

#endif /*#ifdef EN_CAN_TP*/

#endif /* CAN_TP_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
