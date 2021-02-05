/*
 * @ 名称: multi_cyc_fifo.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月5日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月5日 Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#include "multi_cyc_fifo.h"

typedef enum
{
    FIFO_EMPTY,           /*fifo empty*/
    FIFO_USING,           /*fifo using*/
    FIFO_FULL            /*fifo full */
}tFifoStatus;

typedef struct
{
    tId xOwnerId;                  /*owner fifo id*/
    tLen xFifoLen;                 /*fifo len*/
    tLen xReadAddr;                /*read fifo addr*/
    tLen xWriteAddr;               /*write fifo addr*/
    tFifoStatus eFifoStatus;       /*fifo status*/
    unsigned char *pStartFifoAddr; /*start fifo addr*/
    void *pvNextFifoList;          /*next fifo list*/
}tFifoInfo;

/*********************Marco define***************************/
#define STRUCT_LEN (20u) /*every fifo struct used space*/
#define TOTAL_BYTES ((STRUCT_LEN) * (FIFO_NUM) + TOTAL_FIFO_BYTES) /*config total bytes*/
/**********************************************************/

/*********************Static value define**********************/
static unsigned char gs_ucFifo[TOTAL_BYTES] = {0};           /*total fifo len*/
static tFifoInfo *gs_pstListHeader = (tFifoInfo *)0u;        /*manage list fifo header*/
static tLen gs_xCleanFifoLen = TOTAL_BYTES;                  /*can used fifo len*/

/**********************************************************
** Description          :   Add counter
** Input Parameter  :   i_xFifoLen total fifo len
** Modify Paramerer :   m_pxCounter need modify counter  Read/Write counter
***********************************************************/
#define AddCounter(i_xFifoLen, m_pxCounter)\
do{\
    (*(m_pxCounter))++;\
    if(*(m_pxCounter) >= (i_xFifoLen))\
    {\
        *(m_pxCounter) -= (i_xFifoLen);\
    }\
}while(0)

/*add write counter used in write fifo*/
#define AddWriteCounter(m_pstNode)\
do{\
    AddCounter(m_pstNode->xFifoLen, &(m_pstNode->xWriteAddr));\
}while(0)

/*Check and change current  write FIFO status*/
#define CheckAndChangeWriteFIFOStatus(m_pstNode) \
do{\
    DisableAllInterrupts();\
    if((m_pstNode)->xWriteAddr == (m_pstNode)->xReadAddr)\
    {\
        (m_pstNode)->eFifoStatus = FIFO_FULL;\
    }\
    else\
    {\
        (m_pstNode)->eFifoStatus = FIFO_USING;\
    }\
    EnableAllInterrupts();\
}while(0u)

/*add read counter used in read fifo*/
#define AddReadCounter(m_pstNode)\
do{\
    AddCounter(m_pstNode->xFifoLen, &(m_pstNode->xReadAddr));\
}while(0)

/*Check and change current read FIFO status*/
#define CheckAndChangeReadFIFOStatus(m_pstNode) \
do{\
    DisableAllInterrupts();\
    if((m_pstNode)->xWriteAddr == (m_pstNode)->xReadAddr)\
    {\
        (m_pstNode)->eFifoStatus = FIFO_EMPTY;\
    }\
    else\
    {\
        (m_pstNode)->eFifoStatus = FIFO_USING;\
    }\
    EnableAllInterrupts();\
}while(0u)


/*get fifo list header*/
#define GetListHeader(o_psListHeader)\
do{\
    (o_psListHeader) = gs_pstListHeader;\
}while(0)

/**********************************************************/

/*********************Static function define*******************/
static void AddInList(tFifoInfo *i_pstFifoNode, tFifoInfo **m_ppstHeader, tErroCode *o_peAddStatus);
static void FindFifo(tId i_xFifoId, tFifoInfo **o_ppstNode, tErroCode *o_peFindStatus);
/**********************************************************/

