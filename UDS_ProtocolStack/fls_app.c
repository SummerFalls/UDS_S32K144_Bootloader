/*
 * @ 名称: fls_app.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "fls_app.h"
#include "CRC_hal.h"
#include "watchdog_hal.h"
#include "uds_app.h"


#define MAX_DATA_ITEM (16u)      /* Max data item */
#define MAX_DATA_BUF_LEN (128u)  /* Max data buffer len */

typedef struct
{
    uint32 startAddr;   /* Start addr */
    uint32 dataLen;     /* Data len */
} tFlashDataInfo;

typedef struct
{
    uint8 numberOfItem; /* Number of item */
    tFlashDataInfo astFlashDataItem[MAX_DATA_ITEM]; /* Item flash data */
    tCrc xReceivedCrc;
} tFlashDataCheckInfo;

typedef struct
{
    /* Flash programming successful? If programming successful, the value set TRUE, else set FALSE */
    uint8 isFlashProgramSuccessfull;

    /* Is erase flash successful? If erased flash successful, set the TRUE, else set the FALSE. */
    uint8 isFlashErasedSuccessfull;

    /* Is Flash struct data valid? If written set the value is TRUE, else set the valid FALSE */
    uint8 isFlashStructValid;

    /* Indicate APP Counter. Before download. */
    uint8 appCnt;

    /* Flag if finger print buffer */
    uint8 aFingerPrint[FL_FINGER_PRINT_LENGTH];

    /* Reset handler length */
    uint32 appStartAddrLen;

    /* APP Start address - reset handler */
    uint32 appStartAddr;

    /* Count CRC */
    uint32 crc;
} tAppFlashStatus;


typedef struct
{
    /* Flag if finger print has written */
    uint8 isFingerPrintWritten;

    /* Flag if flash driver has download */
    uint8 isFlashDrvDownloaded;

    /* Error code for flash active job */
    uint8 errorCode;

    /* Request active job UDS service ID */
    uint8 requestActiveJobUDSSerID;

    /* Storage program data buffer */
    uint8 aProgramDataBuff[MAX_FLASH_DATA_LEN];

    /* Current process start address */
    uint32 startAddr;

    /* Current process length */
    uint32 length;

    /* Receive data start address */
    uint32 receivedDataStartAddr;

    /* Received data length */
    uint32 receivedDataLength;

    /* Received CRC value */
    uint32 receivedCRC;

    /* Received program data length */
    uint32 receiveProgramDataLength;

    /* Flash loader download step */
    tFlDownloadStepType eDownloadStep;

    /* Current job status */
    tFlshJobModle eActiveJob;

    /* Active job finished callback */
    tpfResponse pfActiveJobFinshedCallBack;

    /* Request more time from host */
    tpfReuestMoreTime pfRequestMoreTime;

    /* Point APP flash status */
    tAppFlashStatus *pstAppFlashStatus;

    /* Operate flash API */
    tFlashOperateAPI stFlashOperateAPI;

} tFlsDownloadStateType;

/* Erase flash status control */
typedef enum
{
    START_ERASE_FLASH, /* Start erase flash */
    DO_ERASING_FLASH,  /* Do erase flash */
    END_ERASE_FLASH    /* End erase flash */
} tEraseFlashStep;

/* Is flash driver download? */
#define IsFlashDriverDownload() (gs_stFlashDownloadInfo.isFlashDrvDownloaded)
#define SetFlashDriverDowload() (gs_stFlashDownloadInfo.isFlashDrvDownloaded = TRUE)
#define SetFlashDriverNotDonwload() (gs_stFlashDownloadInfo.isFlashDrvDownloaded = FALSE)

/* Flash download info */
static tFlsDownloadStateType gs_stFlashDownloadInfo;

/* Erase flash status. */
static tEraseFlashStep gs_eEraseFlashStep = START_ERASE_FLASH;

/* Get current erase flash step */
#define GetCurEraseFlashStep() (gs_eEraseFlashStep)

/* Set erase flash status */
#define SetEraseFlashStep(eEraseStep) \
    do{\
        gs_eEraseFlashStep = (eEraseStep);\
    }while(0u)

/* Request time status define */
#define REQ_TIME_SUCCESSFUL (1u)

/* Request time failed */
#define REQ_TIME_FAILED (2u)

/* Is request time successful? */
static uint8 gs_reqTimeStatus = 0xFFu;

/* Application flash status */
static tAppFlashStatus gs_stAppFlashStatus;

/* Set request time status */
#define ClearRequestTimeStauts()\
    do{\
        gs_reqTimeStatus = 0xFFu;\
    }while(0u)

/* Request more time successful */
#define SetRequestMoreTimeStatus(status) \
    do{\
        gs_reqTimeStatus = status;\
    }while(0u)

/* Is request time successful? */
#define IsReqestTimeSuccessfull() ((1u == gs_reqTimeStatus) ? TRUE : FALSE)

#define IsRequestTimeFailed() (((2u == gs_reqTimeStatus)) ? TRUE : FALSE)

/* Application flash status */
#define SaveAppStatus(stAppStatus)\
    do{\
        gs_stAppFlashStatus = stAppStatus;\
    }while(0u)

#define IsFlashAppCrcEqualStorage(xCrc) ((gs_stAppFlashStatus.crc == xCrc) ? TRUE : FALSE)

#define SetFlashEreaseStatus(bIsFlashEraseSuccessful) \
    do{\
        gs_stAppFlashStatus.isFlashErasedSuccessfull = bIsFlashEraseSuccessful;\
    }while(0u)

