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

/*every program flash size*/
#define PROGRAM_SIZE (128u)

/*Flash finger print length*/
#define FL_FINGER_PRINT_LENGTH  (17u)

/*invalid UDS services ID*/
#define INVALID_UDS_SERVICES_ID (0xFFu)

typedef enum {
    FLASH_IDLE,           /*flash idle*/
    FLASH_ERASING,        /*erase flash */
    FLASH_PROGRAMMING,    /*program flash*/
    FLASH_CHECKING,       /*check flash*/
    FLASH_WAITTING       /*waitting transmitted message successful*/
} tFlshJobModle;

typedef enum {
    FLASH_OPERATE_ERRO,  /*flash operate erro*/
    FLASH_OPERATE_RIGHT  /*flash operate right*/
} tFlashErroCode;

/** flashloader download step */
typedef enum {
    FL_REQUEST_STEP,      /*flash request step*/
    FL_TRANSFER_STEP,     /*flash transfer data step*/
    FL_EXIT_TRANSFER_STEP,/*exit transfter data step*/
    FL_CHECKSUM_STEP      /*check sum step*/

} tFlDownloadStepType;



/*input parameter : TRUE/FALSE. TRUE = operation successfull, else failled.*/
typedef void (*tpfResponse)(uint8);
typedef void (*tpfReuestMoreTime)(uint8, void (*)(uint8));

/*flash app module init*/
extern void FLASH_APP_Init(void);

/*Init flash download*/
extern void Flash_InitDowloadInfo(void);

/*flash operate main function*/
extern void Flash_OperateMainFunction(void);

/*Flash program region. Called by uds servive 0x36u*/
extern uint8 Flash_ProgramRegion(const uint32 i_addr,
                                 const uint8 *i_pDataBuf,
                                 const uint32 i_dataLen);

/*Is read application inforemation from flash valid?*/
extern uint8 Flash_IsReadAppInfoFromFlashValid(void);

/*Is application in flash valid? If valid return TRUE, else return FALSE.*/
extern uint8 Flash_IsAppInFlashValid(void);

/*save received check sum crc*/
extern void Flash_SavedReceivedCheckSumCrc(uint32 i_receivedCrc);

/*erase flash driver in RAM*/
extern void Flash_EraseFlashDriverInRAM(void);

extern void Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep);

extern tFlDownloadStepType Flash_GetCurDownloadStep(void);


/*Is equal download step?*/
extern boolean Flash_IsEqualDonwloadStep(const tFlDownloadStepType i_donwloadStep);

/*save download data information*/
extern void Flash_SaveDownloadDataInfo(const uint32 i_dataStartAddr, const uint32 i_dataLen);


/*set opeate flash active job*/
extern void Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob,
                                           const tpfResponse i_pfActiveFinshedCallBack,
                                           const uint8 i_requestUDSSerID,
                                           const tpfReuestMoreTime i_pfRequestMoreTimeCallback);


/*Get flash active job*/
extern tFlshJobModle Flash_GetOperateFlashActiveJob(void);


/*set wait do response*/
/*Register flash job callbck. Callback will called when after flash job.*/
extern void Flash_RegisterJobCallback(tpfResponse i_pfDoResponse);

/*save printfigner*/
extern void Flash_SavePrintfigner(const uint8 *i_pPrintfigner, const uint8 i_printfinerLen);

/*write flash application information called by bootloader last step*/
extern uint8 Flash_WriteFlashAppInfo(void);

/*Get newest app info*/
extern tAPPType Flash_GetNewestAPPType(void);

/*Get old app info*/
extern tAPPType Flash_GetOldAPPType(void);

/*get rest hander address*/
extern uint32 Flash_GetResetHandlerAddr(void);

#endif /* FLS_APP_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