/**********************************************************
**  Function Name   :   ApplyFifo
**  Description     :   Apply a fifo
**  Input Parameter :   i_xApplyFifoLen need apply fifo len
                        i_xFifoId fifo id. Use find this fifo.
**  Modify Parameter    :   none
**  Output Parameter    :   o_peApplyStatus apply status. If apply success ERRO_NONE, else ERRO_XXX
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-27
**********************************************************/
void ApplyFifo(tLen i_xApplyFifoLen, tLen i_xFifoId, tErroCode *o_peApplyStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;
    tLen xNodeNeedSpace = 0u;
    uint32 CleanFIFOLenTmp = 0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peApplyStatus)
    {
        return;
    }

    /*confirm apply FIFO header start align as 4Bytes*/
    CleanFIFOLenTmp = (uint32)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]) & 0x03u;

    if((i_xApplyFifoLen + STRUCT_LEN + CleanFIFOLenTmp) > gs_xCleanFifoLen)
    {
        *o_peApplyStatus = ERRO_OVER_MAX;

        return;
    }
#endif

    FindFifo(i_xFifoId, &pstNode, o_peApplyStatus);
    if(ERRO_NONE == *o_peApplyStatus) /*note if ERRO_NONE that means id have registered.*/
    {
        *o_peApplyStatus = ERRO_REGISTERED_SECOND;
        return;
    }

    if(CleanFIFOLenTmp)
    {
        gs_xCleanFifoLen -= CleanFIFOLenTmp;
    }

    pstNode = (tFifoInfo *)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]);
    pstNode->xOwnerId = i_xFifoId;
    pstNode->pvNextFifoList = (void *)0u;
    pstNode->xFifoLen = i_xApplyFifoLen;
    pstNode->xReadAddr = 0u;
    pstNode->xWriteAddr = 0u;
    pstNode->pStartFifoAddr = (unsigned char *)((tFifoInfo *)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]) + 1u);
    pstNode->eFifoStatus = FIFO_EMPTY;

    xNodeNeedSpace = (tLen)((unsigned char *)((tFifoInfo *)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]) + 1u) -
                       (unsigned char *)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]));

    xNodeNeedSpace += i_xApplyFifoLen;
    gs_xCleanFifoLen -= xNodeNeedSpace;

    AddInList(pstNode, &gs_pstListHeader, o_peApplyStatus);
}

/**********************************************************
**  Function Name   :   WriteDataInFifo
**  Description     :   write data in fifo.
**  Input Parameter :   i_xFifoId   fifo id
                        i_pucWriteDataBuf Need write data buf
                        i_xWriteDatalen  write data len
**  Modify Parameter    :   none
**  Output Parameter    :   o_peWriteStatus write data status. If successfull ERRO_NONE, else ERRO_XX
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-27
**********************************************************/
void WriteDataInFifo(tId i_xFifoId,
                       unsigned char *i_pucWriteDataBuf,
                       tLen i_xWriteDatalen,
                       tErroCode *o_peWriteStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;
    tLen xIndex = 0u;
    tLen xCanWriteTotal = 0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peWriteStatus)
    {
        return;
    }

    if((unsigned char *)0u == i_pucWriteDataBuf)
    {
        *o_peWriteStatus = ERRO_POINTER_NULL;

        return;
    }
#endif

    GetCanWriteLen(i_xFifoId, &xCanWriteTotal, o_peWriteStatus);
    if(ERRO_NONE != *o_peWriteStatus)
    {
        return;
    }

    if(i_xWriteDatalen > xCanWriteTotal)
    {
        *o_peWriteStatus = ERRO_OVER_MAX;

        return;
    }

    FindFifo(i_xFifoId, &pstNode, o_peWriteStatus);
    if(ERRO_NONE != *o_peWriteStatus)
    {
        return;
    }

    for(xIndex = 0u; xIndex < i_xWriteDatalen; xIndex++)
    {
        (pstNode->pStartFifoAddr)[pstNode->xWriteAddr] = i_pucWriteDataBuf[xIndex];
        AddWriteCounter(pstNode);
    }

    /*check and change write FIFO status*/
    CheckAndChangeWriteFIFOStatus(pstNode);

    *o_peWriteStatus = ERRO_NONE;
}

