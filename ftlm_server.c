#include <unistd.h>
#include <string.h>
#include "ftlm_server.h"
#include "ftm_mem.h"

static FTM_VOID_PTR FTLM_SERVER_loop(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTLM_SERVER_process(FTM_CHAR_PTR pReqBuff, FTM_ULONG ulReqBuffLen, FTM_CHAR_PTR pRespBuff, FTM_ULONG ulRespBuffLen);

FTLM_SERVER_PTR	FTLM_SERVER_create(FTLM_SERVER_CFG_PTR pConfig)
{
	FTLM_SERVER_PTR pServer = NULL;

	pServer = (FTLM_SERVER_PTR)FTM_MEM_malloc(sizeof(FTLM_SERVER));
	if (pServer == NULL)
	{
		return	NULL;	
	}

	memset(pServer, 0, sizeof(FTLM_SERVER));
	pServer->xMemKey 	= pConfig->xMemKey;
	pServer->ulSlotCount= pConfig->ulSlotCount;
	pServer->xMemID = shmget(pConfig->xMemKey, pConfig->ulSlotCount * sizeof(FTLM_MSG_SLOT), IPC_CREAT|0666);
	if (pServer->xMemID == -1)
	{
		printf("Shared memory allocation error!\n");
		goto error;
	}

	pServer->pMsgSlot = (FTLM_MSG_SLOT_PTR)shmat(pServer->xMemID, 0, 0);
	if (pServer->pMsgSlot == (FTLM_MSG_SLOT_PTR)-1)
	{
		printf("Shared memory allocation error!\n");
		goto error;	
	}

	printf("Shared memory = %08x\n", (FTM_VOID_PTR)pServer->pMsgSlot);
	return	pServer;

error:
	if (pServer != NULL)
	{
		FTM_MEM_free(pServer);	
		pServer = NULL;
	}

	return	NULL;
}

FTM_RET			FTLM_SERVER_destroy(FTLM_SERVER_PTR pServer)
{
	return	FTM_RET_OK;
}

FTM_RET			FTLM_SERVER_start(FTLM_SERVER_PTR	pServer)
{
	ASSERT(pServer != NULL);

	if (pServer->hThread != 0)
	{
		return  FTM_RET_ERROR;
	}

	pServer->bRun = FTM_TRUE;

	if (pthread_create(&pServer->hThread, NULL, FTLM_SERVER_loop, pServer) != 0)
	{
		pServer->bRun = FTM_FALSE;

		return  FTM_RET_ERROR;
	}

	return  FTM_RET_OK;
}

FTM_RET			FTLM_SERVER_stop(FTLM_SERVER_PTR	pServer)
{
	if (pServer->hThread == 0)
	{
		return	FTM_RET_ERROR;	
	}

	pServer->bRun = FTM_FALSE;
	
	while(pServer->hThread != 0)
	{
		sleep(1);	
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTLM_SERVER_loop(FTM_VOID_PTR pData)
{
	FTLM_SERVER_PTR  	pServer = (FTLM_SERVER_PTR)pData;
	FTM_RET				nRet = 0;


	while(pServer->bRun)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < pServer->ulSlotCount ; i++)
		{
			if ((pServer->pMsgSlot[i].bResp == FTM_FALSE) && (pServer->pMsgSlot[i].bReq == FTM_TRUE))
			{
				nRet = FTLM_SERVER_process(	pServer->pMsgSlot[i].pReqBuff, 
											pServer->pMsgSlot[i].ulReqBuffLen, 
											pServer->pMsgSlot[i].pRespBuff, 
											pServer->pMsgSlot[i].ulRespBuffLen);
				if (FTM_RET_OK == nRet)
				{
					pServer->pMsgSlot[i].bResp = FTM_TRUE;	
				}
				pServer->pMsgSlot[i].bReq = FTM_FALSE;	
			}
		}

		usleep(1000);
	}

	pServer->hThread = 0;
 
 	return  0;
}

static FTM_VOID_PTR FTLM_SERVER_process(FTM_CHAR_PTR pReqBuff, FTM_ULONG ulReqBuffLen, FTM_CHAR_PTR pRespBuff, FTM_ULONG ulRespBuffLen)
{

	return	FTM_RET_OK;
}