#define SetFlashProgramStatus(bIsFlashProgramSuccessful) \
    do{\
        gs_stAppFlashStatus.isFlashProgramSuccessfull = bIsFlashProgramSuccessful;\
    }while(0u)


#define SetFlashStructStatus(bIsAppFlashStructValid) \
    do{\
        gs_stAppFlashStatus.isFlashStructValid = bIsAppFlashStructValid;\
    }while(0u)


#define SetAPPStatus(bIsFlashEraseSuccessful, bIsFlashProgramSuccessful, bIsAppFlashStructValid) \
    do{\
        SetFlashEreaseStatus(bIsFlashEraseSuccessful);\
        SetFlashProgramStatus(bIsFlashProgramSuccessful);\
        SetFlashStructStatus(bIsAppFlashStructValid);\
    }while(0u)

#define IsFlashEraseSuccessful() (gs_stAppFlashStatus.isFlashErasedSuccessfull)
#define IsFlashProgramSuccessful() (gs_stAppFlashStatus.isFlashProgramSuccessfull)
#define IsFlashStructValid() (gs_stAppFlashStatus.isFlashStructValid)

#define CreateAppStatusCrc(o_pCrc) CRC_HAL_CreatSoftwareCrc((uint8 *)&gs_stAppFlashStatus, sizeof(gs_stAppFlashStatus) - 4u, o_pCrc)
#define SaveAppStatusCrc(xCrc)\
    do{\
        gs_stAppFlashStatus.crc = xCrc;\
    }while(0u)

/* Create APP status CRC and save */
#define CreateAndSaveAppStatusCrc(o_pCrc) \
    do{\
        CreateAppStatusCrc(o_pCrc);\
        SaveAppStatusCrc(*o_pCrc);\
    }while(0u)

#define GetAppStatusPtr() (&gs_stAppFlashStatus)

#define SaveAppResetHandlerAddr(resetHandlerAddr, resetHnadlerAddrLen) \
    do{\
        gs_stAppFlashStatus.appStartAddr = resetHandlerAddr;\
        gs_stAppFlashStatus.appStartAddrLen = resetHnadlerAddrLen;\
    }while(0u)

/* Is flash driver software data? */
static boolean IsFlashDriverSoftwareData(void);

/* Flash write */
static uint8 FlashWrite(boolean *o_pbIsOperateFinsh);

/* Flash check sum */
static uint8 FlashChecksum(boolean *o_pbIsOperateFinsh);

/* Flash Erase */
static uint8 FlashErase(boolean *o_pbIsOperateFinsh);

/* Save flash data buffer */
static uint8 SavedFlashData(const uint8 *i_pDataBuf, const uint8 i_dataLen);

/* Read application information from flash */
static void ReadNewestAppInfoFromFlash(void);

/* Restore operate flash active job */
static void RestoreOperateFlashActiveJob(const tFlshJobModle i_activeJob);

/* Init flash download */
void Flash_InitDowloadInfo(void)
{
    gs_stFlashDownloadInfo.isFingerPrintWritten = FALSE;

    if (TRUE == IsFlashDriverDownload())
    {
#ifdef UDS_PROJECT_FOR_BOOTLOADER
        /* TODO : #00 由于链接文件地址空间分配的问题，APP 中不可执行本函数，否则会导致 HardFault */
        Flash_EraseFlashDriverInRAM();
#endif
        SetFlashDriverNotDonwload();
    }

    Flash_SetNextDownloadStep(FL_REQUEST_STEP);
    Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
    gs_stFlashDownloadInfo.pstAppFlashStatus = &gs_stAppFlashStatus;
    fsl_memset(&gs_stFlashDownloadInfo.stFlashOperateAPI, 0x0u, sizeof(tFlashOperateAPI));
    fsl_memset(&gs_stAppFlashStatus, 0xFFu, sizeof(tAppFlashStatus));
}

/* Flash APP module init */
void FLASH_APP_Init(void)
{
    gs_stFlashDownloadInfo.isFingerPrintWritten = FALSE;
#ifdef UDS_PROJECT_FOR_BOOTLOADER
    /* TODO : #00 由于链接文件地址空间分配的问题，APP 中不可执行本函数，否则会导致 HardFault */
    Flash_EraseFlashDriverInRAM();
#endif
    SetFlashDriverNotDonwload();
    Flash_SetNextDownloadStep(FL_REQUEST_STEP);
    Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
    gs_stFlashDownloadInfo.pstAppFlashStatus = &gs_stAppFlashStatus;
    fsl_memset(&gs_stFlashDownloadInfo.stFlashOperateAPI, 0x0u, sizeof(tFlashOperateAPI));
    fsl_memset(&gs_stAppFlashStatus, 0xFFu, sizeof(tAppFlashStatus));
}

