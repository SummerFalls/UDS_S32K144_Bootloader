/*
 * @ 名称: uds_app_cfg.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "uds_app_cfg.h"
#include "TP.h"
#include "fls_app.h"
#include "boot.h"
#include "watchdog_hal.h"
#include "uds_alg_hal.h"

typedef struct
{
    uint32 StartAddr;         /*data start address*/
    uint32 DataLen;           /*data len*/
} tDowloadDataInfo;

/*define security access info*/
typedef struct
{
    uint8 SubfunctionNumber;    /*subfunction number*/
    uint8 RequestSession;       /*request session*/
    uint8 RequestIDMode;        /*request id mode*/
    uint8 RequestSecurityLevel; /*request security level*/
    void (*pfRoutine)(void);    /*routine*/
} tSecurityAccessInfo;

/*define write data subfunction*/
typedef struct
{
    uint8 Subfunction;      /*subfunction*/
    uint8 RequestSession;   /*request session*/
    uint8 RequestIdMode;    /*request id mode*/
    uint8 RequestLevel;     /*request level*/
    void (*pfRoutine)(void);/*routine*/
} tWriteDataByIdentifierInfo;

typedef enum
{
    ERASE_MEMORY_ROUTINE_CONTROL,       /*check erase memory routine control*/
    CHECK_SUM_ROUTINE_CONTROL,          /*check sum routine control*/
    CHECK_DEPENDENCY_ROUTINE_CONTROL    /*check dependency routine control*/
} tCheckRoutineCtlInfo;

#define DOWLOAD_DATA_ADDR_LEN (4u)      /*dowload data addr len*/
#define DOWLOAD_DATA_LEN (4u)           /*dowload data len*/

/*support function/physical ID request*/
#define ERRO_REQUEST_ID (0u)             /*received ID failled*/
#define SUPPORT_PHYSICAL_ADDR (1u << 0u) /*support physical ID request */
#define SUPPORT_FUNCTION_ADDR (1u << 1u)  /*support function ID request*/

/*********************************************************/
/***********************UDS App Const configuration Information************************/
typedef struct
{
    uint8 CalledPeriod;         /*called uds period*/
    /*security request count. If over this security request count, locked server some time.*/
    uint8 SecurityRequestCnt;
    tUdsTime xLockTime;         /*lock time*/
    tUdsTime xS3Server;         /*s3 server time. */
} tUdsTimeInfo;

/* UDS time control information config table*/
static const tUdsTimeInfo gs_stUdsAppCfg =
{
    1u,
    3u,
    10000u,
    5000u
};

/*uds app time to count*/
#define UdsAppTimeToCount(xTime) ((xTime) / gs_stUdsAppCfg.CalledPeriod)

/*get UDS s3 watermark timer. return s3 * S3_TIMER_WATERMARK_PERCENT / 100*/
uint32 UDS_GetUDSS3WatermarkTimerMs(void)
{
    const uint32 watermarkTimerMs = (gs_stUdsAppCfg.xS3Server * S3_TIMER_WATERMARK_PERCENT) / 100u;

    return (uint32)watermarkTimerMs;
}

#ifdef EN_DELAY_TIME
typedef struct
{
    boolean isReceiveUDSMsg;
    uint32 jumpToAPPDelayTime;
} tJumpAppDelayTimeInfo;

/* it should be UdsAppTimeToCount(DELAY_MAX_TIME_MS) for Parameter [jumpToAPPDelayTime], but it will occur Error when compiling... */
static tJumpAppDelayTimeInfo gs_stJumpAPPDelayTimeInfo = {FALSE, DELAY_MAX_TIME_MS};
#endif

/*********************************************************/
/***********************UDS Information************************/

/**********************UDS Information Static function************************/
static tUdsTime GetUdsS3ServerTime(void);

static void SubUdsS3ServerTime(tUdsTime i_SubTime);

static tUdsTime GetUdsSecurityReqLockTime(void);

static void SubUdsSecurityReqLockTime(tUdsTime i_SubTime);

typedef struct
{
    uint8 CurSessionMode;  /*current session mode. default/program/extend mode*/
    uint8 RequsetIdMode;   /*SUPPORT_PHYSICAL_ADDR/SUPPORT_FUNCTION_ADDR*/
    uint8 SecurityLevel;   /*current security level*/
    tUdsTime xUdsS3ServerTime;      /*uds s3 server time*/
    tUdsTime xSecurityReqLockTime;  /*security request lock time*/
} tUdsInfo;

/***********************UDS Information Static Global value************************/
/* UDS support Session mode? RequestId and Security level config */
static tUdsInfo gs_stUdsInfo =
{
    DEFALUT_SESSION,
    ERRO_REQUEST_ID,
    NONE_SECURITY,
    0u,
    0u,
};

