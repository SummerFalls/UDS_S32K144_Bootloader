/*
 * @ 名称: TP.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "TP.h"

/*******************************************************************************
 * User Include
 ******************************************************************************/
#ifdef EN_CAN_TP
#include "can_tp.h"
#endif /*#ifdef EN_CAN_TP*/

#ifdef EN_LIN_TP
#include "LIN_TP.h"
#endif /*#ifdef EN_LIN_TP*/

#include "multi_cyc_fifo.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*FUNCTION**********************************************************************
 *
 * Function Name : TP_Init
 * Description   : This function initial this module.
 *
 * Implements : TP_Init_Activity
 *END**************************************************************************/
void TP_Init(void)
{

#ifdef EN_CAN_TP
    CANTP_Init();

#endif /*#ifdef EN_CAN_TP*/

#ifdef EN_LIN_TP
    LINTP_Init();
#endif /*#ifdef EN_LIN_TP*/


}

/*FUNCTION**********************************************************************
 *
 * Function Name : TP_mainFunc
 * Description   : This function main function this module.
 *
 * Implements : TP_Init_Activity
 *END**************************************************************************/
void TP_MainFun(void)
{
#ifdef EN_CAN_TP
    CANTP_MainFun();
#endif   /*#ifdef EN_CAN_TP*/

#ifdef EN_LIN_TP
    LINTP_MainFun();
#endif /*#ifdef EN_LIN_TP*/


}

/*TP system tick control*/
void TP_SystemTickCtl(void)
{
#ifdef EN_CAN_TP
    CANTP_SytstemTickControl();
#endif   /*#ifdef EN_CAN_TP*/

#ifdef EN_LIN_TP
    LINTP_SytstemTickControl();
#endif /*#ifdef EN_LIN_TP*/

}


/*read a frame from TP Rx FIFO. If no data can read return FALSE, else return TRUE*/
boolean TP_ReadAFrameDataFromTP(uint32 *o_pRxMsgID,
                                      uint32 *o_pxRxDataLen,
                                      uint8 *o_pDataBuf)
{
    tErroCode eStatus;
    tLen xReadDataLen = 0u;

    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    ASSERT(NULL_PTR == o_pRxMsgID);
    ASSERT(NULL_PTR == o_pDataBuf);
    ASSERT(NULL_PTR == o_pxRxDataLen);

    /*can read data from buf*/
    GetCanReadLen(RX_TP_QUEUE_ID, &xReadDataLen, &eStatus);
    if(ERRO_NONE != eStatus || (xReadDataLen < sizeof(tUDSAndTPExchangeMsgInfo)))
    {
        return FALSE;
    }

    /*read receive ID and data len*/
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                    sizeof(exchangeMsgInfo),
                    (uint8 *)&exchangeMsgInfo,
                    &xReadDataLen,
                    &eStatus);
    if(ERRO_NONE != eStatus || sizeof(exchangeMsgInfo) != xReadDataLen)
    {
        TPDebugPrintf("Read data len erro!\n");
        return FALSE;
    }

    /*read data from fifo*/
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                    exchangeMsgInfo.dataLen,
                    o_pDataBuf,
                    &xReadDataLen,
                    &eStatus);
    if(ERRO_NONE != eStatus || (exchangeMsgInfo.dataLen != xReadDataLen))
    {
        TPDebugPrintf("Read data erro!\n");
        return FALSE;
    }

    *o_pRxMsgID = exchangeMsgInfo.msgID;
    *o_pxRxDataLen = exchangeMsgInfo.dataLen;

    return TRUE;
}

/*write a frame data  to tp TX FIFO*/
boolean TP_WriteAFrameDataInTP(const uint32 i_TxMsgID,
                                     const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                                     const uint32 i_xTxDataLen,
                                     const uint8 *i_pDataBuf)
{
    tErroCode eStatus;
    tLen xCanWriteLen = 0u;
    tLen xWritDataLen = (tLen)i_xTxDataLen;

    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;
    uint32 totalWriteDataLen = i_xTxDataLen + sizeof(tUDSAndTPExchangeMsgInfo);

    exchangeMsgInfo.msgID = (uint32)i_TxMsgID;
    exchangeMsgInfo.dataLen = (uint32)i_xTxDataLen;
    exchangeMsgInfo.pfCallBack = (tpfUDSTxMsgCallBack)i_pfUDSTxMsgCallBack;

    ASSERT(NULL_PTR == i_pDataBuf);

    /*check transmit ID*/
    if(i_TxMsgID != TP_GetConfigTxMsgID())
    {
        return FALSE;
    }

    if(0u == xWritDataLen)
    {
        return FALSE;
    }

    /*check can wirte data len*/
    GetCanWriteLen(TX_TP_QUEUE_ID, &xCanWriteLen, &eStatus);
    if(ERRO_NONE != eStatus || xCanWriteLen < totalWriteDataLen)
    {
        return FALSE;
    }

    /*write uds transmitt ID*/
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8 *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);
    if(ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    /*write data in fifo*/
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8 *)i_pDataBuf, xWritDataLen, &eStatus);
    if(ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    return TRUE;
}



/*FUNCTION**********************************************************************
 *
 * Function Name : TP_Deinit
 * Description   : This function initial this module.
 *
 * Implements : TP_Deinit_Activity
 *END**************************************************************************/
void TP_Deinit(void)
{

}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
