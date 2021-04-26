/*
 * @ 名称: can_driver.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "can_driver.h"
#include "user_config.h"
#include "TP.h"

#if 1
volatile uint8_t g_ucIsCountTime = 0u;
volatile uint32_t g_ulTxStartTime = 0u;
volatile uint32_t g_ulRxEndTime[2] = {0u};
uint8_t g_ucRxTimeRecord = 0u;
#endif  //end of 1

static void CAN_Filiter_RXIndividual(void);
static void Config_Rx_Buffer(void);
static void Config_Tx_Buffer(void);
static uint8_t IsRxCANMsgId(uint32_t i_usRxMsgId);
static void CheckCANTranmittedStatus(void);

void RxCANMsgMainFun(void);
void TransmittedCanMsgCallBack(void);

/* Config CAN filiter is individual */
static void CAN_Filiter_RXIndividual(void)
{
    uint32_t i;

    /* Set the ID mask type is individual */
    FLEXCAN_DRV_SetRxMaskType(0u, FLEXCAN_RX_MASK_INDIVIDUAL);

    for (i = 0u; i < g_ucRxCANMsgIDNum; i++) {
        FLEXCAN_DRV_SetRxIndividualMask(0u, g_astRxMsgConfig[i].RxID_Type, g_astRxMsgConfig[i].usRxMailBox, g_astRxMsgConfig[i].usRxMask);
    }
}

/* Config rx MB buffer */
static void Config_Rx_Buffer(void)
{
    uint32_t i;

    /* Configure RX buffer with index RX_MAILBOX for Function ID and Physical ID*/
    for (i = 0u; i < g_ucRxCANMsgIDNum; i++) {
#ifdef IsUse_CAN_Pal_Driver
        /* According to Rx msg ID type to configure MB msg ID type */
        buff_RxTx_Cfg.idType = g_astRxMsgConfig[i].RxID_Type;
        CAN_ConfigRxBuff(&can_pal1_instance, g_astRxMsgConfig[i].usRxMailBox, &buff_RxTx_Cfg, g_astRxMsgConfig[i].usRxID);

#else
        buff_RxTx_Cfg.msg_id_type = g_astRxMsgConfig[i].RxID_Type;
        FLEXCAN_DRV_ConfigRxMb(INST_CANCOM1, g_astRxMsgConfig[i].usRxMailBox, &buff_RxTx_Cfg, g_astRxMsgConfig[i].usRxID);
#endif  //end of IsUse_CAN_Pal_Driver
    }
}

/* Config tx MB buffer */
static void Config_Tx_Buffer(void)
{
    /* According to Tx msg ID type to config MB msg ID type */
    buff_RxTx_Cfg.msg_id_type = g_stTxMsgConfig.TxID_Type;

#ifdef IsUse_CAN_Pal_Driver
    /* Config Tx buffer with index Tx mailbox */
    CAN_ConfigTxBuff(&can_pal1_instance, g_stTxMsgConfig.ucTxMailBox, &buff_RxTx_Cfg);
#else
    FLEXCAN_DRV_ConfigTxMb(INST_CANCOM1, g_stTxMsgConfig.ucTxMailBox, &buff_RxTx_Cfg, g_stTxMsgConfig.usTxID);
#endif  //end of IsUse_CAN_Pal_Driver
}

/*Is Rx can message ID? If is rx can message id return TRUE, else return FALSE.*/
static uint8_t IsRxCANMsgId(uint32_t i_usRxMsgId)
{
    uint8_t Index = 0u;

    while (Index < g_ucRxCANMsgIDNum) {
        if (i_usRxMsgId  == g_astRxMsgConfig[Index].usRxID ) {
            return TRUE;
        }

        Index++;
    }

    return FALSE;
}

/*check can transmitted data successfull?*/
static void CheckCANTranmittedStatus(void)
{
    status_t Can_Tx_Statu;

#ifdef IsUse_CAN_Pal_Driver
    Can_Tx_Statu = CAN_GetTransferStatus(&can_pal1_instance, g_stTxMsgConfig.ucTxMailBox);

#else
    Can_Tx_Statu = FLEXCAN_DRV_GetTransferStatus(INST_CANCOM1, g_stTxMsgConfig.ucTxMailBox);
#endif  //end of IsUse_CAN_Pal_Driver

    if (STATUS_SUCCESS == Can_Tx_Statu) {
        if (NULL != g_stTxMsgConfig.pfCallBack) {
            g_stTxMsgConfig.pfCallBack();
            g_stTxMsgConfig.pfCallBack = NULL;
        }
    }
}

