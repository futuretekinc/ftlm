#include <unistd.h>
#include <string.h>
#include "ftlm_server.h"
#include "ftm_mem.h"

static FTM_VOID_PTR FTLM_SERVER_process(FTM_VOID_PTR pData);

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
		goto error;
	}

	pServer->pMsgSlot = (FTLM_MSG_SLOT_PTR)shmat(pServer->xMemID, 0, 0);
	if (pServer->pMsgSlot == (FTLM_MSG_SLOT_PTR)-1)
	{
		goto error;	
	}

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

	if (pthread_create(&pServer->hThread, NULL, FTLM_SERVER_process, pServer) != 0)
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

FTM_VOID_PTR FTLM_SERVER_process(FTM_VOID_PTR pData)
{
	FTLM_SERVER_PTR  	pServer = (FTLM_SERVER_PTR)pData;
	FTM_RET				nRet = 0;


	while(pServer->bRun)
	{
	}

	pServer->hThread = 0;
 
 	return  0;
 }

FTM_RET			FTLM_SERVER_sendFrame(FTLM_SERVER_PTR pServer, FTLM_FRAME_PTR pFrame)
{
	return	FTM_RET_OK;
}

FTM_RET			FTLM_SERVER_setMessageCB(FTLM_SERVER_PTR pServer, FTLM_SERVER_CB_MESSAGE pCB)
{
	return	FTM_RET_OK;
}