static tUdsTime GetUdsS3ServerTime(void)
{
    return (gs_stUdsInfo.xUdsS3ServerTime);
}

static void SubUdsS3ServerTime(tUdsTime i_SubTime)
{
    gs_stUdsInfo.xUdsS3ServerTime -= i_SubTime;
}

static tUdsTime GetUdsSecurityReqLockTime(void)
{
    return (gs_stUdsInfo.xSecurityReqLockTime);
}

static void SubUdsSecurityReqLockTime(tUdsTime i_SubTime)
{
    gs_stUdsInfo.xSecurityReqLockTime -= i_SubTime;
}

/*Is security request lock timeout?*/
#pragma GCC diagnostic ignored "-Wunused-function"
static uint8 IsSecurityRequestLockTimeout(void)
{
    uint8 status = 0u;

    if(gs_stUdsInfo.xSecurityReqLockTime)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/***********************UDS Information Global function************************/
/*set current request id  SUPPORT_PHYSICAL_ADDR/SUPPORT_FUNCTION_ADDR */
#define SetRequestIdType(xRequestIDType) (gs_stUdsInfo.RequsetIdMode = (xRequestIDType))

/*restart s3server time*/
void RestartS3Server(void)
{
    gs_stUdsInfo.xUdsS3ServerTime = UdsAppTimeToCount(gs_stUdsAppCfg.xS3Server);
}

/*set currrent session mode. DEFAULT_SESSION/PROGRAM_SESSION/EXTEND_SESSION */
void SetCurrentSession(const uint8 i_SerSessionMode)
{
    gs_stUdsInfo.CurSessionMode = i_SerSessionMode;
}

/*********************************************************/
/**********************UDS service configuration and function************************/

/**********************UDS service correlation subfunction define************************/
/*app memcopy*/
static void AppMemcopy(const void *i_pvSource, const uint8 i_CopyLen, void *o_pvDest);

/*app memset*/
static void AppMemset(const uint8 i_SetValue, const uint16 i_Len, void *m_pvSource);

/*check routine control right?*/
static uint8 IsCheckRoutineControlRight(const tCheckRoutineCtlInfo i_eCheckRoutineCtlId,
                                        const tUdsAppMsgInfo *m_pstPDUMsg);

/*Is erase memory routine control?*/
static uint8 IsEraseMemoryRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg);

/*Is check sum routine control?*/
static uint8 IsCheckSumRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg);

/*Is check programming dependency?*/
static uint8 IsCheckProgrammingDependency(const tUdsAppMsgInfo *m_pstPDUMsg);

/*Is write fingerprint right?*/
static uint8 IsWriteFingerprintRight(const tUdsAppMsgInfo *m_pstPDUMsg);

/*Is download data address valid?*/
static uint8 IsDownloadDataAddrValid(const uint32 i_DataAddr);

/*Is dowload data len valid?*/
static uint8 IsDownloadDataLenValid(const uint32 i_DataLen);

/*check random is right?*/
static uint8 IsReceivedKeyRight(const uint8 *i_pReceivedKey,
                                const uint8 *i_pTxSeed,
                                const uint8 KeyLen);

/*do check sum. If check sum right return TRUE, else return FALSE.*/
static void DoCheckSum(uint8 i_TxStatus);

/*do erase flash*/
static void DoEraseFlash(uint8 i_TxStatus);

/*do check programming dependency*/
static uint8 DoCheckProgrammingDependency(void);

/*do response checksum*/
static void DoResponseChecksum(uint8 i_Status);

/*do erase flash response*/
static void DoEraseFlashResponse(uint8 i_Status);

/* When do uds service need more time, need call the funciton */
static void RequestMoreTime(const uint8 UDSServiceID, void (*pcallback)(uint8));

/*do reset mcu*/
static void DoResetMCU(uint8 i_Txstatus);

