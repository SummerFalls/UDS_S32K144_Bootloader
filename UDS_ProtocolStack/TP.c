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

#ifdef EN_CAN_TP
#include "can_tp.h"
#endif

#ifdef EN_LIN_TP
#include "LIN_TP.h"
#endif

#include "multi_cyc_fifo.h"

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
#endif

#ifdef EN_LIN_TP
    LINTP_Init();
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TP_mainFunc
 * Description   : To period run the function for TP.
 *
 * Implements : TP_Init_Activity
 *END**************************************************************************/
void TP_MainFun(void)
{
#ifdef EN_CAN_TP
    CANTP_MainFun();
#endif

#ifdef EN_LIN_TP
    LINTP_MainFun();
#endif
}

/* TP system tick control */
void TP_SystemTickCtl(void)
{
#ifdef EN_CAN_TP
    CANTP_SytstemTickControl();
#endif

#ifdef EN_LIN_TP
    LINTP_SytstemTickControl();
#endif
}

/* Read a frame from TP RX FIFO. If no data can read return FALSE, else return TRUE */
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
    /* CAN read data from buffer */
    GetCanReadLen(RX_TP_QUEUE_ID, &xReadDataLen, &eStatus);

    if (ERRO_NONE != eStatus || (xReadDataLen < sizeof(tUDSAndTPExchangeMsgInfo)))
    {
        return FALSE;
    }

    /* Read receive ID and data len */
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                     sizeof(exchangeMsgInfo),
                     (uint8 *)&exchangeMsgInfo,
                     &xReadDataLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || sizeof(exchangeMsgInfo) != xReadDataLen)
    {
        TPDebugPrintf("Read data len error!\n");
        return FALSE;
    }

    /* Read data from FIFO */
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                     exchangeMsgInfo.dataLen,
                     o_pDataBuf,
                     &xReadDataLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || (exchangeMsgInfo.dataLen != xReadDataLen))
    {
        TPDebugPrintf("Read data error!\n");
        return FALSE;
    }

    *o_pRxMsgID = exchangeMsgInfo.msgID;
    *o_pxRxDataLen = exchangeMsgInfo.dataLen;
    return TRUE;
}

/* Write a frame data to TP TX FIFO */
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

    /* Check transmit ID */
    if (i_TxMsgID != TP_GetConfigTxMsgID())
    {
        return FALSE;
    }

    if (0u == xWritDataLen)
    {
        return FALSE;
    }

    /* Check can write data len */
    GetCanWriteLen(TX_TP_QUEUE_ID, &xCanWriteLen, &eStatus);

    if (ERRO_NONE != eStatus || xCanWriteLen < totalWriteDataLen)
    {
        return FALSE;
    }

    /* Write UDS transmit ID */
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8 *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    /* Write data in FIFO */
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8 *)i_pDataBuf, xWritDataLen, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