/* Flash operate main function */
void Flash_OperateMainFunction(void)
{
    tFlshJobModle currentFlashJob = FLASH_IDLE;
    boolean bIsOperateFinshed = FALSE;
    currentFlashJob = Flash_GetOperateFlashActiveJob();

    switch (currentFlashJob)
    {
        case FLASH_ERASING:
            /* Do the flash erase */
            bIsOperateFinshed = FALSE;
            gs_stFlashDownloadInfo.errorCode = FlashErase(&bIsOperateFinshed);
            break;

        case FLASH_PROGRAMMING:
            bIsOperateFinshed = TRUE;
            /* Do the flash program */
            gs_stFlashDownloadInfo.errorCode = FlashWrite(&bIsOperateFinshed);
            break;

        case FLASH_CHECKING:
            /* Do the flash checksum */
            bIsOperateFinshed = TRUE;
            gs_stFlashDownloadInfo.errorCode = FlashChecksum(&bIsOperateFinshed);
            break;

        case FLASH_WAITING:
            if (TRUE == IsReqestTimeSuccessfull())
            {
                ClearRequestTimeStauts();
                RestoreOperateFlashActiveJob(FLASH_ERASING);
            }
            else if (TRUE == IsRequestTimeFailed())
            {
                ClearRequestTimeStauts();
                /* Set erase flash step to start! */
                SetEraseFlashStep(START_ERASE_FLASH);
                /* Initialize the flash download state */
                Flash_InitDowloadInfo();
                /* Set flash job is IDLE */
                Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
            }
            else
            {
                /* Do nothing */
            }

            break;

        default:
            break;
    }

    /* Just operate flash finished, can do callback and set next job. */
    if (TRUE == bIsOperateFinshed)
    {
        if ((NULL_PTR != gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack) && (FLASH_IDLE != currentFlashJob))
        {
            (gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack)(gs_stFlashDownloadInfo.errorCode);
            gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = NULL_PTR;
        }

        if ((gs_stFlashDownloadInfo.errorCode != TRUE) &&
                ((FLASH_ERASING == currentFlashJob) ||
                 (FLASH_PROGRAMMING == currentFlashJob) ||
                 (FLASH_CHECKING == currentFlashJob)))
        {
            /* Initialize the flash download state */
            Flash_InitDowloadInfo();
        }

        /* Set flash job is IDLE */
        Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
    }
}

/* Set next download step. */
void Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep)
{
    gs_stFlashDownloadInfo.eDownloadStep = i_donwloadStep;
}

/* Get current download step */
tFlDownloadStepType Flash_GetCurDownloadStep(void)
{
    return gs_stFlashDownloadInfo.eDownloadStep;
}

/* Judge download step */
boolean Flash_IsEqualDonwloadStep(const tFlDownloadStepType i_donwloadStep)
{
    boolean result = FALSE;

    if (i_donwloadStep == Flash_GetCurDownloadStep())
    {
        result =  TRUE;
    }

    return result;
}

/* Save download data information, the API called by UDS request download service */
void Flash_SaveDownloadDataInfo(const uint32 i_dataStartAddr, const uint32 i_dataLen)
{
    /* Program data info */
    gs_stFlashDownloadInfo.startAddr = i_dataStartAddr;
    gs_stFlashDownloadInfo.length = i_dataLen;
    /* Calculate data CRC info */
    gs_stFlashDownloadInfo.receivedDataStartAddr = i_dataStartAddr;
    gs_stFlashDownloadInfo.receivedDataLength = i_dataLen;
}

/* Set operate flash active job. */
void Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob,
                                    const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8 i_requestUDSSerID,
                                    const tpfReuestMoreTime i_pfRequestMoreTimeCallback)
{
    gs_stFlashDownloadInfo.eActiveJob = i_activeJob;
    gs_stFlashDownloadInfo.requestActiveJobUDSSerID = i_requestUDSSerID;
    gs_stFlashDownloadInfo.pfRequestMoreTime = i_pfRequestMoreTimeCallback;
    gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = i_pfActiveFinshedCallBack;
}

/* Restore operate flash active job */
static void RestoreOperateFlashActiveJob(const tFlshJobModle i_activeJob)
{
    gs_stFlashDownloadInfo.eActiveJob = i_activeJob;
}

/* Get operate flash active job */
tFlshJobModle Flash_GetOperateFlashActiveJob(void)
{
    return gs_stFlashDownloadInfo.eActiveJob;
}

/* Register flash job callback. Callback will called when after flash job. */
void Flash_RegisterJobCallback(tpfResponse i_pfDoResponse)
{
    gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = i_pfDoResponse;
}

/* Save flash data buffer */
static uint8 SavedFlashData(const uint8 *i_pDataBuf, const uint8 i_dataLen)
{
    ASSERT(NULL_PTR == i_pDataBuf);

    if (i_dataLen > MAX_FLASH_DATA_LEN)
    {
        return FALSE;
    }

    fsl_memcpy(gs_stFlashDownloadInfo.aProgramDataBuff, i_pDataBuf, i_dataLen);
    gs_stFlashDownloadInfo.receiveProgramDataLength = i_dataLen;
    return TRUE;
}

/* Request more time successful from host. i_txMsgStatus: 0 is successful, others is failed. */
static void RequetMoreTimeSuccessfulFromHost(uint8 i_txMsgStatus)
{
    if (0u == i_txMsgStatus)
    {
        /* Request time successful */
        SetRequestMoreTimeStatus(REQ_TIME_SUCCESSFUL);
    }
    else
    {
        /* TX message failed. */
        SetRequestMoreTimeStatus(REQ_TIME_FAILED);
    }
}