/******************************UDS service main function define***************************************/
/*dig session*/
static void DigSession(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*control DTC setting*/
static void ControlDTCSetting(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*communication control*/
static void CommunicationControl(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*security access*/
static void SecurityAccess(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*write data by identifier*/
static void WriteDataByIdentifier(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*request download*/
static void RequestDownload(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*transfer data*/
static void TransferData(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*request transfer exit*/
static void RequestTransferExit(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*routine control*/
static void RoutineControl(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*reset ECU*/
static void ResetECU(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/*Tester present service*/
static void TesterPresent(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

/***********************UDS service Static Global value************************/
/* XXX Bootloader: #00 UDS Service Configuration Table */
static const tUDSService gs_astUDSService[] =
{
    /*diagnose mode control*/
    {
        0x10u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        DigSession
    },

    /*communication control*/
    {
        0x28u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        CommunicationControl
    },

    /*control DTC setting*/
    {
        0x85u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        ControlDTCSetting
    },

    /*security access*/
    {
        0x27u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        NONE_SECURITY,
        SecurityAccess
    },

    /*write data by identifier*/
    {
        0x2Eu,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        WriteDataByIdentifier
    },

    /*request download data */
    {
        0x34u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        RequestDownload
    },

    /*transter data*/
    {
        0x36u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        TransferData
    },

    /*request exit transfer data*/
    {
        0x37u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        RequestTransferExit
    },

    /*routine control*/
    {
        0x31u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        RoutineControl
    },

    /*reset ECU*/
    {
        0x11u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        SECURITY_LEVEL_1,
        ResetECU
    },

    /*diagnose mode control*/
    {
        0x3Eu,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        TesterPresent
    },
};

/*uds servie sub function config table*/
/*erase memory routine cotnrol ID*/
static const uint8 gs_aEraseMemoryRoutineControlId[] = {0x31u, 0x01u, 0xFFu, 0x00u};

/*check sum routine control ID*/
static const uint8 gs_aCheckSumRoutineControlId[] = {0x31u, 0x01u, 0x02u, 0x02u};

/*check programming dependency*/
static const uint8 gs_aCheckProgrammingDependencyId[] = {0x31u, 0x01u, 0xFFu, 0x01u};

/*write fingerprint id*/
static const uint8 gs_aWriteFingerprintId[] = {0x2Eu, 0xF1u, 0x5Au};

/**********************UDS service correlation main function realizing************************/
/*dig session*/
static void DigSession(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 RequestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    /*set send postive message*/
    m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
    m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
    m_pstPDUMsg->xDataLen = 2u;

    /*sub function*/
    switch(RequestSubfunction)
    {
    case 0x01u :  /*default mode*/
    case 0x81u :
        SetCurrentSession(DEFALUT_SESSION);

        if(0x81u == RequestSubfunction)
        {
            m_pstPDUMsg->xDataLen = 0u;
        }

        break;

    case 0x02u :  /*program mode*/
    case 0x82u :
        SetCurrentSession(PROGRAM_SESSION);

        if(0x82u == RequestSubfunction)
        {
            m_pstPDUMsg->xDataLen = 0u;
        }

        /*restart s3server time*/
        RestartS3Server();

        break;

    case 0x03u :  /*extend mode*/
    case 0x83u :
        SetCurrentSession(EXTEND_SESSION);

        if(0x83u == RequestSubfunction)
        {
            m_pstPDUMsg->xDataLen = 0u;
        }

        /*restart s3server time*/
        RestartS3Server();
        break;

    default :
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);
        break;
    }
}

/*control DTC setting*/
static void ControlDTCSetting(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 RequestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch(RequestSubfunction)
    {
    case 0x01u :
    case 0x02u :
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
        m_pstPDUMsg->xDataLen = 2u;
        break;

    case 0x81u :
    case 0x82u :
        m_pstPDUMsg->xDataLen = 0u;
        break;

    default :
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);
        break;
    }
}

/*communication control*/
static void CommunicationControl(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 RequestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch(RequestSubfunction)
    {
    case 0x0u :
    case 0x03u :
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
        m_pstPDUMsg->xDataLen = 2u;

        break;

    case 0x80u :
    case 0x83u :
        /*don't transmit uds message.*/
        m_pstPDUMsg->aDataBuf[0u] = 0u;
        m_pstPDUMsg->xDataLen = 0u;

        break;

    default :
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);

        break;
    }
}

/*security access*/
static void SecurityAccess(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 RequestSubfunction = 0u;
    static uint8 s_aSeedBuf[AES_SEED_LEN] = {0u};
    boolean ret = FALSE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*get subfunction*/
    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch(RequestSubfunction)
    {
    case 0x01u :
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;

        /*get random and put in m_pstPDUMsg->aDataBuf[2u] ~ 17u byte*/
        ret = UDS_ALG_HAL_GetRandom(AES_SEED_LEN, s_aSeedBuf);

        if(TRUE == ret)
        {
            AppMemcopy(s_aSeedBuf, AES_SEED_LEN, &m_pstPDUMsg->aDataBuf[2u]);
            m_pstPDUMsg->xDataLen = 2u + AES_SEED_LEN;
        }
        else
        {
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, IK, m_pstPDUMsg);
        }

        break;

    case 0x02u :
        /*count random to key and check received key right?*/
        if(TRUE == IsReceivedKeyRight(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, AES_SEED_LEN))
        {
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;

            m_pstPDUMsg->xDataLen = 2u;

            AppMemset(0x1u, sizeof(s_aSeedBuf), s_aSeedBuf);

            SetSecurityLevel(SECURITY_LEVEL_1);
        }
        else
        {
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, IK, m_pstPDUMsg);
        }

        break;

    default :

        break;
    }
}

/*write data by identifier*/
static void WriteDataByIdentifier(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*Is write fingerprint id right?*/
    if(TRUE== IsWriteFingerprintRight(m_pstPDUMsg))//
    {
        /*do write fingerprint*/
        Flash_SavePrintfigner(&m_pstPDUMsg->aDataBuf[3u], (m_pstPDUMsg->xDataLen - 3u));

        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = 0xF1u;
        m_pstPDUMsg->aDataBuf[2u] = 0x5Au;
        m_pstPDUMsg->xDataLen = 3u;
    }
    else
    {
        /*don't have this routine control ID*/
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);
    }
}

/*download data info*/
static tDowloadDataInfo gs_stDowloadDataInfo = {0u, 0u};

/* received block number */
static uint8 gs_RxBlockNum = 0u;

/*request download*/
static void RequestDownload(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Index = 0u;
    uint8 Ret = TRUE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    if(m_pstPDUMsg->xDataLen < (DOWLOAD_DATA_ADDR_LEN + DOWLOAD_DATA_LEN + 1u + 2u))
    {
        Ret = FALSE;

        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, IMLOIF, m_pstPDUMsg);
    }

    if(TRUE == Ret)
    {
        /*get data addr */
        gs_stDowloadDataInfo.StartAddr = 0u;
        for(Index = 0u; Index < DOWLOAD_DATA_ADDR_LEN; Index++)
        {
            gs_stDowloadDataInfo.StartAddr <<= 8u;
            /* 3u = N_PCI(1) + SID34(1) + dataFormatldentifier(1) */
            gs_stDowloadDataInfo.StartAddr |= m_pstPDUMsg->aDataBuf[Index + 3u];
        }

        /*get data len*/
        gs_stDowloadDataInfo.DataLen = 0u;
        for(Index = 0u; Index < DOWLOAD_DATA_LEN; Index++)
        {
            gs_stDowloadDataInfo.DataLen <<= 8u;
            gs_stDowloadDataInfo.DataLen |= m_pstPDUMsg->aDataBuf[Index + 7u];
        }
    }

    /*Is download data  addr  and len valid?*/
    if(((TRUE != IsDownloadDataAddrValid(gs_stDowloadDataInfo.StartAddr)) ||
            (TRUE != IsDownloadDataLenValid(gs_stDowloadDataInfo.DataLen))) && (TRUE == Ret))
    {
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, ROOR, m_pstPDUMsg);

        Ret = FALSE;
    }

    if(TRUE == Ret)
    {
        /*set wait transfer data step(0x34 service)*/
        Flash_SetNextDownloadStep(FL_TRANSFER_STEP);

        /*save received program addr and data len*/
        Flash_SaveDownloadDataInfo(gs_stDowloadDataInfo.StartAddr, gs_stDowloadDataInfo.DataLen);

        /*fill postive message*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = 0x10u;
        m_pstPDUMsg->aDataBuf[2u] = 0x82u;
        m_pstPDUMsg->xDataLen = 3u;

        /*set wait received block number*/
        gs_RxBlockNum = 1u;
    }
    else
    {
        FLSDebugPrintf("\n 4\n");
        Flash_InitDowloadInfo();

        /*set request transfer data step(0x34 service)*/
        Flash_SetNextDownloadStep(FL_REQUEST_STEP);
    }
}

/*transfer data*/
static void TransferData(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Ret = TRUE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*request sequence erro*/
    if((FL_TRANSFER_STEP != Flash_GetCurDownloadStep()) && (TRUE == Ret))
    {
        Ret = FALSE;

        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, RSE, m_pstPDUMsg);
    }

    if((gs_RxBlockNum != m_pstPDUMsg->aDataBuf[1u]) && (TRUE == Ret))
    {
        Ret = FALSE;

        /*received data is not wait block number*/
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, RSE, m_pstPDUMsg);
    }

    gs_RxBlockNum++;

    /*copy flash data in flash area*/
    if((TRUE != Flash_ProgramRegion(gs_stDowloadDataInfo.StartAddr,
                                    &m_pstPDUMsg->aDataBuf[2u],
                                    (m_pstPDUMsg->xDataLen - 2u))) && (TRUE == Ret))
    {
        Ret = FALSE;

        /*saved data and information failled!*/
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, CNC, m_pstPDUMsg);
    }
    else
    {
        gs_stDowloadDataInfo.StartAddr += (m_pstPDUMsg->xDataLen - 2u);
        gs_stDowloadDataInfo.DataLen -= (m_pstPDUMsg->xDataLen - 2u);
    }

    /*received all data*/
    if((0u == gs_stDowloadDataInfo.DataLen) && (TRUE == Ret))
    {
        gs_RxBlockNum = 0u;

        /*set wait exit transfer step(0x37 service)*/
        Flash_SetNextDownloadStep(FL_EXIT_TRANSFER_STEP);
    }

    if(TRUE == Ret)
    {
        /*tranmitted postive message.*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->xDataLen = 4u;
    }
    else
    {
        Flash_InitDowloadInfo();

        /*set request transfer data step(0x34 service)*/
        Flash_SetNextDownloadStep(FL_REQUEST_STEP);

        gs_RxBlockNum = 0u;
    }
}

