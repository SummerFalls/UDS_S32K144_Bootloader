/*
 * @ 名称: uds_app_cfg.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef UDS_APP_CFG_H_
#define UDS_APP_CFG_H_

#include "includes.h"
#include "TP.h"

typedef uint16 tUdsTime;

typedef struct
{
    tUdsId xUdsId;
    tUdsLen xDataLen;
    uint8 aDataBuf[150u];
    void (*pfUDSTxMsgServiceCallBack)(uint8); /* TX message callback */
} tUdsAppMsgInfo;

typedef struct UDSServiceInfo
{
    uint8 SerNum;      /* Service ID eg 0x3E/0x87... */
    uint8 SessionMode; /* Default session / program session / extend session */
    uint8 SupReqMode;  /* Support physical / function addr */
    uint8 ReqLevel;    /* Request level. Lock / unlock */
    void (*pfSerNameFun)(struct UDSServiceInfo *, tUdsAppMsgInfo *);
} tUDSService;

/* S3 timer water mark time percent */
#ifndef S3_TIMER_WATERMARK_PERCENT
#define S3_TIMER_WATERMARK_PERCENT (90u)
#endif

#if (S3_TIMER_WATERMARK_PERCENT <= 0) || (S3_TIMER_WATERMARK_PERCENT >= 100)
#error "S3_TIMER_WATERMARK_PERCENT should config (0, 100]"
#endif

/* UDS negative response code */
enum __UDS_NRC__
{
    NRC_GENERAL_REJECT                           = 0x10,
    NRC_SERVICE_NOT_SUPPORTED                    = 0x11,
    NRC_SUBFUNCTION_NOT_SUPPORTED                = 0x12,
    NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT         = 0x13,
    NRC_BUSY_REPEAT_REQUEST                      = 0x21,
    NRC_CONDITIONS_NOT_CORRECT                   = 0x22,
    NRC_REQUEST_SEQUENCE_ERROR                   = 0x24,
    NRC_REQUEST_OUT_OF_RANGE                     = 0x31,
    NRC_SECURITY_ACCESS_DENIED                   = 0x33,
    NRC_INVALID_KEY                              = 0x35,
    NRC_EXCEEDED_NUMBER_OF_ATTEMPTS              = 0x36,
    NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED          = 0x37,
    NRC_GENERAL_PROGRAMMING_FAILURE              = 0x72,
    NRC_SERVICE_BUSY                             = 0x78, /* Request correctly received and response pending */
    NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION  = 0x7F,
};

#define NEGTIVE_RESPONSE_ID (0x7Fu)

/* Define session mode */
#define DEFALUT_SESSION (1u << 0u)       /* Default session */
#define PROGRAM_SESSION (1u << 1u)       /* Program session */
#define EXTEND_SESSION (1u << 2u)        /* Extend session */

/* Security request */
#define NONE_SECURITY (1u << 0u)                          /* None security can request */
#define SECURITY_LEVEL_1 ((1 << 1u) | NONE_SECURITY)      /* Security level 1 request */
#define SECURITY_LEVEL_2 ((1u << 2u) | SECURITY_LEVEL_1)  /* Security level 2 request */

typedef struct
{
    uint8 CalledPeriod;         /* called UDS period */
    /* Security request count. If over this security request count, locked server some time */
    uint8 SecurityRequestCnt;
    tUdsTime xLockTime;         /* Lock time */
    tUdsTime xS3Server;         /* S3 Server time */
} tUdsTimeInfo;

extern const tUdsTimeInfo gs_stUdsAppCfg;

/* UDS APP time to count */
#define UdsAppTimeToCount(xTime) ((xTime) / gs_stUdsAppCfg.CalledPeriod)

#ifdef UDS_PROJECT_FOR_BOOTLOADER
#ifdef EN_DELAY_TIME
typedef struct
{
    boolean isReceiveUDSMsg;
    uint32 jumpToAPPDelayTime;
} tJumpAppDelayTimeInfo;
#endif
#endif

void SetCurrentSession(const uint8 i_SerSessionMode);

uint8 IsCurDefaultSession(void);

uint8 IsS3ServerTimeout(void);

void RestartS3Server(void);

uint8 IsCurSeesionCanRequest(uint8 i_SerSessionMode);

void SaveRequestIdType(const uint32 i_SerRequestID);

uint8 IsCurRxIdCanRequest(uint8 i_SerRequestIdMode);

void SetSecurityLevel(const uint8 i_SerSecurityLevel);

uint8 IsCurSecurityLevelRequet(uint8 i_SerSecurityLevel);

tUDSService *GetUDSServiceInfo(uint8 *m_pSupServItem);

#ifdef UDS_PROJECT_FOR_BOOTLOADER
void SetIsRxUdsMsg(const boolean i_SetValue);

boolean IsRxUdsMsg(void);
#endif

void SetNegativeErroCode(const uint8 i_UDSServiceNum,
                         const uint8 i_ErroCode,
                         tUdsAppMsgInfo *m_pstPDUMsg);

void UDS_SystemTickCtl(void);

uint32 UDS_GetUDSS3WatermarkTimerMs(void);

boolean UDS_TxMsgToHost(void);

#endif /* UDS_APP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
