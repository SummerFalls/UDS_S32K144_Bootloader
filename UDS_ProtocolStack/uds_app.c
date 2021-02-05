/*
 * @ 名称: uds_app.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "uds_app.h"
#include "TP.h"
#include "Boot.h"
#include "fls_app.h"
#include "uds_alg_hal.h"

/*********************************************************/

/*UDS init*/
void UDS_Init(void)
{
#ifdef EN_DELAY_TIME
    //    gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime = UdsAppTimeToCount(DELAY_MAX_TIME_MS);
#endif

    /*UDS alg hal init*/
    UDS_ALG_HAL_Init();
}

/*uds main function. ISO14229*/
void UDS_MainFun(void)
{
    uint8 UDSSerIndex = 0u;
    uint8 UDSSerNum = 0u;
    tUdsAppMsgInfo stUdsAppMsg = {0u, 0u, {0u}, NULL_PTR};
    uint8 isFindService = FALSE;
    uint8 SupSerItem = 0u;
    tUDSService *pstUDSService = NULL_PTR;

#ifdef EN_ALG_SW
    UDS_ALG_HAL_AddSWTimerTickCnt();
#endif

    if (TRUE == IsS3ServerTimeout()) {
        /*If s3 server timeout, back default session mode.*/
        SetCurrentSession(DEFALUT_SESSION);

        /*set security level. If S3server timeout, clear current security.*/
        SetSecurityLevel(NONE_SECURITY);

        Flash_InitDowloadInfo();
    }

    /*read data from can tp*/
    if (TRUE == TP_ReadAFrameDataFromTP(&stUdsAppMsg.xUdsId,
                                        &stUdsAppMsg.xDataLen,
                                        stUdsAppMsg.aDataBuf)) {
        SetIsRxUdsMsg(TRUE);

        if (TRUE != IsCurDefaultSession()) {
            /*restart s3server time*/
            RestartS3Server();
        }

        /*save request id type.*/
        SaveRequestIdType(stUdsAppMsg.xUdsId);
    } else {
        return;
    }

    /*get UDS service Information, start PTR and service item*/
    pstUDSService = GetUDSServiceInfo(&SupSerItem);

    /*get UDS service ID*/
    UDSSerNum = stUdsAppMsg.aDataBuf[0u];

    while ((UDSSerIndex < SupSerItem) && (NULL_PTR != pstUDSService)) {
        if (UDSSerNum == pstUDSService[UDSSerIndex].SerNum) {
            isFindService = TRUE;

            if (TRUE != IsCurRxIdCanRequest(pstUDSService[UDSSerIndex].SupReqMode)) {
                /*received ID cann't request this service.*/
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);

                break;
            }

            if (TRUE != IsCurSeesionCanRequest(pstUDSService[UDSSerIndex].SessionMode)) {
                /*currnet session mode cann't request ths service.*/
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);

                break;
            }

            if (TRUE != IsCurSecurityLevelRequet(pstUDSService[UDSSerIndex].ReqLevel)) {
                /*current security level cann't request this service.*/
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);

                break;
            }

            stUdsAppMsg.pfUDSTxMsgServiceCallBack = NULL_PTR;

            /*find service, and do it.*/
            if (NULL_PTR != pstUDSService[UDSSerIndex].pfSerNameFun) {
                pstUDSService[UDSSerIndex].pfSerNameFun((tUDSService *)&pstUDSService[UDSSerIndex], &stUdsAppMsg);
            } else {
                /*current security level cann't request this service.*/
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
            }

            break;
        }

        UDSSerIndex++;
    }

    if (TRUE != isFindService) {
        /*response not support service.*/
        SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
    }

    if (0u != stUdsAppMsg.xDataLen) {
        stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();

        (void)TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId,
                                     stUdsAppMsg.pfUDSTxMsgServiceCallBack,
                                     stUdsAppMsg.xDataLen,
                                     stUdsAppMsg.aDataBuf);
    }
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