/*request transfer exit*/
static void RequestTransferExit(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Ret = TRUE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    if(FL_EXIT_TRANSFER_STEP != Flash_GetCurDownloadStep())
    {
        Ret = FALSE;

        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, RSE, m_pstPDUMsg);
    }

    if(TRUE == Ret)
    {
        Flash_SetNextDownloadStep(FL_CHECKSUM_STEP);

        /*tranmitted postive message.*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->xDataLen = 1u;
    }
    else
    {
        Flash_InitDowloadInfo();
    }
}

/*routine control*/
static void RoutineControl(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Ret = FALSE;
    uint32 ReceivedCrc = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    RestartS3Server();

    /*Is erase memory routine control?*/
    if(TRUE == IsEraseMemoryRoutineControl(m_pstPDUMsg))
    {
        /*request client timeout time*/
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, RCRRP, m_pstPDUMsg);

        m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoEraseFlash;
    }
    /*Is check sum routine control?*/
    else if(TRUE == IsCheckSumRoutineControl(m_pstPDUMsg))
    {
        ReceivedCrc = m_pstPDUMsg->aDataBuf[4u];
        ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[5u];
        ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[6u];
        ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[7u];
        Flash_SavedReceivedCheckSumCrc(ReceivedCrc);

        /*request client timeout time*/
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, RCRRP, m_pstPDUMsg);

        m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoCheckSum;
    }

    /*Is check programming dependency?*/
    else if(TRUE == IsCheckProgrammingDependency(m_pstPDUMsg))
    {
        /*write application information in flash.*/
        (void)Flash_WriteFlashAppInfo();

        /*do check programming dependency*/
        Ret = DoCheckProgrammingDependency();
        if(TRUE == Ret)
        {
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
            m_pstPDUMsg->xDataLen = 4u;
        }
        else
        {
            /*don't have this routine control ID*/
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);
        }
    }
    else
    {
        /*don't have this routine control ID*/
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);
    }
}

