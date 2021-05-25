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

#ifdef UDS_PROJECT_FOR_BOOTLOADER
#ifdef EN_DELAY_TIME
extern tJumpAppDelayTimeInfo gs_stJumpAPPDelayTimeInfo;
#endif
#endif

void UDS_Init(void)
{
#ifdef UDS_PROJECT_FOR_BOOTLOADER
#ifdef EN_DELAY_TIME
    gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime = UdsAppTimeToCount(DELAY_MAX_TIME_MS);
#endif
#endif

    UDS_ALG_HAL_Init();
}

void UDS_MainFun(void)
{
    uint8 UDSSerIndex = 0u;
    uint8 UDSSerNum = 0u;
    tUdsAppMsgInfo stUdsAppMsg = {0u, 0u, {0u}, NULL_PTR};
    uint8 isFindService = FALSE;
    uint8 SupSerItem = 0u;
    tUDSService *pstUDSService = NULL_PTR;
#if defined (EN_AES_SA_ALGORITHM_SW) || defined (EN_ZLG_SA_ALGORITHM)
    UDS_ALG_HAL_AddSWTimerTickCnt();
#endif

    if (TRUE == IsS3ServerTimeout())
    {
        /* If s3 server timeout, back default session mode */
        SetCurrentSession(DEFALUT_SESSION);
        /* Set security level. If S3server timeout, clear current security */
        SetSecurityLevel(NONE_SECURITY);
        Flash_InitDowloadInfo();
    }

    /* Read data from can TP */
    if (TRUE == TP_ReadAFrameDataFromTP(&stUdsAppMsg.xUdsId,
                                        &stUdsAppMsg.xDataLen,
                                        stUdsAppMsg.aDataBuf))
    {
#ifdef UDS_PROJECT_FOR_BOOTLOADER
        SetIsRxUdsMsg(TRUE);
#endif

        if (TRUE != IsCurDefaultSession())
        {
            /* Restart S3Server time */
            RestartS3Server();
        }

        /* Save request ID type */
        SaveRequestIdType(stUdsAppMsg.xUdsId);
    }
    else
    {
        return;
    }

    /* Get UDS service Information, start PTR and service item */
    pstUDSService = GetUDSServiceInfo(&SupSerItem);
    /* Get UDS service ID */
    UDSSerNum = stUdsAppMsg.aDataBuf[0u];

    while ((UDSSerIndex < SupSerItem) && (NULL_PTR != pstUDSService))
    {
        if (UDSSerNum == pstUDSService[UDSSerIndex].SerNum)
        {
            isFindService = TRUE;

            if (TRUE != IsCurRxIdCanRequest(pstUDSService[UDSSerIndex].SupReqMode))
            {
                /* received ID can't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
                break;
            }

            if (TRUE != IsCurSeesionCanRequest(pstUDSService[UDSSerIndex].SessionMode))
            {
                /* Current session mode can't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
                break;
            }

            if (TRUE != IsCurSecurityLevelRequet(pstUDSService[UDSSerIndex].ReqLevel))
            {
                /* Current security level can't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
                break;
            }

            stUdsAppMsg.pfUDSTxMsgServiceCallBack = NULL_PTR;

            /* Find service and do it */
            if (NULL_PTR != pstUDSService[UDSSerIndex].pfSerNameFun)
            {
                pstUDSService[UDSSerIndex].pfSerNameFun((tUDSService *)&pstUDSService[UDSSerIndex], &stUdsAppMsg);
            }
            else
            {
                /* Current security level cann't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
            }

            break;
        }

        UDSSerIndex++;
    }

    if (TRUE != isFindService)
    {
        /* Response not support service */
        SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
    }

    if (0u != stUdsAppMsg.xDataLen)
    {
        stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();
        (void)TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId,
                                     stUdsAppMsg.pfUDSTxMsgServiceCallBack,
                                     stUdsAppMsg.xDataLen,
                                     stUdsAppMsg.aDataBuf);
    }
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
