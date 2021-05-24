/*
 * @ ����: can_driver.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "can_cfg.h"
#include "can_tp_cfg.h"

#ifdef EN_CAN_TP

typedef struct
{
    uint32_t ucRxDataLen;       /* RX CAN hardware data len */
    uint32_t usRxDataId;        /* RX data ID */
    uint8_t aucDataBuf[64u];    /* RX data buffer */
} tRxCanMsg;

void InitCAN(void);

void RxCANMsgMainFun(void);

void TxCANMsgMainFun(void);

uint8_t TransmitCANMsg(const uint32_t i_usCANMsgID,
                       const uint8_t i_ucDataLen,
                       const uint8_t *i_pucDataBuf,
                       const tpfNetTxCallBack i_pfNetTxCallBack,
                       const uint32_t i_txBlockingMaxtime);

void TransmittedCanMsgCallBack(void);

void SetWaitTransmittedMsg(void);

uint8_t IsTransmittedMsg(void);

#endif /* EN_CAN_TP */

#endif /* CAN_DRIVER_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