/*reset ECU*/
static void ResetECU(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*If program data in flash successfull, set Bootloader will jump to application flag*/
    Flash_EraseFlashDriverInRAM();

    /*If invalid application software in flash, then this step set application jump to bootloader flag*/
    SetDownloadAppSuccessful();

    m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoResetMCU;

    /*request client timeout time*/
    SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, RCRRP, m_pstPDUMsg);
}

/*Tester present service*/
static void TesterPresent(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 RequestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    /*sub function*/
    switch(RequestSubfunction)
    {
    case 0x00u :  /*zero subFunction*/
        /*set send postive message*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
        m_pstPDUMsg->xDataLen = 2u;
        break;

    case 0x80u :  /*program mode*/
        m_pstPDUMsg->xDataLen = 0u;
        break;

    default :
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, SFNS, m_pstPDUMsg);
        break;
    }
}

/*do reset mcu*/
static void DoResetMCU(uint8 Txstatus)
{
    if(TX_MSG_SUCCESSFUL == Txstatus)
    {
        /*reset ECU*/
        WATCHDOG_HAL_SystemRest();
        while(1)
        {
            /*wait watch dog reset mcu*/
        }
    }
}

/**********************UDS service correlation other function realizing************************/
/* get uds service config information */
tUDSService* GetUDSServiceInfo(uint8 *m_pSupServItem)
{
    ASSERT(NULL_PTR == m_pSupServItem);

    *m_pSupServItem = sizeof(gs_astUDSService) / sizeof(gs_astUDSService[0u]);

    return (tUDSService*) &gs_astUDSService[0u];
}

/* If Rx UDS msg, set UDS layer received message TURE */
void SetIsRxUdsMsg(const uint8 i_SetValue)
{
#ifdef EN_DELAY_TIME
    if(i_SetValue)
    {
        gs_stJumpAPPDelayTimeInfo.isReceiveUDSMsg = TRUE;
    }
    else
    {
        gs_stJumpAPPDelayTimeInfo.isReceiveUDSMsg = FALSE;
    }
#endif
}

uint8 IsRxUdsMsg(void)
{
#ifdef EN_DELAY_TIME
    return gs_stJumpAPPDelayTimeInfo.isReceiveUDSMsg;
#else
    return TRUE;
#endif
}

