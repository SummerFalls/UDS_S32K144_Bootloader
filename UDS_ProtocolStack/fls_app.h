/*
 * @ 名称: fls_app.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef FLS_APP_H_
#define FLS_APP_H_

#include "flash_hal.h"
#include "includes.h"

/* Every program flash size */
#define PROGRAM_SIZE (128u)

/* Flash finger print length */
#define FL_FINGER_PRINT_LENGTH  (17u)

/* Invalid UDS services ID */
#define INVALID_UDS_SERVICES_ID (0xFFu)

typedef enum
{
    FLASH_IDLE,           /* Flash idle */
    FLASH_ERASING,        /* Erase flash */
    FLASH_PROGRAMMING,    /* Program flash */
    FLASH_CHECKING,       /* Check flash */
    FLASH_WAITING         /* Waiting transmitted message successful */
} tFlshJobModle;

typedef enum
{
    FLASH_OPERATE_ERRO,  /* Flash operate error */
    FLASH_OPERATE_RIGHT  /* Flash operate right */
} tFlashErroCode;

/* Flash loader download step */
typedef enum
{
    FL_REQUEST_STEP,       /* Flash request step */
    FL_TRANSFER_STEP,      /* Flash transfer data step */
    FL_EXIT_TRANSFER_STEP, /* Exit transfer data step */
    FL_CHECKSUM_STEP       /* Check sum step */
} tFlDownloadStepType;


/* input parameter : TRUE/FALSE. TRUE = operation successful, else failed. */
typedef void (*tpfResponse)(uint8);
typedef void (*tpfReuestMoreTime)(uint8, void (*)(uint8));

void FLASH_APP_Init(void);

void Flash_InitDowloadInfo(void);

void Flash_OperateMainFunction(void);

uint8 Flash_ProgramRegion(const uint32 i_addr,
                          const uint8 *i_pDataBuf,
                          const uint32 i_dataLen);

uint8 Flash_IsReadAppInfoFromFlashValid(void);

uint8 Flash_IsAppInFlashValid(void);

void Flash_SavedReceivedCheckSumCrc(uint32 i_receivedCrc);

void Flash_EraseFlashDriverInRAM(void);

void Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep);

tFlDownloadStepType Flash_GetCurDownloadStep(void);

boolean Flash_IsEqualDonwloadStep(const tFlDownloadStepType i_donwloadStep);

void Flash_SaveDownloadDataInfo(const uint32 i_dataStartAddr, const uint32 i_dataLen);

void Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob,
                                    const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8 i_requestUDSSerID,
                                    const tpfReuestMoreTime i_pfRequestMoreTimeCallback);

tFlshJobModle Flash_GetOperateFlashActiveJob(void);

void Flash_RegisterJobCallback(tpfResponse i_pfDoResponse);

void Flash_SaveFingerPrint(const uint8 *i_pFingerPrint, const uint8 i_FingerPrintLen);

uint8 Flash_WriteFlashAppInfo(void);

tAPPType Flash_GetNewestAPPType(void);

tAPPType Flash_GetOldAPPType(void);

uint32 Flash_GetResetHandlerAddr(void);

#endif /* FLS_APP_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
