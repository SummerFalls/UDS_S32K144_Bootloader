/*
 * @ 名称: can_driver.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "can_cfg.h"
#include "can_tp_cfg.h"

//#define RX_CAN_FIFO ('r') /*Rx can fifo number*/

typedef struct
{
    uint32_t ucRxDataLen;      /*RX can harware data len*/
    uint32_t usRxDataId;      /*RX data ID*/
    uint8_t aucDataBuf[64u];   /*RX data buf*/
}tRxCanMsg;
#define DLEN_ID_LEN 8u  /* DLEN_ID_LEN include ucRxDataLen len add usRxDataId len */

/*init can */
extern void InitCAN(void);

/*Init rx can fifo*/
extern void InitRxCanFIFO(void);

/*RX CAn message main function*/
extern void RxCANMsgMainFun(void);

extern void TxCANMsgMainFun(void);

/*can transmite message*/
extern uint8_t TransmiteCANMsg(const uint32_t i_usCANMsgID,
                                const uint8_t i_ucDataLen,
                                const uint8_t *i_pucDataBuf,
                                const tpfNetTxCallBack i_pfNetTxCallBack,
                                const uint32_t i_txBlockingMaxtime);

/*transmitted can message callback*/
extern void TransmittedCanMsgCallBack(void);

/*set wait transmitted can message*/
extern void SetWaitTransmittedMsg(void);

/*Is transmitted can messag? If transmitted return TRUE, else return FALSE.*/
extern uint8_t IsTransmittedMsg(void);

#endif /* CAN_DRIVER_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