/*set negative erro code*/
void SetNegativeErroCode(const uint8 i_UDSServiceNum,
                         const uint8 i_ErroCode,
                         tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);

    m_pstPDUMsg->aDataBuf[0u] = NEGTIVE_RESPONSE_ID;
    m_pstPDUMsg->aDataBuf[1u] = i_UDSServiceNum;
    m_pstPDUMsg->aDataBuf[2u] = i_ErroCode;
    m_pstPDUMsg->xDataLen = 3u;
}

/*Is current session DEFAULT return TRUE, else return FALSE.*/
uint8 IsCurDefaultSession(void)
{
    uint8 isCurDefaultSessionStatus = FALSE;

    if(DEFALUT_SESSION == gs_stUdsInfo.CurSessionMode)
    {
        isCurDefaultSessionStatus = TRUE;
    }
    else
    {
        isCurDefaultSessionStatus = FALSE;
    }

    return isCurDefaultSessionStatus;
}

/*Is S3server timeout?*/
uint8 IsS3ServerTimeout(void)
{
    uint8 TimeoutStatus = FALSE;

    if(0u == gs_stUdsInfo.xUdsS3ServerTime)
    {
        TimeoutStatus = TRUE;
    }
    else
    {
        TimeoutStatus = FALSE;
    }

    return TimeoutStatus;
}

/*Is current session can request?*/
uint8 IsCurSeesionCanRequest(uint8 i_SerSessionMode)
{
    uint8 status = FALSE;

    if((i_SerSessionMode & gs_stUdsInfo.CurSessionMode) == gs_stUdsInfo.CurSessionMode)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*save received request id. If receved physical/function/none phy and function ID set rceived physicali/function/erro ID.*/
void SaveRequestIdType(const uint32 i_SerRequestID)
{
    if(i_SerRequestID == TP_GetConfigRxMsgPHYID())
    {
        SetRequestIdType(SUPPORT_PHYSICAL_ADDR);
    }
    else if(i_SerRequestID == TP_GetConfigRxMsgFUNID())
    {
        SetRequestIdType(SUPPORT_FUNCTION_ADDR);
    }
    else
    {
        SetRequestIdType(ERRO_REQUEST_ID);
    }
}

/*Is current received id can request?*/
uint8 IsCurRxIdCanRequest(uint8 i_SerRequestIdMode)
{
    uint8 status = 0u;

    if((i_SerRequestIdMode & gs_stUdsInfo.RequsetIdMode) == gs_stUdsInfo.RequsetIdMode)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*set security level*/
void SetSecurityLevel(const uint8 i_SerSecurityLevel)
{
    gs_stUdsInfo.SecurityLevel = i_SerSecurityLevel;
}

/*Is current security level can request?*/
uint8 IsCurSecurityLevelRequet(uint8 i_SerSecurityLevel)
{
    uint8 status = 0u;

    if((i_SerSecurityLevel & gs_stUdsInfo.SecurityLevel) == gs_stUdsInfo.SecurityLevel)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/**********************UDS service correlation subfunction realizing************************/
/*app memcopy*/
static void AppMemcopy(const void *i_pvSource, const uint8 i_CopyLen, void *o_pvDest)
{
    ASSERT(NULL_PTR == i_pvSource);
    ASSERT(NULL_PTR == o_pvDest);

    fsl_memcpy(o_pvDest, i_pvSource, i_CopyLen);
}

/*app memset*/
static void AppMemset(const uint8 i_SetValue, const uint16 i_Len, void *m_pvSource)
{
    ASSERT(NULL_PTR == m_pvSource);

    fsl_memset(m_pvSource, i_SetValue, i_Len);
}

/*check random is right?*/
static uint8 IsReceivedKeyRight(const uint8 *i_pReceivedKey,
                                const uint8 *i_pTxSeed,
                                const uint8 KeyLen)
{
    uint8 index = 0u;
    uint8 aPlainText[AES_SEED_LEN] = {0u};

    ASSERT(NULL_PTR == i_pReceivedKey);
    ASSERT(NULL_PTR == i_pTxSeed);

    UDS_ALG_HAL_DecryptData(i_pReceivedKey, KeyLen, aPlainText);

    index = 0u;
    while(index < AES_SEED_LEN)
    {
        if(aPlainText[index] != i_pTxSeed[index])
        {
            return FALSE;
        }

        index++;
    }

    return TRUE;
}

/*check routine control right?*/
static uint8 IsCheckRoutineControlRight(const tCheckRoutineCtlInfo i_eCheckRoutineCtlId,
                                        const tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Index = 0u;
    uint8 FindCnt = 0u;
    uint8 *pDestRoutineCltId = NULL_PTR;

    ASSERT(NULL_PTR == m_pstPDUMsg);

    switch(i_eCheckRoutineCtlId)
    {
    case ERASE_MEMORY_ROUTINE_CONTROL :
        pDestRoutineCltId = (uint8 *)&gs_aEraseMemoryRoutineControlId[0u];

        FindCnt = sizeof(gs_aEraseMemoryRoutineControlId);

        break;

    case CHECK_SUM_ROUTINE_CONTROL :
        pDestRoutineCltId = (uint8 *)&gs_aCheckSumRoutineControlId[0u];

        FindCnt = sizeof(gs_aCheckSumRoutineControlId);

        break;

    case CHECK_DEPENDENCY_ROUTINE_CONTROL :
        pDestRoutineCltId = (uint8 *)&gs_aCheckProgrammingDependencyId[0u];

        FindCnt = sizeof(gs_aCheckProgrammingDependencyId);

        break;

    default :

        return FALSE;

        /*This is not have break*/
    }

    if((NULL_PTR == pDestRoutineCltId) || (m_pstPDUMsg->xDataLen < FindCnt))
    {
        return FALSE;
    }

    while(Index < FindCnt)
    {
        if(m_pstPDUMsg->aDataBuf[Index] != pDestRoutineCltId[Index])
        {
            return FALSE;
        }

        Index++;
    }

    return TRUE;
}

/*Is erase memory routine control?*/
static uint8 IsEraseMemoryRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);

    return IsCheckRoutineControlRight(ERASE_MEMORY_ROUTINE_CONTROL, m_pstPDUMsg);
}

/*Is check sum routine control?*/
static uint8 IsCheckSumRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);

    return IsCheckRoutineControlRight(CHECK_SUM_ROUTINE_CONTROL, m_pstPDUMsg);
}