/* Flash Erase */
static uint8 FlashErase(boolean *o_pbIsOperateFinsh)
{
    static uint8 s_result = TRUE;
    uint32 eraseFlashLen = 0u;
    tCrc xCountCrc = 0u;
    static tAPPType s_appType = APP_INVLID_TYPE;
    static BlockInfo_t *s_pAppFlashMemoryInfo = NULL_PTR;
    static uint32 s_appFlashItem = 0u;
    uint32 sectorNo = 0u;
    const uint32 maxEraseSectors = UDS_GetUDSS3WatermarkTimerMs() / FLASH_HAL_GetEraseFlashASectorMaxTimeMs();
    uint32 totalSectors = 0u;
    uint32 canEraseMaxSectors = 0u;
    static uint32 s_eraseSectorsCnt = 0u;
    uint32 eraseFlashStartAddr = 0u;
    uint32 eraseSectorNoTmp = 0u;
    ASSERT(NULL_PTR == o_pbIsOperateFinsh);

    /* Check flash driver valid or not? */
    if (TRUE != IsFlashDriverDownload())
    {
        return FALSE;
    }

    *o_pbIsOperateFinsh = FALSE;

    switch (GetCurEraseFlashStep())
    {
        case START_ERASE_FLASH:
            /* Get old APP type */
            s_appType = Flash_GetOldAPPType();
            s_pAppFlashMemoryInfo = NULL_PTR;
            s_appFlashItem = 0u;
            s_eraseSectorsCnt = 0u;
            s_result = TRUE;

            /* Get old APP type flash config */
            if (TRUE == FLASH_HAL_GetFlashConfigInfo(s_appType, &s_pAppFlashMemoryInfo, &s_appFlashItem))
            {
                SetEraseFlashStep(DO_ERASING_FLASH);
            }

            break;

        case DO_ERASING_FLASH:
            /* Get total sectors */
            totalSectors = FLASH_HAL_GetTotalSectors(s_appType);

            /* One time erase all flash sectors */
            if (totalSectors <= maxEraseSectors)
            {
                while (s_appFlashItem)
                {
                    eraseFlashLen = s_pAppFlashMemoryInfo->xBlockEndLogicalAddr -
                                    s_pAppFlashMemoryInfo->xBlockStartLogicalAddr;
                    /* Feed watch dog */
                    WATCHDOG_HAL_Feed();
                    sectorNo = FLASH_HAL_GetFlashLengthToSectors(s_pAppFlashMemoryInfo->xBlockStartLogicalAddr, eraseFlashLen);

                    if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr)
                    {
                        /* Disable all interrupts */
                        DisableAllInterrupts();
                        eraseSectorNoTmp = sectorNo;
                        eraseFlashStartAddr = s_pAppFlashMemoryInfo->xBlockStartLogicalAddr;

                        /* Erase a sector once because for watch dog */
                        while (eraseSectorNoTmp)
                        {
                            /* Feed watch dog */
                            WATCHDOG_HAL_Feed();
                            /* Do erase flash */
                            s_result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(eraseFlashStartAddr, 1u);
                            eraseSectorNoTmp--;

                            if (TRUE != s_result)
                            {
                                break;
                            }

                            eraseFlashStartAddr += FLASH_HAL_Get1SectorBytes();
                        }

                        /* Enable all all interrupts */
                        EnableAllInterrupts();
                    }
                    else
                    {
                        s_result = FALSE;
                    }

                    if (TRUE != s_result)
                    {
                        break;
                    }

                    s_eraseSectorsCnt += sectorNo;
                    s_appFlashItem--;
                    s_pAppFlashMemoryInfo++;
                }
            }
            else
            {
                while ((s_eraseSectorsCnt < totalSectors) && (0u != s_appFlashItem))
                {
                    /* Feed watch dog */
                    WATCHDOG_HAL_Feed();

                    /* Get erase sector start address */
                    if (TRUE != FLASH_HAL_SectorNumberToFlashAddress(s_appType, s_eraseSectorsCnt, &eraseFlashStartAddr))
                    {
                        s_result = FALSE;
                        break;
                    }

                    /* Check erase sector indicate flash address is valid or not? */
                    if ((eraseFlashStartAddr >= s_pAppFlashMemoryInfo->xBlockStartLogicalAddr) &&
                            (eraseFlashStartAddr < s_pAppFlashMemoryInfo->xBlockEndLogicalAddr))
                    {
                        /* Calculate length */
                        eraseFlashLen = s_pAppFlashMemoryInfo->xBlockEndLogicalAddr -
                                        eraseFlashStartAddr;
                    }
                    else
                    {
                        s_result = FALSE;
                        break;
                    }

                    /* Save erase flash memory address */
                    eraseFlashStartAddr = s_pAppFlashMemoryInfo->xBlockStartLogicalAddr;
                    /* Calculate can erase max sectors */
                    canEraseMaxSectors = maxEraseSectors - (s_eraseSectorsCnt % maxEraseSectors);
                    /* Calculate flash length to sectors */
                    sectorNo = FLASH_HAL_GetFlashLengthToSectors(eraseFlashStartAddr, eraseFlashLen);

                    if (sectorNo > maxEraseSectors)
                    {
                        sectorNo = maxEraseSectors;

                        if (sectorNo > canEraseMaxSectors)
                        {
                            sectorNo = canEraseMaxSectors;
                        }
                    }
                    else
                    {
                        if (sectorNo <= canEraseMaxSectors)
                        {
                            s_appFlashItem--;
                            s_pAppFlashMemoryInfo++;
                        }
                        else
                        {
                            sectorNo = canEraseMaxSectors;
                        }
                    }

                    /* Erase flash memory */
                    if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr)
                    {
                        /* Disable all interrupts */
                        DisableAllInterrupts();
                        eraseSectorNoTmp = sectorNo;

                        /* Erase a sector once because for watch dog */
                        while (eraseSectorNoTmp)
                        {
                            /* Feed watch dog */
                            WATCHDOG_HAL_Feed();
                            /* Do erase flash */
                            s_result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(eraseFlashStartAddr, 1u);
                            eraseSectorNoTmp--;

                            if (TRUE != s_result)
                            {
                                break;
                            }

                            eraseFlashStartAddr += FLASH_HAL_Get1SectorBytes();
                        }

                        /* Enable all all interrupts */
                        EnableAllInterrupts();
                    }
                    else
                    {
                        s_result = FALSE;
                    }

                    if (TRUE != s_result)
                    {
                        break;
                    }

                    /* Add erased sectors count */
                    s_eraseSectorsCnt += sectorNo;

                    /* If erase max Erase sectors and have some sectors wait to erase, then request time from host */
                    if ((0u == (s_eraseSectorsCnt % maxEraseSectors)) && (s_eraseSectorsCnt < totalSectors))
                    {
                        *o_pbIsOperateFinsh = FALSE;
                        break;
                    }
                }
            }

            if ((FALSE == *o_pbIsOperateFinsh) && (TRUE == s_result) && (s_eraseSectorsCnt < totalSectors))
            {
                RestoreOperateFlashActiveJob(FLASH_WAITING);

                /* Request more time from host */
                if (NULL_PTR != gs_stFlashDownloadInfo.pfRequestMoreTime)
                {
                    gs_stFlashDownloadInfo.pfRequestMoreTime(gs_stFlashDownloadInfo.requestActiveJobUDSSerID, RequetMoreTimeSuccessfulFromHost);
                }
            }
            else
            {
                if ((TRUE == s_result) && (s_eraseSectorsCnt == totalSectors))
                {
                    SetAPPStatus(TRUE, FALSE, TRUE);
                }
                else
                {
                    SetAPPStatus(FALSE, FALSE, TRUE);
                }

                CreateAndSaveAppStatusCrc(&xCountCrc);
                s_eraseSectorsCnt = 0u;
                SetEraseFlashStep(END_ERASE_FLASH);
            }

            break;

        case END_ERASE_FLASH:
            WATCHDOG_HAL_Feed();
            s_pAppFlashMemoryInfo = NULL_PTR;
            s_appFlashItem = 0u;
            s_eraseSectorsCnt = 0u;
            *o_pbIsOperateFinsh = TRUE;
            SetEraseFlashStep(START_ERASE_FLASH);
            break;

        default:
            SetEraseFlashStep(START_ERASE_FLASH);
            break;
    }

    return s_result;
}

