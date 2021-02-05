/*
 * @ 名称: TP.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef TP_H_
#define TP_H_

/*******************************************************************************
 * User Include
 ******************************************************************************/
#include "includes.h"
#include "TP_cfg.h"

#ifdef EN_CAN_TP
#include "can_tp.h"
#endif

#ifdef EN_LIN_TP
#include "LIN_TP.h"
#endif

extern void TP_Init(void);

/*!
 * @brief To peroid run the function for TP.
 *
 */
extern void TP_MainFun(void);

/*TP system tick control*/
extern void TP_SystemTickCtl(void);


/*read a frame  tp data  from UDS to Tp TxFIFO. If no data can read return FALSE, else return TRUE*/
extern boolean TP_ReadAFrameDataFromTP(uint32 *o_pRxMsgID,
                                       uint32 *o_pxRxDataLen,
                                       uint8 *o_pDataBuf);

/*write a frame data from  Tp to UDS RxFIFO*/
extern boolean TP_WriteAFrameDataInTP(const uint32 i_TxMsgID,
                                      const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                                      const uint32 i_xTxDataLen,
                                      const uint8 *i_pDataBuf);

extern void TP_Deinit(void);

#endif /* TP_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