/*Is check programming dependency?*/
static uint8 IsCheckProgrammingDependency(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);

    return IsCheckRoutineControlRight(CHECK_DEPENDENCY_ROUTINE_CONTROL, m_pstPDUMsg);
}

/*Is write fingerprint right?*/
#pragma GCC diagnostic ignored "-Wunused-function"
static uint8 IsWriteFingerprintRight(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Index = 0u;
    uint8 WriteFingerprintIdLen = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);

    WriteFingerprintIdLen = sizeof(gs_aWriteFingerprintId);

    if(m_pstPDUMsg->xDataLen < WriteFingerprintIdLen)
    {
        return FALSE;
    }

    while(Index < WriteFingerprintIdLen)
    {
        if(m_pstPDUMsg->aDataBuf[Index] != gs_aWriteFingerprintId[Index])
        {
            return FALSE;
        }

        Index++;
    }

    return TRUE;
}

/*Is download data address valid?*/
static uint8 IsDownloadDataAddrValid(const uint32 i_DataAddr)
{

    return TRUE;
}

/*Is dowload data len valid?*/
static uint8 IsDownloadDataLenValid(const uint32 i_DataLen)
{

    return TRUE;
}

typedef void (*tpfFlashOperateMoreTimecallback)(uint8);

/* For erasing or programming flash were timeout callback */
static tpfFlashOperateMoreTimecallback gs_pfFlashOperateMoreTimecallback = NULL_PTR;

static void RequestMoreTimeCallback(uint8 i_TxStatus)
{
    if(TX_MSG_SUCCESSFUL == i_TxStatus)
    {
        RestartS3Server();
    }

    if(NULL_PTR != gs_pfFlashOperateMoreTimecallback)
    {
        gs_pfFlashOperateMoreTimecallback(i_TxStatus);
        gs_pfFlashOperateMoreTimecallback = NULL_PTR;
    }
}

static void RequestMoreTime(const uint8 UDSServiceID, void (*pcallback)(uint8))
{
    tUdsAppMsgInfo stMsgBuf = {0};

    ASSERT(NULL_PTR == pcallback);

    stMsgBuf.xUdsId = TP_GetConfigTxMsgID();
    SetNegativeErroCode(UDSServiceID, RCRRP, &stMsgBuf);
    stMsgBuf.pfUDSTxMsgServiceCallBack = &RequestMoreTimeCallback;
    gs_pfFlashOperateMoreTimecallback = pcallback;

    (void)TP_WriteAFrameDataInTP(stMsgBuf.xUdsId, stMsgBuf.pfUDSTxMsgServiceCallBack,\
                                 stMsgBuf.xDataLen, stMsgBuf.aDataBuf);
}

/*do check sum. If check sum right return TRUE, else return FALSE.*/
static void DoCheckSum(uint8 TxStatus)
{
    if(TX_MSG_SUCCESSFUL == TxStatus)
    {
        /*need request client delay time for flash checking flash data*/
        Flash_SetOperateFlashActiveJob(FLASH_CHECKING, &DoResponseChecksum, 0x31u, &RequestMoreTime);
    }
}