/* Flash write */
static uint8 FlashWrite(boolean *o_pbIsOperateFinsh)
{
    uint8 result = FALSE;
    uint32 countCrc = 0u;
    uint8 flashDataIndex = 0u;
    uint8 fillCnt = 0u;

    /* Check flash driver valid or not? */
    if (TRUE != IsFlashDriverDownload())
    {
        return FALSE;
    }

    result = TRUE;

    while (gs_stFlashDownloadInfo.receiveProgramDataLength >= PROGRAM_SIZE)
    {
        /* Count application flash CRC */
        CreateAppStatusCrc(&countCrc);

        if (TRUE != IsFlashAppCrcEqualStorage(countCrc))
        {
            /* CRC not right */
            result = FALSE;
            break;
        }

        if ((TRUE == IsFlashEraseSuccessful()) &&
                (TRUE == IsFlashStructValid()))
        {
            WATCHDOG_HAL_Feed();

            /* Write data in flash */
            if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData)
            {
                DisableAllInterrupts();
                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(gs_stFlashDownloadInfo.startAddr,
                         &gs_stFlashDownloadInfo.aProgramDataBuff[flashDataIndex * PROGRAM_SIZE],
                         PROGRAM_SIZE);
                EnableAllInterrupts();
            }
            else
            {
                result = FALSE;
            }

            if (TRUE == result)
            {
                gs_stFlashDownloadInfo.length -= PROGRAM_SIZE;
                gs_stFlashDownloadInfo.receiveProgramDataLength -= PROGRAM_SIZE;
                gs_stFlashDownloadInfo.startAddr += PROGRAM_SIZE;
                flashDataIndex++;
            }
            else
            {
                result = FALSE;
                break;
            }
        }
        else
        {
            result = FALSE;
            break;
        }
    }

    /* Calculate if program data is align < 8 bytes, need to fill 0xFF to align 8 bytes. */
    if ((0u != gs_stFlashDownloadInfo.receiveProgramDataLength) && (TRUE == result))
    {
        fillCnt = (uint8)(gs_stFlashDownloadInfo.receiveProgramDataLength & 0x07u);
        fillCnt = (~fillCnt + 1u) & 0x07u;
        fsl_memset((void *)&gs_stFlashDownloadInfo.aProgramDataBuff[flashDataIndex * PROGRAM_SIZE + gs_stFlashDownloadInfo.receiveProgramDataLength],
                   0xFFu,
                   fillCnt);
        gs_stFlashDownloadInfo.receiveProgramDataLength += fillCnt;

        /* Write data in flash */
        if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData)
        {
            DisableAllInterrupts();
            result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(gs_stFlashDownloadInfo.startAddr,
                     &gs_stFlashDownloadInfo.aProgramDataBuff[flashDataIndex * PROGRAM_SIZE],
                     gs_stFlashDownloadInfo.receiveProgramDataLength);
            EnableAllInterrupts();
        }
        else
        {
            result = FALSE;
        }

        if (TRUE == result)
        {
            gs_stFlashDownloadInfo.length -= (gs_stFlashDownloadInfo.receiveProgramDataLength - fillCnt);
            gs_stFlashDownloadInfo.startAddr += gs_stFlashDownloadInfo.receiveProgramDataLength;
            gs_stFlashDownloadInfo.receiveProgramDataLength = 0;
            flashDataIndex++;
        }
    }

    if (TRUE == result)
    {
        SetFlashProgramStatus(TRUE);
        CreateAndSaveAppStatusCrc(&countCrc);
        return TRUE;
    }

    Flash_InitDowloadInfo();
    return FALSE;
}