/**********************************************************
**  Function Name   :   ReadDataFromFifo
**  Description     :   Read data from fifo.
**  Input Parameter :   i_xFifoId need read fifo
                        i_xNeedReadDataLen read data len
**  Modify Parameter    :   none
**  Output Parameter    :   o_pucReadDataBuf need read data buf.
                        o_pxReadLen need read data len
                        o_peReadStatus read status. If read successfull ERRO_NONE, else ERRO_XXX
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-27
**********************************************************/
void ReadDataFromFifo(tId i_xFifoId, tLen i_xNeedReadDataLen,
                           unsigned char *o_pucReadDataBuf,
                           tLen *o_pxReadLen,
                           tErroCode *o_peReadStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;
    tLen xIndex = 0u;
    tLen xCanReadTotal = 0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peReadStatus)
    {
        return;
    }

    if((unsigned char *)0u == o_pucReadDataBuf ||
        (tLen *)0u == o_pxReadLen ||
        (tLen)0u == i_xNeedReadDataLen )
    {
        *o_peReadStatus = ERRO_POINTER_NULL;

        return;
    }
#endif

    GetCanReadLen(i_xFifoId, &xCanReadTotal, o_peReadStatus);
    if(ERRO_NONE != *o_peReadStatus)
    {
        return;
    }

    FindFifo(i_xFifoId, &pstNode, o_peReadStatus);
    if(ERRO_NONE != *o_peReadStatus)
    {
        return;
    }

    xCanReadTotal = xCanReadTotal > i_xNeedReadDataLen ? i_xNeedReadDataLen : xCanReadTotal;
    *o_pxReadLen = xCanReadTotal;

    for(xIndex = 0u; xIndex < xCanReadTotal; xIndex++)
    {
        o_pucReadDataBuf[xIndex] = (pstNode->pStartFifoAddr)[pstNode->xReadAddr] ;
        AddReadCounter(pstNode);
    }

    /*check and change read FIFO status*/
    CheckAndChangeReadFIFOStatus(pstNode);

    *o_peReadStatus = ERRO_NONE;
}

/**********************************************************
**  Function Name   :   GetCanReadLen
**  Description     :   Get fifo have data.
**  Input Parameter :   i_xFifoId fifo id
**  Modify Parameter    :   none
**  Output Parameter    :   o_pxCanReadLen how much data can read.
                        o_peGetStatus get status. If get successfull ERRO_NONE, else ERRO_XXX
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-27
**********************************************************/
void GetCanReadLen(tId i_xFifoId, tLen *o_pxCanReadLen, tErroCode *o_peGetStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peGetStatus)
    {
        return;
    }

    if((tLen *)0u == o_pxCanReadLen)
    {
        *o_peGetStatus = ERRO_POINTER_NULL;

        return;
    }
#endif

    FindFifo(i_xFifoId, &pstNode, o_peGetStatus);
    if(ERRO_NONE != *o_peGetStatus)
    {
        return;
    }

    if(FIFO_USING == pstNode->eFifoStatus)
    {
        *o_pxCanReadLen = (pstNode->xReadAddr > pstNode->xWriteAddr) ?
                          (pstNode->xFifoLen - pstNode->xReadAddr + pstNode->xWriteAddr) :
                          (pstNode->xWriteAddr - pstNode->xReadAddr);
    }
    else if(FIFO_FULL == pstNode->eFifoStatus)
    {
        *o_pxCanReadLen = pstNode->xFifoLen;
    }
    else
    {
        *o_pxCanReadLen = (tLen)0u;
    }

    *o_peGetStatus = ERRO_NONE;
}

/**********************************************************
**  Function Name   :   GetCanWriteLen
**  Description     :   Get can write data.
**  Input Parameter :   i_xFifoId fifo id
**  Modify Parameter    :   none
**  Output Parameter    :   o_pxCanWriteLen how much data can write.
                        o_peGetStatus get data status. If get successfull ERRO_NONE, esle ERRO_XX
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-27
**********************************************************/
void GetCanWriteLen(tId i_xFifoId, tLen *o_pxCanWriteLen, tErroCode *o_peGetStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peGetStatus)
    {
        return;
    }

    if((tLen *)0u == o_pxCanWriteLen)
    {
        *o_peGetStatus = ERRO_POINTER_NULL;

        return;
    }