/*do response checksum*/
static void DoResponseChecksum(uint8 i_Status)
{
    uint8 Index = 0u;
    uint8 aResponseBuf[8u] = {0u};
    uint8 TxDataLen = 0u;
    tUdsId UdsTxId = 0u;

    TxDataLen = sizeof(gs_aCheckSumRoutineControlId) / sizeof(gs_aCheckSumRoutineControlId[0u]);
    aResponseBuf[0u] = gs_aCheckSumRoutineControlId[0u] + 0x40u;

    for(Index = 0u; Index < TxDataLen - 1u; Index++)
    {
        aResponseBuf[Index + 1u] = gs_aCheckSumRoutineControlId[Index + 1u];
    }

    if(TRUE == i_Status)
    {
        aResponseBuf[TxDataLen] = 0u;
    }
    else
    {
        aResponseBuf[TxDataLen] = 1u;
    }

    TxDataLen++;

    UdsTxId = TP_GetConfigTxMsgID();

    (void)TP_WriteAFrameDataInTP(UdsTxId, NULL_PTR, TxDataLen, aResponseBuf);
}

/*do erase flash response*/
static void DoEraseFlashResponse(uint8 i_Status)
{
    uint8 Index = 0u;
    uint8 aResponseBuf[8u] = {0u};
    uint8 TxDataLen = 0u;
    tUdsId UdsTxId = 0u;

    TxDataLen = sizeof(gs_aEraseMemoryRoutineControlId) / sizeof(gs_aEraseMemoryRoutineControlId[0u]);
    aResponseBuf[0u] = gs_aEraseMemoryRoutineControlId[0u] + 0x40u;

    for(Index = 0u; Index < TxDataLen - 1u; Index++)
    {
        aResponseBuf[Index + 1u] = gs_aEraseMemoryRoutineControlId[Index + 1u];
    }

    if(TRUE == i_Status)
    {
        aResponseBuf[TxDataLen] = 0u;
    }
    else
    {
        aResponseBuf[TxDataLen] = 1u;
    }

    TxDataLen++;

    UdsTxId = TP_GetConfigTxMsgID();

    (void)TP_WriteAFrameDataInTP(UdsTxId, NULL_PTR, TxDataLen, aResponseBuf);
}

/*do erase flash*/
static void DoEraseFlash(uint8 TxStatus)
{
    if(TX_MSG_SUCCESSFUL == TxStatus)
    {
        /*do erase flash need request client delay timeout*/
        Flash_SetOperateFlashActiveJob(FLASH_ERASING, &DoEraseFlashResponse, 0x31, &RequestMoreTime);
    }
}

/*do check programming dependency*/
static uint8 DoCheckProgrammingDependency(void)
{
    uint8 ret = FALSE;

    if(TRUE == Flash_IsReadAppInfoFromFlashValid())
    {
        if(TRUE == Flash_IsAppInFlashValid())
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

/*********************************************************/
/**********************UDS service other module call function realizing************************/

/*transmitted confirm message callback*/
static void TXConfrimMsgCallback(uint8 i_status)
{
    if(TX_MSG_SUCCESSFUL == i_status)
    {
        SetCurrentSession(PROGRAM_SESSION);
        SetSecurityLevel(NONE_SECURITY);

        /*restart s3server time*/
        RestartS3Server();
    }
}

/*write message to host basd on UDS for request enter bootloader mode*/
boolean UDS_TxMsgToHost(void)
{
    tUdsAppMsgInfo stUdsAppMsg = {0u, 0u, {0u}, NULL_PTR};
    boolean ret = FALSE;

    stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();
    stUdsAppMsg.xDataLen = 2;
    stUdsAppMsg.aDataBuf[0u] = 0x50u;
    stUdsAppMsg.aDataBuf[1u] = 0x02u;
    stUdsAppMsg.pfUDSTxMsgServiceCallBack = TXConfrimMsgCallback;

    ret = TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId, stUdsAppMsg.pfUDSTxMsgServiceCallBack,
                                 stUdsAppMsg.xDataLen, stUdsAppMsg.aDataBuf);

    return ret;
}

/*uds time control*/
void UDS_SystemTickCtl(void)
{
    if(GetUdsS3ServerTime())
    {
        SubUdsS3ServerTime(1u);
    }

    if(GetUdsSecurityReqLockTime())
    {
        SubUdsSecurityReqLockTime(1u);
    }

#ifdef EN_DELAY_TIME
    if(TRUE != IsRxUdsMsg())
    {
        if(gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime)
        {
            gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime--;
        }
        else
        {
            /*max timeout time out*/
            Boot_JumpToAppOrNot();
        }
    }
#endif
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