/* Flash check sum */
static uint8 FlashChecksum(boolean *o_pbIsOperateFinsh)
{
    tCrc xCountCrc = 0u;
    WATCHDOG_HAL_Feed();

    /* Reserved the if and else for external flash memory, like external flash need to flash driver read or write. */
    if ((TRUE == IsFlashDriverSoftwareData()) )
    {
        CRC_HAL_CreatHardwareCrc((const uint8 *)gs_stFlashDownloadInfo.receivedDataStartAddr, gs_stFlashDownloadInfo.receivedDataLength, &xCountCrc);
    }
    /* Only flash driver is download, can do erase/write flash */
    else if (TRUE == IsFlashDriverDownload())
    {
        CRC_HAL_CreatHardwareCrc((const uint8 *)gs_stFlashDownloadInfo.receivedDataStartAddr, gs_stFlashDownloadInfo.receivedDataLength, &xCountCrc);
        gs_stFlashDownloadInfo.receivedDataStartAddr += gs_stFlashDownloadInfo.receivedDataLength;
        gs_stFlashDownloadInfo.receivedDataLength = 0u;
    }
    else
    {
        /* Do nothing */
    }

    /* Feed watch dog */
    WATCHDOG_HAL_Feed();
#ifdef DebugBootloader_NOTCRC
    if (1)
#else
    if (gs_stFlashDownloadInfo.receivedCRC == xCountCrc)
#endif
    {
        if ((TRUE == IsFlashDriverSoftwareData()))
        {
            SetFlashDriverDowload();

            if (TRUE != FLASH_HAL_RegisterFlashAPI(&gs_stFlashDownloadInfo.stFlashOperateAPI))
            {
                SetFlashDriverNotDonwload();
            }
            else
            {
                if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfFlashInit)
                {
                    gs_stFlashDownloadInfo.stFlashOperateAPI.pfFlashInit();
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

/* Save received check sum CRC */
void Flash_SavedReceivedCheckSumCrc(uint32 i_receivedCrc)
{
    gs_stFlashDownloadInfo.receivedCRC = (tCrc)i_receivedCrc;
}

/* Flash program region. Called by UDS service 0x36u */
uint8 Flash_ProgramRegion(const uint32 i_addr,
                          const uint8 *i_pDataBuf,
                          const uint32 i_dataLen)
{
    uint8 dataLen = (uint8)i_dataLen;
    uint8 result = TRUE;
    ASSERT(NULL_PTR == i_pDataBuf);
    result = TRUE;

    if (FL_TRANSFER_STEP != Flash_GetCurDownloadStep())
    {
        result = FALSE;
    }

    /* Saved flash data */
    if (TRUE != SavedFlashData(i_pDataBuf, dataLen))
    {
        result = FALSE;
    }

    if (TRUE == result)
    {
        if ((FALSE == IsFlashDriverDownload()) || (TRUE == IsFlashDriverSoftwareData()))
        {
            /* If flash driver, copy the data to RAM */
            if (TRUE == IsFlashDriverSoftwareData())
            {
                fsl_memcpy((void *)i_addr, (void *)i_pDataBuf, dataLen);
            }

            Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
        }
        else
        {
            Flash_SetOperateFlashActiveJob(FLASH_PROGRAMMING, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
            gs_stFlashDownloadInfo.errorCode = TRUE;
        }
    }

    /* Received error data. */
    if (TRUE != result)
    {
        Flash_InitDowloadInfo();
    }

    return result;
}

/* Get rest hander address */
uint32 Flash_GetResetHandlerAddr(void)
{
    return gs_stAppFlashStatus.appStartAddr;
}

/* Is flash driver software data? */
static boolean IsFlashDriverSoftwareData(void)
{
    uint32 flashDriverStartAddr = 0u;
    uint32 flashDriverEndAddr = 0u;
    boolean result = FALSE;
    result = FLASH_HAL_GetFlashDriverInfo(&flashDriverStartAddr, &flashDriverEndAddr);

    if ((gs_stFlashDownloadInfo.startAddr >= flashDriverStartAddr) &&
            ((gs_stFlashDownloadInfo.startAddr + gs_stFlashDownloadInfo.length) < flashDriverEndAddr) &&
            (TRUE == result))
    {
        result =  TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}

/* Read application information from flash */
static void ReadNewestAppInfoFromFlash(void)
{
    tAPPType newestAppType = APP_A_TYPE;
    uint32 appInfoStart = 0u;
    uint32 appInfoBlocksize = 0u;
    boolean result = FALSE;
    newestAppType = Flash_GetNewestAPPType();
    result = FLASH_HAL_GetAPPInfo(newestAppType, &appInfoStart, &appInfoBlocksize);

    if ((sizeof(tAppFlashStatus) <= appInfoBlocksize) && (TRUE == result))
    {
        SaveAppStatus(*(tAppFlashStatus *)appInfoStart);
    }
}

/* Is read application information from flash valid? */
uint8 Flash_IsReadAppInfoFromFlashValid(void)
{
    tCrc xCrc = 0u;
    /* Read application information from flash */
    ReadNewestAppInfoFromFlash();
    CreateAppStatusCrc(&xCrc);
//    APPDebugPrintf("\n0x%08X\t0x%08X\n", gs_stAppFlashStatus.crc, xCrc);
    return IsFlashAppCrcEqualStorage(xCrc);
}

/* Is application in flash valid? If valid return TRUE, else return FALSE. */
uint8 Flash_IsAppInFlashValid(void)
{
    if (((TRUE == IsFlashProgramSuccessful()) &&
            (TRUE == IsFlashEraseSuccessful())) &&
            (TRUE == IsFlashStructValid()))
    {
        return TRUE;
    }

    return FALSE;
}

/* Erase flash driver in RAM */
void Flash_EraseFlashDriverInRAM(void)
{
    uint32 flashDriverStartAddr = 0u;
    uint32 flashDriverEndAddr = 0u;
    boolean result = FALSE;
    result = FLASH_HAL_GetFlashDriverInfo(&flashDriverStartAddr, &flashDriverEndAddr);

    if (TRUE == result)
    {
        fsl_memset((void *)flashDriverStartAddr, 0x0u, flashDriverEndAddr - flashDriverStartAddr);
    }
}

/* Save FingerPrint */
void Flash_SaveFingerPrint(const uint8 *i_pFingerPrint, const uint8 i_FingerPrintLen)
{
    uint8 FingerPrintLen = 0u;
    tCrc crc = 0u;
    ASSERT(NULL_PTR == i_pFingerPrint);

    if (i_FingerPrintLen > FL_FINGER_PRINT_LENGTH)
    {
        FingerPrintLen = FL_FINGER_PRINT_LENGTH;
    }
    else
    {
        FingerPrintLen = (uint8)i_FingerPrintLen;
    }

    fsl_memcpy((void *) gs_stFlashDownloadInfo.pstAppFlashStatus->aFingerPrint, (const void *) i_pFingerPrint,
               FingerPrintLen);
    CreateAndSaveAppStatusCrc(&crc);
}

/* Write flash application information called by bootloader last step */
uint8 Flash_WriteFlashAppInfo(void)
{
    uint8 result = FALSE;
    tAPPType oldAppType = APP_A_TYPE;
    uint32 appInfoStartAddr = 0u;
    uint32 appInfoLen = 0u;
    uint32 crc = 0u;
    tAppFlashStatus *pAppStatusPtr = NULL_PTR;
    tAPPType newestAPPType = APP_A_TYPE;
    uint32 newestAPPInfoStartAddr = 0u;
    uint32 newestAPPInfoLen = 0u;
    tAppFlashStatus *pstNewestAPPFlashStatus = NULL_PTR;
    uint32 resetHandleAddr = 0u;
    boolean bIsEnableWriteResetHandlerInFlash = FALSE;
    uint32 resetHandlerOffset = 0u;
    uint32 resetHandlerLength = 0u;
    CreateAndSaveAppStatusCrc(&crc);
    oldAppType = Flash_GetOldAPPType();
    newestAPPType = Flash_GetNewestAPPType();
    result = FLASH_HAL_GetAPPInfo(oldAppType, &appInfoStartAddr, &appInfoLen);

    if (TRUE == result)
    {
        /* Write data information in flash */
        pAppStatusPtr = GetAppStatusPtr();
        FLASH_HAL_GetResetHandlerInfo(&bIsEnableWriteResetHandlerInFlash, &resetHandlerOffset, &resetHandlerLength);

        /* Update APP cnt */
        if (TRUE == FLASH_HAL_GetAPPInfo(newestAPPType, &newestAPPInfoStartAddr, &newestAPPInfoLen))
        {
            pstNewestAPPFlashStatus = (tAppFlashStatus *)newestAPPInfoStartAddr;
            pAppStatusPtr->appCnt = pstNewestAPPFlashStatus->appCnt + 1u;

            if (0xFFu == pAppStatusPtr->appCnt)
            {
                pAppStatusPtr->appCnt = 0u;
            }

            /* Get APP start address from flash. The address is the newest APP, because the APP info not write in flash, so the APP is old */
            resetHandleAddr = appInfoStartAddr + resetHandlerOffset;
            SaveAppResetHandlerAddr(*((uint32 *)resetHandleAddr), resetHandlerLength);
            FLSDebugPrintf("APP type =%X, APP address=0x%X\n", oldAppType, *((uint32 *)resetHandleAddr));
            crc = 0u;
            CreateAndSaveAppStatusCrc(&crc);
        }

        if ((NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData) && (NULL_PTR != pAppStatusPtr))
        {
            result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(appInfoStartAddr,
                     (uint8 *)pAppStatusPtr,
                     sizeof(tAppFlashStatus));
        }
        else
        {
            result = FALSE;
        }
    }

    return result;
}

#ifdef EN_SUPPORT_APP_B
static tAPPType DoCheckNewestAPPCnt(const tAppFlashStatus *i_pAppAInfo, const tAppFlashStatus *i_pAppBInfo)
{
    uint8 appACnt = 0u;
    uint8 appBCnt = 0u;
    uint8 deltaCnt = 0u;
    tAPPType newestAPP = APP_A_TYPE;
    ASSERT(NULL_PTR == i_pAppAInfo);
    ASSERT(NULL_PTR == i_pAppBInfo);
    appACnt = i_pAppAInfo->appCnt;
    appBCnt = i_pAppBInfo->appCnt;
    deltaCnt = (appACnt > appBCnt) ? (appACnt - appBCnt) : (appBCnt - appACnt);

    if (1u == deltaCnt)
    {
        if (appACnt > appBCnt)
        {
            newestAPP = APP_A_TYPE;
        }
        else
        {
            newestAPP = APP_B_TYPE;
        }
    }
    else if (0xFEu == deltaCnt)
    {
        if (appACnt < appBCnt)
        {
            newestAPP = APP_A_TYPE;
        }
        else
        {
            newestAPP = APP_B_TYPE;
        }
    }
    else
    {
        /* When cnt = 0xFF, then current cnt is invalid */
        if ((0xFFu == appACnt) && (0xFFu != appBCnt))
        {
            newestAPP = APP_B_TYPE;
        }
        else if ((0xFFu != appACnt) && (0xFFu == appBCnt))
        {
            newestAPP = APP_A_TYPE;
        }
        else if ((0xFFu == appACnt) && (0xFFu == appBCnt))
        {
            /* Invalid counter */
            newestAPP = APP_INVLID_TYPE;
        }
        else
        {
            newestAPP = APP_A_TYPE;
        }
    }

    return newestAPP;
}


static tAPPType DoCheckNewestAPPInfo(const tAppFlashStatus *i_pAppAInfo, const tAppFlashStatus *i_pAppBInfo)
{
#ifdef EN_SUPPORT_APP_B
    uint32 crc = 0u;
    boolean bIsAppAValid = FALSE;
    boolean bIsAppBValid = FALSE;
    tAPPType newestAPP = APP_A_TYPE;
#endif /* EN_SUPPORT_APP_B */
    ASSERT(NULL_PTR == i_pAppAInfo);
    ASSERT(NULL_PTR == i_pAppBInfo);
#ifndef EN_SUPPORT_APP_B
    return APP_A_TYPE;
#else /* EN_SUPPORT_APP_B */
    crc = 0u;
    CRC_HAL_CreatSoftwareCrc((const uint8 *)i_pAppAInfo, sizeof(tAppFlashStatus) - 4u, &crc);

    if (crc == i_pAppAInfo->crc)
    {
        bIsAppAValid = TRUE;
    }

    crc = 0u;
    CRC_HAL_CreatSoftwareCrc((const uint8 *)i_pAppBInfo, sizeof(tAppFlashStatus) - 4u, &crc);

    if (crc == i_pAppBInfo->crc)
    {
        bIsAppBValid = TRUE;
    }

    if ((TRUE == bIsAppAValid) && (TRUE != bIsAppBValid))
    {
        newestAPP = APP_A_TYPE;
    }
    else if ((TRUE != bIsAppAValid) && (TRUE == bIsAppBValid))
    {
        newestAPP = APP_B_TYPE;
    }
    else if ((TRUE != bIsAppAValid) && (TRUE != bIsAppBValid))
    {
        newestAPP = APP_A_TYPE;
    }
    else
    {
        /* Check APP A and B who is newest, both APP A & B is valid */
        newestAPP = DoCheckNewestAPPCnt(i_pAppAInfo, i_pAppBInfo);
    }

    return newestAPP;
#endif  /* EN_SUPPORT_APP_B */
}

#endif /* EN_SUPPORT_APP_B */

/* Get newest APP info */
tAPPType Flash_GetNewestAPPType(void)
{
#ifdef EN_SUPPORT_APP_B
    uint32 appInfoStartAddr_A = 0u;
    uint32 appInfoBlockSize_A = 0u;
    tAPPType newestAPP = APP_A_TYPE;
    tAppFlashStatus appAInfo;
    tAppFlashStatus appBInfo;
    uint32 appInfoStartAddr_B = 0u;
    uint32 appInfoBlockSize_B = 0u;
#endif /* EN_SUPPORT_APP_B */

    /* Not support APP B, so APP A is always newest. */
#ifndef EN_SUPPORT_APP_B
    return APP_A_TYPE;
#else
    FLASH_HAL_GetAPPInfo(APP_A_TYPE, &appInfoStartAddr_A, &appInfoBlockSize_A);
    FLASH_HAL_GetAPPInfo(APP_B_TYPE, &appInfoStartAddr_B, &appInfoBlockSize_B);
    /* Read APP A info */
    appAInfo = *(tAppFlashStatus *)appInfoStartAddr_A;
    /* Read APP B info */
    appBInfo = *(tAppFlashStatus *)appInfoStartAddr_B;
    newestAPP = DoCheckNewestAPPInfo(&appAInfo, &appBInfo);
    return newestAPP;
#endif /* EN_SUPPORT_APP_B */
}

/* Get old APP info */
tAPPType Flash_GetOldAPPType(void)
{
#ifdef EN_SUPPORT_APP_B
    uint32 appInfoStartAddr_A = 0u;
    uint32 appInfoBlockSize_A = 0u;
    tAPPType oldAPP = APP_A_TYPE;
    uint32 appInfoStartAddr_B = 0u;
    uint32 appInfoBlockSize_B = 0u;
    tAPPType newestAPP = APP_A_TYPE;
    tAppFlashStatus appAInfo;
    tAppFlashStatus appBInfo;
#endif /* EN_SUPPORT_APP_B */

    /* Not support APP B, so APP A is always old. */
#ifndef EN_SUPPORT_APP_B
    return APP_A_TYPE;
#else
    FLASH_HAL_GetAPPInfo(APP_A_TYPE, &appInfoStartAddr_A, &appInfoBlockSize_A);
    FLASH_HAL_GetAPPInfo(APP_B_TYPE, &appInfoStartAddr_B, &appInfoBlockSize_B);
    /* Read APP A info */
    appAInfo = *(tAppFlashStatus *)appInfoStartAddr_A;
    /* Read APP B info */
    appBInfo = *(tAppFlashStatus *)appInfoStartAddr_B;
    newestAPP = DoCheckNewestAPPInfo(&appAInfo, &appBInfo);

    if (APP_A_TYPE == newestAPP)
    {
        oldAPP = APP_B_TYPE;
    }
    else if (APP_B_TYPE == newestAPP)
    {
        oldAPP = APP_A_TYPE;
    }
    else
    {
        /* Here is set old APP type is default, just for error */
        oldAPP = APP_A_TYPE;
    }

    return oldAPP;
#endif  /* EN_SUPPORT_APP_B */
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
