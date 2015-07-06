#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ftlm_server.h"
#include "ftlm_object.h"
#include "ftm_mem.h"
#include "ftlm.h"
#include <nxjson.h>

static FTM_VOID_PTR FTLM_SERVER_loop(FTM_VOID_PTR pData);
static FTM_RET 		FTLM_SERVER_process(FTM_BYTE_PTR pReqBuff, FTM_ULONG ulReqBuffLen, FTM_BYTE_PTR pRespBuff, FTM_ULONG ulRespBuffLen, FTM_ULONG_PTR ulRespLen);

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
	pServer->xMemID 	= shmget(pConfig->xMemKey, pConfig->ulSlotCount * sizeof(FTLM_MSG_SLOT), IPC_CREAT|0666);
	if (pServer->xMemID == -1)
	{
		ERROR("Shared memory allocation error!\n");
		goto error;
	}

	pServer->pMsgSlot = (FTLM_MSG_SLOT_PTR)shmat(pServer->xMemID, 0, 0);
	if (pServer->pMsgSlot == (FTLM_MSG_SLOT_PTR)-1)
	{
		ERROR("Shared memory allocation error!\n");
		goto error;	
	}

	TRACE("Shared Memory : %08lx\n", (FTM_ULONG)pServer->pMsgSlot);
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

		for(i = 0 ; i < 1 ; i++)//pServer->ulSlotCount ; i++)
		{
			if ((pServer->pMsgSlot[i].bResp == FTM_FALSE) && (pServer->pMsgSlot[i].bReq == FTM_TRUE))
			{
				TRACE("Service called[%s]\n", (FTM_CHAR_PTR)pServer->pMsgSlot[i].pReqBuff);
				nRet = FTLM_SERVER_process(	pServer->pMsgSlot[i].pReqBuff, 
											pServer->pMsgSlot[i].ulReqBuffLen, 
											pServer->pMsgSlot[i].pRespBuff, 
											sizeof(pServer->pMsgSlot[i].pRespBuff),
											&pServer->pMsgSlot[i].ulRespBuffLen);
				if (FTM_RET_OK == nRet)
				{
					TRACE("Service response[%lu] : %s\n", pServer->pMsgSlot[i].ulRespBuffLen, (FTM_CHAR_PTR)pServer->pMsgSlot[i].pRespBuff);
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


FTM_RET FTLM_SERVER_process(FTM_BYTE_PTR pReqBuff, FTM_ULONG ulReqBuffLen, FTM_BYTE_PTR pRespBuff, FTM_ULONG ulRespBuffLen, FTM_ULONG_PTR pulRespLen)
{
	int	nRespLen = 0;
	const nx_json *pRoot;
	const nx_json *pItem;

	pRoot = nx_json_parse_utf8((char *)pReqBuff);
	if (pRoot == NULL)
	{
		return FTM_RET_ERROR;
	}

	pItem = nx_json_get(pRoot, "cmd");
	if (pItem == NULL)
	{
		return FTM_RET_ERROR;
	}

	memset(pRespBuff, 0, ulRespBuffLen);

	pRespBuff[nRespLen++] = '{';

	if (strcmp(pItem->text_value, "getLightList") == 0)
	{
		int	i, nCount;
		
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"lights\":[");

		nCount = FTLM_OBJ_getLightCount();
		for(i = 0 ; i < nCount ; i++)
		{
			FTLM_LIGHT_PTR	pLight = FTLM_OBJ_getLightAt(i);
			if (pLight != NULL)
			{
				if (i != 0)
				{
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], ", %d", (int)pLight->xCommon.xID);
				}
				else
				{
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], "%d", (int)pLight->xCommon.xID);
				}
			}
		}

		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");
		*pulRespLen = nRespLen;
	}
	else if (strcmp(pItem->text_value, "getLightInfo") == 0)
	{
		pItem = nx_json_get(pRoot, "id");
		if (pItem != NULL)
		{
			FTLM_LIGHT_PTR	pLight = FTLM_OBJ_getLight((FTM_ID)pItem->int_value);
			if (pLight != NULL)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"id\":%d,\"name\":\"%s\",\"cmd\":%lu,\"level\":%lu,\"time\":%lu",
					(int)pLight->xCommon.xID, pLight->xCommon.pName, pLight->ulCmd, pLight->ulLevel, pLight->ulTime);
			
			}
			else
			{
				goto error;
			
			}
		}
	}
	else if (strcmp(pItem->text_value, "getLightCtrls") == 0)
	{
		int	i;
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"lights\":[");
		for(i = 0 ; i < FTLM_OBJ_getLightCount(); i++)
		{
			if (i != 0)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], ",");
			}

			FTLM_LIGHT_PTR	pLight = FTLM_OBJ_getLightAt(i);
			if (pLight != NULL)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "{\"id\":%d,\"cmd\":%lu,\"level\":%lu,\"time\":%lu}",
					(int)pLight->xCommon.xID, pLight->ulCmd, pLight->ulLevel, pLight->ulTime);
			
			}
			else
			{
				goto error;
			
			}
		}
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");
	}
	else if (strcmp(pItem->text_value, "setLightCtrls") == 0)
	{
		const nx_json *pLights;
		int	i;

		pLights = nx_json_get(pRoot, "lights");
		if ((pLights == NULL) || (pLights->type != NX_JSON_ARRAY))
		{
			goto error;	
		}

		for(i = 0 ; i < pLights->length ; i++)
		{
			const nx_json *pLight;
			const nx_json *pID;
			const nx_json *pCmd;
			const nx_json *pLevel;
			const nx_json *pTime;

			pLight = nx_json_item(pLights, i);
			if (pLight == NULL)
			{
				printf("Light[%d] not found\n", i);
				goto error;	
			}

			pID = nx_json_get(pLight, "id");
			if ((pID == NULL) || (pID->type != NX_JSON_INTEGER))
			{	
				printf("ID not found!\n");
				goto error;
			}

			pCmd = nx_json_get(pLight, "cmd");
			if ((pCmd == NULL) || (pCmd->type != NX_JSON_INTEGER))
			{	
				printf("Cmd  not found!\n");
				goto error;
			}

			pLevel = nx_json_get(pLight, "level");
			if ((pLevel == NULL) || (pLevel->type != NX_JSON_INTEGER))
			{	
				printf("Level not found!\n");
				goto error;
			}

			pTime = nx_json_get(pLight, "time");
			if ((pTime == NULL) || (pTime->type != NX_JSON_INTEGER))
			{	
				printf("Time not found!\n");
				goto error;
			}

			FTLM_LIGHT_set((FTM_ID)pID->int_value, (unsigned char)pCmd->int_value, (unsigned char)pLevel->int_value, (unsigned char)pTime->int_value);
		}
	}
	else if (strcmp(pItem->text_value, "getLightGroups") == 0)
	{
		int	i, j;
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"lights\":[");
	
		int	nLightCount = FTLM_OBJ_getLightCount();
		for(i = 0 ; i < nLightCount ; i++)
		{
			if (i != 0)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], ",");
			}

			FTLM_LIGHT_PTR	pLight = FTLM_OBJ_getLightAt(i);
			if (pLight != NULL)
			{
				int	nGroupCount = FTLM_OBJ_getGroupCount();
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "{\"id\":%d,\"groups\":[", (int)pLight->xCommon.xID);	
			
				for(j = 0 ; j < nGroupCount ; j++)
				{
					FTLM_GROUP_PTR pGroup = FTLM_OBJ_getGroupAt(j);
					if (FTLM_GROUP_getLight(pGroup, pLight->xCommon.xID) != NULL)
					{
						if (j != 0)
						{
							nRespLen += sprintf((char *)&pRespBuff[nRespLen], ",");
						}

						nRespLen += sprintf((char *)&pRespBuff[nRespLen], "%d", (int)pGroup->xCommon.xID);	
					}
				
				}

				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]}");
			}
		}
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");

	}
	else if (strcmp(pItem->text_value, "getGroupList") == 0)
	{
		int	i, nCount;
		
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"groups\":[");

		nCount = FTLM_OBJ_getGroupCount();
		for(i = 0 ; i < nCount ; i++)
		{
			FTLM_GROUP_PTR	pGroup = FTLM_OBJ_getGroupAt(i);
			if (pGroup != NULL)
			{
				if (i != 0)
				{
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], ", %d", (int)pGroup->xCommon.xID);
				}
				else
				{
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], "%d", (int)pGroup->xCommon.xID);
				}
			}
		}

		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");
		*pulRespLen = nRespLen;
	}
	else if (strcmp(pItem->text_value, "getGroupInfo") == 0)
	{
		pItem = nx_json_get(pRoot, "id");
		if (pItem != NULL)
		{
			FTLM_GROUP_PTR	pGroup = FTLM_OBJ_getGroup((FTM_ID)pItem->int_value);
			if (pGroup != NULL)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"id\":%d,\"name\":\"%s\",\"cmd\":%lu,\"level\":%lu,\"time\":%lu",
					(int)pGroup->xCommon.xID, pGroup->xCommon.pName, pGroup->ulCmd, pGroup->ulLevel, pGroup->ulTime);
			
			}
			else
			{
				goto error;
			
			}
		}
	}
	else if (strcmp(pItem->text_value, "getGroupCtrls") == 0)
	{
		int	i;
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"lights\":[");
		for(i = 0 ; i < FTLM_OBJ_getGroupCount(); i++)
		{
			if (i != 0)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], ",");
			}

			FTLM_GROUP_PTR	pGroup = FTLM_OBJ_getGroupAt(i);
			if (pGroup != NULL)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "{\"id\":%d,\"cmd\":%lu,\"level\":%lu,\"time\":%lu}",
					(int)pGroup->xCommon.xID, pGroup->ulCmd, pGroup->ulLevel, pGroup->ulTime);
			
			}
			else
			{
				goto error;
			
			}
		}
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");
	}
	else if (strcmp(pItem->text_value, "setGroupCtrls") == 0)
	{
		const nx_json *pGroups;
		int	i;

		pGroups = nx_json_get(pRoot, "groups");
		if ((pGroups == NULL) || (pGroups->type != NX_JSON_ARRAY))
		{
			goto error;	
		}

		for(i = 0 ; i < pGroups->length ; i++)
		{
			const nx_json *pGroup;
			const nx_json *pID;
			const nx_json *pStatus;
			const nx_json *pLevel;
			const nx_json *pTime;

			pGroup = nx_json_item(pGroups, i);
			if (pGroup == NULL)
			{
				goto error;	
			}

			pID = nx_json_get(pGroup, "id");
			if ((pID == NULL) || (pID->type != NX_JSON_INTEGER))
			{	
				goto error;
			}

			pStatus = nx_json_get(pGroup, "cmd");
			if ((pStatus == NULL) || (pStatus->type != NX_JSON_INTEGER))
			{	
				goto error;
			}

			pLevel = nx_json_get(pGroup, "level");
			if ((pLevel == NULL) || (pLevel->type != NX_JSON_INTEGER))
			{	
				goto error;
			}

			pTime = nx_json_get(pGroup, "time");
			if ((pTime == NULL) || (pTime->type != NX_JSON_INTEGER))
			{	
				goto error;
			}

			FTLM_GROUP_set((FTM_ID)pID->int_value, (unsigned char)pStatus->int_value, (unsigned char)pLevel->int_value, (unsigned char)pTime->int_value);
		}
	}
	else if (strcmp(pItem->text_value, "getSwitchList") == 0)
	{
		int	i, nCount;
		
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"switchs\":[");

		nCount = FTLM_OBJ_getSwitchCount();
		for(i = 0 ; i < nCount ; i++)
		{
			FTLM_SWITCH_PTR	pSwitch = FTLM_OBJ_getSwitchAt(i);
			if (pSwitch != NULL)
			{
				if (i != 0)
				{
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], ", %d", (int)pSwitch->xCommon.xID);
				}
				else
				{
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], "%d", (int)pSwitch->xCommon.xID);
				}
			}
		}

		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");
		*pulRespLen = nRespLen;
	}
	else if (strcmp(pItem->text_value, "getSwitchGroups") == 0)
	{
		int	i, j;
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"switches\":[");
	
		int	nSwitchCount = FTLM_OBJ_getSwitchCount();
		for(i = 0 ; i < nSwitchCount ; i++)
		{
			if (i != 0)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], ",");	
			}
			FTLM_SWITCH_PTR	pSwitch = FTLM_OBJ_getSwitchAt(i);
			if (pSwitch != NULL)
			{
				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "{\"id\":%d,\"groups\":[", (int)pSwitch->xCommon.xID);
				for(j = 0 ; j < FTM_LIST_count(pSwitch->pGroupList); j++)
				{
					FTM_ID	xGroupID;

					FTM_LIST_getAt(pSwitch->pGroupList, j, (FTM_VOID_PTR _PTR_)&xGroupID);
					if (j != 0)
					{
						nRespLen += sprintf((char *)&pRespBuff[nRespLen], ",");	
					}
					nRespLen += sprintf((char *)&pRespBuff[nRespLen], "%d", (int)xGroupID);	
				}

				nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]}");
			}
		}
		nRespLen += sprintf((char *)&pRespBuff[nRespLen], "]");

	}
	else if (strcmp(pItem->text_value, "saveConfig") == 0)
	{
	}
	else
	{
		goto error;
	}

	if (nRespLen > 1)
	{
		pRespBuff[nRespLen++] = ',';
	}

	nRespLen += sprintf((char *)&pRespBuff[nRespLen], "\"result\":\"ok\"}");

	*pulRespLen = nRespLen;

	nx_json_free(pRoot);

	return	FTM_RET_OK;

error:

	*pulRespLen = sprintf((char *)pRespBuff, "{\"result\":\"error\"}");

	nx_json_free(pRoot);

	return	FTM_RET_OK;
}
