/*
 * @ ����: TP.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef TP_H_
#define TP_H_

#include "includes.h"
#include "TP_cfg.h"

#ifdef EN_CAN_TP
#include "can_tp.h"
#endif

#ifdef EN_LIN_TP
#include "LIN_TP.h"
#endif

void TP_Init(void);

void TP_MainFun(void);

void TP_SystemTickCtl(void);

boolean TP_ReadAFrameDataFromTP(uint32 *o_pRxMsgID,
                                uint32 *o_pxRxDataLen,
                                uint8 *o_pDataBuf);

boolean TP_WriteAFrameDataInTP(const uint32 i_TxMsgID,
                               const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                               const uint32 i_xTxDataLen,
                               const uint8 *i_pDataBuf);

#endif /* TP_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