#endif

    FindFifo(i_xFifoId, &pstNode, o_peGetStatus);
    if(ERRO_NONE != *o_peGetStatus)
    {
        return;
    }

    if(FIFO_USING == pstNode->eFifoStatus)
    {
        *o_pxCanWriteLen = (pstNode->xReadAddr > pstNode->xWriteAddr) ?
                           (pstNode->xReadAddr - pstNode->xWriteAddr) :
                           (pstNode->xFifoLen + pstNode->xReadAddr - pstNode->xWriteAddr);
    }
    else if(FIFO_EMPTY == pstNode->eFifoStatus)
    {
        *o_pxCanWriteLen = pstNode->xFifoLen;
    }
    else
    {
        *o_pxCanWriteLen = (tLen)0u;
    }

    *o_peGetStatus = ERRO_NONE;
}

/**********************************************************
**  Function Name   :   AddInList
**  Description     :   Add node in list
**  Input Parameter :   i_pstFifoNode need add in list node.
**  Modify Parameter    :   m_ppstHeader header list
**  Output Parameter    :   o_peAddStatus add status
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-26
**********************************************************/
static void AddInList(tFifoInfo *i_pstFifoNode, tFifoInfo **m_ppstHeader, tErroCode *o_peAddStatus)
{
    tFifoInfo *pstTemp = (tFifoInfo *)0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peAddStatus)
    {
        return;
    }

    if((tFifoInfo **)0u == m_ppstHeader || (tFifoInfo *)0u == i_pstFifoNode)
    {
        *o_peAddStatus = ERRO_POINTER_NULL;

        return;
    }
#endif

    if((tFifoInfo *)0u == *m_ppstHeader)
    {
        *m_ppstHeader = i_pstFifoNode;
        *o_peAddStatus = ERRO_NONE;

        return;
    }

    pstTemp = *m_ppstHeader;

    while((void *)0u != pstTemp->pvNextFifoList)
    {
        if(i_pstFifoNode == pstTemp)
        {
            *o_peAddStatus = ERRO_REGISTERED_SECOND;

            return;
        }

        pstTemp = (tFifoInfo *)(pstTemp->pvNextFifoList);
    }

    pstTemp->pvNextFifoList = (void *)i_pstFifoNode;
    i_pstFifoNode->pvNextFifoList = (tErroCode *)0u;

    *o_peAddStatus = ERRO_NONE;

}

/**********************************************************
**  Function Name   :   FindFifo
**  Description     :   Find fifo in fifo list.
**  Input Parameter :   FindFifo
**  Modify Parameter    :   none
**  Output Parameter    :   o_ppstNode
                        o_peFindStatus
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2013-3-26
**********************************************************/
static void FindFifo(tId i_xFifoId, tFifoInfo **o_ppstNode, tErroCode *o_peFindStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peFindStatus)
    {
        return;
    }

    if((tFifoInfo **)0u == o_ppstNode)
    {
        *o_peFindStatus = ERRO_POINTER_NULL;

        return;
    }
#endif

    GetListHeader(pstNode);
    while((tFifoInfo *)0u != pstNode)
    {
        if(i_xFifoId == pstNode->xOwnerId)
        {
            *o_peFindStatus = ERRO_NONE;

            *o_ppstNode = pstNode;

            return;
        }

        pstNode = (tFifoInfo *)pstNode->pvNextFifoList;
    }

    *o_peFindStatus = ERRO_NO_NODE;
}

/**********************************************************
**  Function Name   :   ClearFIFO
**  Description     :   Clear FIFO, set read pointer equal write pointer
**  Input Parameter :   i_xFifoId fifo id
**  Modify Parameter    :   none
**  Output Parameter    :   o_peGetStatus get data status. If get successfull ERRO_NONE, esle ERRO_XX
**  Return Value        :   none
**  Version         :   v00.00.01
**  Author          :   Tomlin
**  Created Date        :   2019-6-18
**********************************************************/
void ClearFIFO(tId i_xFifoId, tErroCode *o_peGetStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

#ifdef SAFE_LEVEL_O3
    if((tErroCode *)0u == o_peGetStatus)
    {
        return;
    }
#endif

    FindFifo(i_xFifoId, &pstNode, o_peGetStatus);
    if(ERRO_NONE != *o_peGetStatus)
    {
        return;
    }

    DisableAllInterrupts();
    pstNode->eFifoStatus = FIFO_EMPTY;
    pstNode->xReadAddr = pstNode->xWriteAddr;
    EnableAllInterrupts();

    *o_peGetStatus = ERRO_NONE;
}

/* -------------------------------------------- END OF FILE -------------------------------------------- */
