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
    /*tx message call back*/
    void (*pfUDSTxMsgServiceCallBack)(uint8);
} tUdsAppMsgInfo;

typedef struct UDSServiceInfo
{
    uint8 SerNum;     /*service num. eg 0x3e/0x87...*/
    uint8 SessionMode;/*default session / program session / extend session*/
    uint8 SupReqMode; /*support physical / function addr*/
    uint8 ReqLevel;   /*request level.Lock/unlock*/
    void (*pfSerNameFun)(struct UDSServiceInfo*, tUdsAppMsgInfo *);
} tUDSService;

/*********************************************************/

/*S3 timer watermark time percent*/
#ifndef S3_TIMER_WATERMARK_PERCENT
#define S3_TIMER_WATERMARK_PERCENT (90u)
#endif

#if (S3_TIMER_WATERMARK_PERCENT <= 0) || (S3_TIMER_WATERMARK_PERCENT >= 100)
#error "S3_TIMER_WATERMARK_PERCENT should config (0, 100]"
#endif

/*uds negative value define*/
//enum __UDS_NRC_ENUM__
//{
//    NRC_GENERAL_REJECT                          = 0x10,
//    NRC_SERVICE_NOT_SUPPORTED                   = 0x11,
//    NRC_SUBFUNCTION_NOT_SUPPORTED               = 0x12,
//    NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT        = 0x13,
//    NRC_BUSY_REPEAT_REQUEST                     = 0x21,
//    NRC_CONDITIONS_NOT_CORRECT                  = 0x22,
//    NRC_REQUEST_SEQUENCE_ERROR                  = 0x24,
//    NRC_REQUEST_OUT_OF_RANGE                    = 0x31,
//    NRC_SECURITY_ACCESS_DENIED                  = 0x33,
//    NRC_INVALID_KEY                             = 0x35,
//    NRC_EXCEEDED_NUMBER_OF_ATTEMPTS             = 0x36,
//    NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED         = 0x37,
//    NRC_GENERAL_PROGRAMMING_FAILURE             = 0x72,
//    NRC_SERVICE_BUSY                            = 0x78,
//    NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F,
//};

enum __UDS_NRC__
{
    GR      = 0x10,
    SNS     = 0x11, /*service not support*/
    SFNS    = 0x12, /*subfunction not support*/
    IMLOIF  = 0x13, /*incorrect message length or invalid format*/
    BRR     = 0x21, /*busy repeat request*/
    CNC     = 0x22, /*conditions not correct*/
    RSE     = 0x24, /*request   sequence error*/
    ROOR    = 0x31, /*request out of range*/
    SAD     = 0x33, /*security access denied*/
    IK      = 0x35, /*invalid key*/
    ENOA    = 0x36, /*exceed number of attempts*/
    RTDNE   = 0x37,
    GPF     = 0x72,
    RCRRP   = 0x78, /*request correctly received-response pending*/
    SNSIAS  = 0x7F,
};

#define NEGTIVE_RESPONSE_ID (0x7Fu)

/*define session mode*/
#define DEFALUT_SESSION (1u << 0u)       /*default session*/
#define PROGRAM_SESSION (1u << 1u)       /*program session*/
#define EXTEND_SESSION (1u << 2u)        /*extend session*/

/*security request*/
#define NONE_SECURITY (1u << 0u)                          /*none security can request*/
#define SECURITY_LEVEL_1 ((1 << 1u) | NONE_SECURITY)      /*security level 1 request*/
#define SECURITY_LEVEL_2 ((1u << 2u) | SECURITY_LEVEL_1)  /*security level 2 request*/

/*********************************************************/
/*set currrent session mode. DEFAULT_SESSION/PROGRAM_SESSION/EXTEND_SESSION */
extern void SetCurrentSession(const uint8 i_SerSessionMode);

/*Is current session DEFAULT return TRUE, else return FALSE.*/
extern uint8 IsCurDefaultSession(void);

/*Is S3server timeout?*/
extern uint8 IsS3ServerTimeout(void);

/*restart s3server time*/
extern void RestartS3Server(void);

/*Is current session can request?*/
extern uint8 IsCurSeesionCanRequest(uint8 i_SerSessionMode);

/*save received request id. If receved physical/function/none phy
and function ID set rceived physicali/function/erro ID.*/
extern void SaveRequestIdType(const uint32 i_SerRequestID);

/*Is current received id can request?*/
extern uint8 IsCurRxIdCanRequest(uint8 i_SerRequestIdMode);

/*set security level*/
extern void SetSecurityLevel(const uint8 i_SerSecurityLevel);

/*Is current security level can request?*/
extern uint8 IsCurSecurityLevelRequet(uint8 i_SerSecurityLevel);

/* Get UDS config Service information */
tUDSService* GetUDSServiceInfo(uint8 *m_pSupServItem);

/* If Rx UDS msg, set g_ucIsRxUdsMsg TURE */
extern void SetIsRxUdsMsg(const uint8 i_SetValue);

extern uint8 IsRxUdsMsg(void);

/*set negative erro code*/
extern void SetNegativeErroCode(const uint8 i_UDSServiceNum,
                                const uint8 i_ErroCode,
                                tUdsAppMsgInfo *m_pstPDUMsg);

/*uds time control*/
extern void UDS_SystemTickCtl(void);

/*get UDS s3 watermark timer. return s3 * 5 * 1000/ 8*/
extern uint32 UDS_GetUDSS3WatermarkTimerMs(void);

/*write message to host basd on UDS for request enter bootloader mode*/
extern boolean UDS_TxMsgToHost(void);

#endif /* UDS_APP_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