/* Can operation completed callback function */
#ifdef IsUse_CAN_Pal_Driver
static void CAN_RxTx_Interrupt_Handle(uint32_t instance,
                                      flexcan_event_type_t eventType,
                                      uint32_t objIdx,
                                      void *driverState)
{
    uint32_t i;

    DEV_ASSERT(driverState != NULL);

    switch (eventType) {
        case CAN_EVENT_RX_COMPLETE:
            RxCANMsgMainFun();
            break;

        case CAN_EVENT_TX_COMPLETE:
            TxCANMsgMainFun();
            break;

        default:
            break;
    }

    /* Enable MB interrupt*/
    for (i = 0u; i < g_ucRxCANMsgIDNum; i++) {
        CAN_Receive(&can_pal1_instance,  g_astRxMsgConfig[i].usRxMailBox, &recvMsg);
    }
}

#else
static void CAN_RxTx_Interrupt_Handle(uint32_t instance,
                                      flexcan_event_type_t eventType,
                                      uint32_t objIdx,
                                      flexcan_state_t *flexcanState)
{
    DEV_ASSERT(driverState != NULL);

    switch (eventType) {
        case FLEXCAN_EVENT_RX_COMPLETE:
            RxCANMsgMainFun();

            /* Enable MB interrupt*/
            if (RX_MAILBOX_FUN_ID == objIdx) {
                FLEXCAN_DRV_Receive(INST_CANCOM1, RX_MAILBOX_FUN_ID, &recvMsg);
            } else if (RX_MAILBOX_PHY_ID == objIdx) {
                FLEXCAN_DRV_Receive(INST_CANCOM1, RX_MAILBOX_PHY_ID, &recvMsg);
            } else {
                ;
            }

            break;

        case FLEXCAN_EVENT_TX_COMPLETE:
            TxCANMsgMainFun();
            break;

        default:
            break;
    }
}
#endif  //end of IsUse_CAN_Pal_Driver

/* Init CAN moudule */
void InitCAN(void)
{
#ifdef IsUse_CAN_Pal_Driver
    /*Init can basical elements */
    CAN_Init(&can_pal1_instance, &can_pal1_Config0);

    /* install can rx and tx interrupt callback function */
    CAN_InstallEventCallback(&can_pal1_instance, (can_callback_t)CAN_RxTx_Interrupt_Handle, NULL);
#else
    /*Init can basical elements */
    FLEXCAN_DRV_Init(INST_CANCOM1, &canCom1_State, &canCom1_InitConfig0);

    /* install can rx and tx interrupt callback function */
    FLEXCAN_DRV_InstallEventCallback(INST_CANCOM1, (flexcan_callback_t)CAN_RxTx_Interrupt_Handle, NULL);
#endif  //end of IsUse_CAN_Pal_Driver

    /* config MBn to Rx buffer */
    Config_Rx_Buffer();

    /* config MBn to Tx buffer */
    Config_Tx_Buffer();

    /* Can Rx individual filiter */
    CAN_Filiter_RXIndividual();

    /* Start receiving data in RX_MAILBOX and Enable MBn of Rx buffer interrupt */
    {
        uint32_t i = 0u;

        for (i = 0u; i < g_ucRxCANMsgIDNum; i++) {
#ifdef IsUse_CAN_Pal_Driver
            CAN_Receive(&can_pal1_instance, g_astRxMsgConfig[i].usRxMailBox, &recvMsg);

#else
            FLEXCAN_DRV_Receive(INST_CANCOM1, g_astRxMsgConfig[i].usRxMailBox, &recvMsg);
#endif  //end of IsUse_CAN_Pal_Driver
        }
    }

}

///*Init rx can fifo*/
//void InitRxCanFIFO(void)
//{
//    tErroCode eStatus;
//    ApplyFifo((300u), RX_CAN_FIFO, &eStatus);
//    if(ERRO_NONE != eStatus)
//    {
//        while(1);
//    }
//}

/*CAN Rx*/

#if USE_CAN_RX == CAN_RX_INTERRUPUT
void RxCANMsgMainFun(void)
{
    tRxCanMsg stRxCANMsg = {0u};
    uint8_t CANDataIndex = 0u;

#ifdef IsUse_CAN_Pal_Driver
    /* Read CAN massage data from recieve buffer recvMsg */
    stRxCANMsg.usRxDataId = recvMsg.id;
    stRxCANMsg.ucRxDataLen = recvMsg.length;
#else
    stRxCANMsg.usRxDataId = recvMsg.msgId;
    stRxCANMsg.ucRxDataLen = recvMsg.dataLen;
#endif  //end of IsUse_CAN_Pal_Driver

    if ((0u != stRxCANMsg.ucRxDataLen) &&
            (TRUE == IsRxCANMsgId(stRxCANMsg.usRxDataId))) {

        /*read can message*/
        for (CANDataIndex = 0u; CANDataIndex < stRxCANMsg.ucRxDataLen; CANDataIndex++) {
            stRxCANMsg.aucDataBuf[CANDataIndex] = recvMsg.data[CANDataIndex];
        }

        if (TRUE != TP_DriverWriteDataInTP(stRxCANMsg.usRxDataId, stRxCANMsg.ucRxDataLen, stRxCANMsg.aucDataBuf)) {
            /*here is TP driver write data in TP failed, TP will lost CAN message*/
            while (1) {
            }
        }

#if 0
        /*write data in softer data FIFO*/
        GetCanWriteLen(RX_BUS_FIFO, &xCanWriteLen, &eStatus);

        if ((ERRO_NONE == eStatus) && (xCanWriteLen > acuCANMsgLen)) {
            /*write data in software fifo*/
            WriteDataInFifo(RX_BUS_FIFO, (uint8_t *)&stRxCANMsg, acuCANMsgLen, &eStatus);

            if (ERRO_NONE != eStatus) {
                while (1);
            }
        } else {
            while (1);
        }

#endif
    }
}
#else

#endif  ////end of USE_CAN_RX == CAN_RX_INTERRUPUT

/* CAN Tx */
#if USE_CAN_TX == CAN_TX_INTERRUPUT
void TxCANMsgMainFun(void)
{
    /*check */
    CheckCANTranmittedStatus();
}
#else

#endif  //end of USE_CAN_TX == CAN_TX_INTERRUPUT

/* CAN communication error  */
#if USE_CAN_ERRO == CAN_ERRO_INTERRUPUT

#else
void CANErroMainFun(void)
{

}
#endif  //end of USE_CAN_ERRO == CAN_ERRO_INTERRUPUT

/*can transmit message*/
uint8_t TransmitCANMsg(const uint32_t i_usCANMsgID,
                       const uint8_t i_ucDataLen,
                       const uint8_t *i_pucDataBuf,
                       const tpfNetTxCallBack i_pfNetTxCallBack,
                       const uint32_t i_txBlockingMaxtime)
{

    status_t CANTxStatus = STATUS_BUSY;
    uint8_t ret = 0u;

    /* change Tx massage length */
    buff_RxTx_Cfg.data_length = i_ucDataLen;

#ifdef IsUse_CAN_Pal_Driver
    uint8_t i;
    can_message_t message;

    DEV_ASSERT(i_pucDataBuf != NULL);

    if (i_usCANMsgID != g_stTxMsgConfig.usTxID) {
        return FALSE;
    }

    message.cs = 0u;
    message.id = i_usCANMsgID;
    message.length = i_ucDataLen;

    for (i = 0u; i < i_ucDataLen; i++) {
        message.data[i] = i_pucDataBuf[i];
    }

    CAN_Send(&can_pal1_instance, g_stTxMsgConfig.ucTxMailBox, &message);

#else
    DEV_ASSERT(i_pucDataBuf != NULL);

    if (i_usCANMsgID != g_stTxMsgConfig.usTxID) {
        return FALSE;
    }

    CANTxStatus = FLEXCAN_DRV_Send(INST_CANCOM1, g_stTxMsgConfig.ucTxMailBox, &buff_RxTx_Cfg, i_usCANMsgID, i_pucDataBuf);

    g_stTxMsgConfig.pfCallBack = i_pfNetTxCallBack;

#endif  //end of IsUse_CAN_Pal_Driver

    if (STATUS_SUCCESS == CANTxStatus) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }

    return ret;
}

/*transmitted can message*/
static uint8_t gs_ucIsTransmittedMsg = FALSE;

/*transmitted can message callback*/
void TransmittedCanMsgCallBack(void)
{
    gs_ucIsTransmittedMsg = TRUE;
}

/*set wait transmitted can message*/
void SetWaitTransmittedMsg(void)
{
    gs_ucIsTransmittedMsg = FALSE;
}

/*Is transmitted can messag? If transmitted return TRUE, else return FALSE.*/
uint8_t IsTransmittedMsg(void)
{
    return gs_ucIsTransmittedMsg;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
