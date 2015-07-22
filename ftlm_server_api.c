#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <nxjson.h>
#include "ftm_types.h"
#include "ftlm_server.h"
#include "ftlm_server_api.h"


FTM_INT	xMemID;
FTLM_MSG_SLOT_PTR pMsgSlot = NULL;

int	FTLM_API_init(void)
{
	xMemID 	= shmget(1114, 1 * sizeof(FTLM_MSG_SLOT), IPC_CREAT|0666);
	if (xMemID == -1)
	{
		ERROR("Shared memory allocation error!\n");
		goto error;
	}

	pMsgSlot = (FTLM_MSG_SLOT_PTR)shmat(xMemID, 0, 0);
	if (pMsgSlot == (FTLM_MSG_SLOT_PTR)-1)
	{
		pMsgSlot = NULL;
		ERROR("Shared memory allocation error!\n");
		goto error;	
	}

	TRACE("Shared Memory : %08lx\n", (FTM_ULONG)pMsgSlot);
	return	FTM_RET_OK;

error:

	return	FTM_RET_ERROR;
}

int FTLM_API_sendRequest(FTM_BYTE_PTR pBuff, FTM_ULONG ulBuffLen)
{
	if ((pMsgSlot == NULL) || (ulBuffLen > sizeof(pMsgSlot->pReqBuff)))
	{
		return	FTM_RET_ERROR;
	}
	
	if (pMsgSlot->bReq == FTM_TRUE)
	{
		return	FTM_RET_ERROR;	
	}

	memset(pMsgSlot->pReqBuff, 0, sizeof(pMsgSlot->pReqBuff));
	memcpy(pMsgSlot->pReqBuff, pBuff, ulBuffLen);

	pMsgSlot->ulReqBuffLen	= ulBuffLen;
	pMsgSlot->bReq 			= FTM_TRUE;
			pMsgSlot->bResp = FTM_FALSE;

	return	FTM_RET_OK;
}

int	FTLM_API_waitingForResponse(FTM_BYTE_PTR pBuff, FTM_ULONG ulBuffLen, FTM_ULONG_PTR pulRespLen)
{
	FTM_INT	i;

	if ((pMsgSlot == NULL) || (ulBuffLen > sizeof(pMsgSlot->pReqBuff)))
	{
		return	FTM_RET_ERROR;
	}

	for(i = 0 ; i < 1000 ; i++)
	{
		if (pMsgSlot->bResp == FTM_TRUE)
		{
			if (pMsgSlot->ulRespBuffLen > ulBuffLen)
			{
				ERROR("Buffer too small!\n");
				return	FTM_RET_ERROR;
			}

			memset(pBuff, 0, ulBuffLen);
			memcpy(pBuff, pMsgSlot->pRespBuff, pMsgSlot->ulRespBuffLen);
			if (pulRespLen != NULL)
			{
				*pulRespLen = pMsgSlot->ulRespBuffLen;
			}

			pMsgSlot->bResp = FTM_FALSE;

			return	FTM_RET_OK;
		}
		usleep(1000);	
	}

	return	FTM_RET_ERROR;
}

int	FTLM_API_CFG_save(char *pName)
{
	char	pBuff[2048];
	int		ulLen;
	FTM_ULONG		ulRespLen;
	const nx_json	*pRoot = NULL;
	const nx_json	*pResult= NULL;

	if (pName != NULL)
	{
		ulLen = sprintf(pBuff, "{\"cmd\":\"saveConfig\",\"fileName\":\"%s\"}", pName);
	}
	else
	{
		ulLen = sprintf(pBuff, "{\"cmd\":\"saveConfig\"}");
	}

	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, ulLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);
		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_ERROR;
}

int	FTLM_API_LIGHT_getList(FTM_ID_PTR pLightIDs, FTM_ULONG ulMaxCount, FTM_ULONG_PTR pulLightCount)
{
	const FTM_CHAR_PTR	pCmd = "{\"cmd\":\"getLightList\"}";
	FTM_CHAR		pBuff[FTLM_MSG_FRAME_SIZE];
	FTM_ULONG		i, ulRespLen;
	const nx_json	*pRoot = NULL;
	const nx_json	*pResult= NULL;
	const nx_json	*pArray= NULL;
	const nx_json	*pItem = NULL;


	FTLM_API_sendRequest((FTM_BYTE_PTR)pCmd, strlen(pCmd));
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);
		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

		pArray = nx_json_get(pRoot, "lights");
		if (pArray->type != NX_JSON_ARRAY)
		{
			goto error;	
		}

		*pulLightCount = 0;
		for(i = 0 ; i < pArray->length; i++)
		{
			pItem = nx_json_item(pArray, i);
			if (pItem == NULL)
			{
				break;	
			}

			pLightIDs[(*pulLightCount)++] = (FTM_ID)pItem->int_value;
		}
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	return	FTM_RET_ERROR;
}

int	FTLM_API_LIGHT_getInfo(FTM_ID	xLightID, FTLM_LIGHT_INFO_PTR pInfo)
{
	FTM_CHAR		pCmd[256];
	FTM_CHAR		pBuff[FTLM_MSG_FRAME_SIZE];
	FTM_ULONG		ulRespLen;
	const nx_json	*pRoot = NULL;
	const nx_json	*pResult= NULL;
	const nx_json	*pItem = NULL;

	ASSERT(pInfo != NULL);

	sprintf(pCmd, "{\"cmd\":\"getLightInfo\", \"id\":%d}", (int)xLightID);

	FTLM_API_sendRequest((FTM_BYTE_PTR)pCmd, strlen(pCmd));
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);
		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

		pItem = nx_json_get(pRoot, "id");
		if((pItem == NULL) || (pItem->int_value != (int)xLightID))
		{
			goto error;	
		}
		pInfo->nID = pItem->int_value;

		pItem = nx_json_get(pRoot, "name");
		if (pItem != NULL)
		{
			strcpy(pInfo->pName, pItem->text_value); 
		}

		pItem = nx_json_get(pRoot, "cmd");
		if (pItem == NULL)
		{
			goto error; 
		}
		pInfo->nCmd = pItem->int_value;	

		pItem = nx_json_get(pRoot, "level");
		if (pItem == NULL)
		{
			goto error; 
		}
		pInfo->nLevel = pItem->int_value;	

		pItem = nx_json_get(pRoot, "time");
		if (pItem == NULL)
		{
			goto error; 
		}
		pInfo->nTime = pItem->int_value;	

	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	return	FTM_RET_ERROR;
}


int	FTLM_API_LIGHT_setCtrls(FTLM_LIGHT_CTRL_PTR pLights, int nLights)
{
	char	pBuff[2048];
	int		i, nBuffLen = 0;
	FTM_ULONG ulRespLen;

	ASSERT(pLights != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"setLightCtrls\", \"lights\":[");
	for(i = 0 ; i < nLights ; i++)
	{
		if (i != 0)
		{
			nBuffLen += sprintf(&pBuff[nBuffLen], ", ");
		}
		nBuffLen += sprintf(&pBuff[nBuffLen], "{\"id\":%d,\"cmd\":%d,\"level\":%d,\"time\":%d}",
								pLights[i].nID, pLights[i].nCmd, pLights[i].nLevel, pLights[i].nTime);
	}
	nBuffLen += sprintf(&pBuff[nBuffLen], "]}");

	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);	
	}
	return	FTM_RET_OK;

}

int	FTLM_API_LIGHT_getCtrls(FTLM_LIGHT_CTRL_PTR pLights, int nMaxLights, int *pnLights)
{
	const nx_json *pRoot = NULL;
	char	pBuff[2048];
	int		i, nBuffLen = 0, nLightCount;
	FTM_ULONG ulRespLen;

	ASSERT(pLights != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"getLightCtrls\"}");
	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		const nx_json *pJSONLights;

		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pJSONLights = nx_json_get(pRoot, "lights");
		if ((pJSONLights == NULL) || (pJSONLights->type != NX_JSON_ARRAY))
		{
			goto error;	
		}

		nLightCount = 0;
		for(i = 0 ; i < pJSONLights->length; i++)
		{
			const nx_json *pJSONID;
			const nx_json *pJSONCmd;
			const nx_json *pJSONLevel;
			const nx_json *pJSONTime;

			const nx_json *pJSONLight = nx_json_item(pJSONLights, i);
			if (pJSONLight == NULL)
			{
				continue;	
			}

			pJSONID = nx_json_get(pJSONLight, "id");
			if (pJSONID == NULL)
			{
				continue;
			}
		
			pJSONCmd = nx_json_get(pJSONLight, "cmd");
			if (pJSONCmd == NULL)
			{
				continue;
			}
		
			pJSONLevel= nx_json_get(pJSONLight, "level");
			if (pJSONLevel == NULL)
			{
				continue;
			}
		
			pJSONTime = nx_json_get(pJSONLight, "time");
			if (pJSONTime == NULL)
			{
				continue;
			}
	
			pLights[nLightCount].nID 	= pJSONID->int_value;
			pLights[nLightCount].nCmd	= pJSONCmd->int_value;
			pLights[nLightCount].nLevel = pJSONLevel->int_value;
			pLights[nLightCount].nTime 	= pJSONTime->int_value;
			nLightCount++;
		}

		*pnLights = nLightCount;
	}
	else
	{
		goto error;	
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;
}

int	FTLM_API_LIGHT_setGroups(FTLM_LIGHT_GROUP_PTR pLights, int nLights)
{
	char	pBuff[2048];
	int		i, j, nBuffLen = 0;
	FTM_ULONG ulRespLen;

	ASSERT(pLights != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"setLightGroups\", \"lights\":[");
	for(i = 0 ; i < nLights ; i++)
	{
		if (i != 0)
		{
			nBuffLen += sprintf(&pBuff[nBuffLen], ", ");
		}
		nBuffLen += sprintf(&pBuff[nBuffLen], "{\"id\":%d,\"groups\":[", pLights[i].nID);

		for(j = 0 ; j < pLights[i].nGroups; j++)
		{
			if (j != 0)
			{
				nBuffLen += sprintf(&pBuff[nBuffLen], ", ");
			}
			nBuffLen += sprintf(&pBuff[nBuffLen], "%d", pLights[i].pGroups[j]);
		}
		nBuffLen += sprintf(&pBuff[nBuffLen], "]}");

	}
	nBuffLen += sprintf(&pBuff[nBuffLen], "]}");

	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);	
	}

	return	FTM_RET_OK;
}

int	FTLM_API_LIGHT_getGroups(FTLM_LIGHT_GROUP_PTR pLights, int nMaxLights, int *pnLights)
{
	const nx_json *pRoot = NULL;
	char	pBuff[2048];
	int		i, j, nBuffLen = 0, nLightCount;
	FTM_ULONG ulRespLen;

	ASSERT(pLights != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"getLightGroups\"}");
	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		const nx_json *pJSONLights;

		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pJSONLights = nx_json_get(pRoot, "lights");
		if ((pJSONLights == NULL) || (pJSONLights->type != NX_JSON_ARRAY))
		{
			goto error;	
		}

		nLightCount = 0;
		for(i = 0 ; i < pJSONLights->length; i++)
		{
			const nx_json *pJSONID;
			const nx_json *pJSONGroups;
			const nx_json *pJSONLight = nx_json_item(pJSONLights, i);
			if (pJSONLight == NULL)
			{
				continue;	
			}

			pJSONID = nx_json_get(pJSONLight, "id");
			if (pJSONID == NULL)
			{
				continue;
			}
		
			pJSONGroups = nx_json_get(pJSONLight, "groups");
			if (pJSONGroups == NULL)
			{
				continue;
			}
		
			pLights[nLightCount].nID = pJSONID->int_value;
			pLights[nLightCount].nGroups = 0;
			for(j = 0 ; j < pJSONGroups->length ; j++)
			{
				const 	nx_json	*pJSONGroupID;

				pJSONGroupID = nx_json_item(pJSONGroups, j);
				if (pJSONGroupID != NULL)
				{
					pLights[nLightCount].pGroups[pLights[nLightCount].nGroups] = pJSONGroupID->int_value;
					pLights[nLightCount].nGroups++;
				}
			}

			nLightCount++;
		}

		*pnLights = nLightCount;
	}
	else
	{
		goto error;	
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;
}

int	FTLM_API_GROUP_getList(unsigned long * pGroupIDs, unsigned long ulMaxCount, unsigned long * pulCount)
{
	const FTM_CHAR_PTR	pCmd = "{\"cmd\":\"getGroupList\"}";
	FTM_CHAR		pBuff[FTLM_MSG_FRAME_SIZE];
	FTM_ULONG		i, ulRespLen;
	const nx_json	*pRoot = NULL;
	const nx_json	*pResult= NULL;
	const nx_json	*pArray= NULL;
	const nx_json	*pItem = NULL;


	FTLM_API_sendRequest((FTM_BYTE_PTR)pCmd, strlen(pCmd));
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);
		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

		pArray = nx_json_get(pRoot, "groups");
		if (pArray ->type != NX_JSON_ARRAY)
		{
			goto error;	
		}

		*pulCount = 0;
		for(i = 0 ; i < pArray->length; i++)
		{
			pItem = nx_json_item(pArray, i);
			if (pItem == NULL)
			{
				break;	
			}

			pGroupIDs[(*pulCount)++] = (FTM_ID)pItem->int_value;
		}
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	return	FTM_RET_ERROR;
}

int	FTLM_API_GROUP_getInfo(FTM_ID	xGroupID, FTLM_GROUP_INFO_PTR pInfo)
{
	FTM_CHAR		pCmd[256];
	FTM_CHAR		pBuff[FTLM_MSG_FRAME_SIZE];
	FTM_ULONG		ulRespLen;
	const nx_json	*pRoot = NULL;
	const nx_json	*pResult= NULL;
	const nx_json	*pItem = NULL;
	const nx_json	*pLights = NULL;

	ASSERT(pInfo != NULL);

	sprintf(pCmd, "{\"cmd\":\"getGroupInfo\", \"id\":%d}", (int)xGroupID);

	FTLM_API_sendRequest((FTM_BYTE_PTR)pCmd, strlen(pCmd));
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);
		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

		pItem = nx_json_get(pRoot, "id");
		if((pItem == NULL) || (pItem->int_value != (int)xGroupID))
		{
			goto error;	
		}
		pInfo->nID = pItem->int_value;	

		pItem = nx_json_get(pRoot, "name");
		if (pItem != NULL)
		{
			strcpy(pInfo->pName, pItem->text_value); 
		}

		pItem = nx_json_get(pRoot, "cmd");
		if (pItem == NULL)
		{
			goto error; 
		}
		pInfo->nCmd = pItem->int_value;	

		pItem = nx_json_get(pRoot, "level");
		if (pItem == NULL)
		{
			goto error; 
		}
		pInfo->nLevel = pItem->int_value;	

		pItem = nx_json_get(pRoot, "time");
		if (pItem == NULL)
		{
			goto error; 
		}
		pInfo->nTime = pItem->int_value;	

		pInfo->nLight = 0;
		pLights = nx_json_get(pRoot, "lights");
		if (pLights->type != NX_JSON_NULL)
		{
			while((pItem = nx_json_item(pLights, pInfo->nLight)) != NULL)
			{

				if (pItem->type == NX_JSON_NULL)
				{
					break;	
				}
				pInfo->pLightIDs[pInfo->nLight] = pItem->int_value;
				pInfo->nLight++;
			}
		}

	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	return	FTM_RET_ERROR;
}

int	FTLM_API_GROUP_setCtrls(FTLM_GROUP_CTRL_PTR pGroups, int nGroups)
{
	char	pBuff[2048];
	int		i, nBuffLen = 0;
	FTM_ULONG ulRespLen;
	const nx_json	*pRoot= NULL;
	const nx_json	*pResult= NULL;

	ASSERT(pGroups != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"setGroupCtrls\", \"groups\":[");
	for(i = 0 ; i < nGroups ; i++)
	{
		if (i != 0)
		{
			nBuffLen += sprintf(&pBuff[nBuffLen], ", ");
		}
		nBuffLen += sprintf(&pBuff[nBuffLen], "{\"id\":%d,\"cmd\":%d,\"level\":%d,\"time\":%d}",
								pGroups[i].nID, pGroups[i].nCmd, pGroups[i].nLevel, pGroups[i].nTime);
	}
	nBuffLen += sprintf(&pBuff[nBuffLen], "]}");

	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);	

		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;

error:

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_ERROR;
}

int	FTLM_API_GROUP_getCtrls(FTLM_GROUP_CTRL_PTR pGroups, int nMaxGroups, int *pnGroups)
{
	const nx_json *pRoot = NULL;
	char	pBuff[2048];
	int		i, nBuffLen = 0, nGroupCount;
	FTM_ULONG ulRespLen;

	ASSERT(pGroups != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"getGroupCtrls\"}");
	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		const nx_json *pJSONGroups;

		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pJSONGroups = nx_json_get(pRoot, "lights");
		if ((pJSONGroups == NULL) || (pJSONGroups->type != NX_JSON_ARRAY))
		{
			goto error;	
		}

		nGroupCount = 0;
		for(i = 0 ; i < pJSONGroups->length; i++)
		{
			const nx_json *pJSONID;
			const nx_json *pJSONCmd;
			const nx_json *pJSONLevel;
			const nx_json *pJSONTime;

			const nx_json *pJSONGroup = nx_json_item(pJSONGroups, i);
			if (pJSONGroup == NULL)
			{
				continue;	
			}

			pJSONID = nx_json_get(pJSONGroup, "id");
			if (pJSONID == NULL)
			{
				continue;
			}
		
			pJSONCmd = nx_json_get(pJSONGroup, "cmd");
			if (pJSONCmd == NULL)
			{
				continue;
			}
		
			pJSONLevel= nx_json_get(pJSONGroup, "level");
			if (pJSONLevel == NULL)
			{
				continue;
			}
		
			pJSONTime = nx_json_get(pJSONGroup, "time");
			if (pJSONTime == NULL)
			{
				continue;
			}
	
			pGroups[nGroupCount].nID 	= pJSONID->int_value;
			pGroups[nGroupCount].nCmd	= pJSONCmd->int_value;
			pGroups[nGroupCount].nLevel = pJSONLevel->int_value;
			pGroups[nGroupCount].nTime 	= pJSONTime->int_value;
			nGroupCount++;
		}

		*pnGroups = nGroupCount;
	}
	else
	{
		goto error;	
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;
}

int	FTLM_API_SWITCH_getList(FTM_ID_PTR pSwitchIDs, FTM_ULONG ulMaxCount, FTM_ULONG_PTR pulSwitchCount)
{
	const FTM_CHAR_PTR	pCmd = "{\"cmd\":\"getSwitchList\"}";
	FTM_CHAR		pBuff[FTLM_MSG_FRAME_SIZE];
	FTM_ULONG		i, ulRespLen;
	const nx_json	*pRoot = NULL;
	const nx_json	*pResult= NULL;
	const nx_json	*pArray= NULL;
	const nx_json	*pItem = NULL;


	FTLM_API_sendRequest((FTM_BYTE_PTR)pCmd, strlen(pCmd));
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);
		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

		pArray = nx_json_get(pRoot, "switchs");
		if (pArray ->type != NX_JSON_ARRAY)
		{
			goto error;	
		}

		*pulSwitchCount = 0;
		for(i = 0 ; i < pArray->length; i++)
		{
			pItem = nx_json_item(pArray, i);
			if (pItem == NULL)
			{
				break;	
			}

			pSwitchIDs[(*pulSwitchCount)++] = (FTM_ID)pItem->int_value;
		}
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	
	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
		pRoot = NULL;
	}
	return	FTM_RET_ERROR;
}

int	FTLM_API_SWITCH_setGroups(FTLM_SWITCH_GROUP_PTR pSwitches, int nSwitches)
{
	char	pBuff[2048];
	int		i, j, nBuffLen = 0;
	FTM_ULONG ulRespLen;

	ASSERT(pSwitches != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"setSwitchGroups\", \"switchs\":[");
	for(i = 0 ; i < nSwitches ; i++)
	{
		if (i != 0)
		{
			nBuffLen += sprintf(&pBuff[nBuffLen], ", ");
		}
		nBuffLen += sprintf(&pBuff[nBuffLen], "{\"id\":%d,\"groups\":[", pSwitches[i].nID);

		for(j = 0 ; j < pSwitches[i].nGroups; j++)
		{
			if (j != 0)
			{
				nBuffLen += sprintf(&pBuff[nBuffLen], ", ");
			}
			nBuffLen += sprintf(&pBuff[nBuffLen], "%d", pSwitches[i].pGroups[j]);
		}
		nBuffLen += sprintf(&pBuff[nBuffLen], "]}");

	}
	nBuffLen += sprintf(&pBuff[nBuffLen], "]}");

	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		TRACE("RESP : %s\n", pBuff);	
	}

	return	FTM_RET_OK;
}

int	FTLM_API_SWITCH_getGroups(FTLM_SWITCH_GROUP_PTR pSwitches, int nMaxSwitches, int *pnSwitches)
{
	const nx_json *pRoot = NULL;
	const nx_json *pResult = NULL;
	char	pBuff[2048];
	int		i, j, nBuffLen = 0, nSwitchCount;
	FTM_ULONG ulRespLen;

	ASSERT(pSwitches != NULL);

	nBuffLen = sprintf(pBuff, "{\"cmd\":\"getSwitchGroups\"}");
	FTLM_API_sendRequest((FTM_BYTE_PTR)pBuff, nBuffLen);
	if (FTLM_API_waitingForResponse((FTM_BYTE_PTR)pBuff, sizeof(pBuff), &ulRespLen) == FTM_RET_OK)
	{
		const nx_json *pJSONSwitches;

		pRoot = nx_json_parse_utf8((FTM_CHAR_PTR)pBuff);
		if (pRoot == NULL)
		{
			return	FTM_RET_ERROR;	
		}

		pResult = nx_json_get(pRoot, "result");
		if ((pResult == NULL) || (strcmp(pResult->text_value, "ok") != 0))
		{
			goto error;
		}

		pJSONSwitches = nx_json_get(pRoot, "switches");
		if ((pJSONSwitches == NULL) || (pJSONSwitches->type != NX_JSON_ARRAY))
		{
			goto error;	
		}

		nSwitchCount = 0;
		for(i = 0 ; i < pJSONSwitches->length; i++)
		{
			const nx_json *pJSONID;
			const nx_json *pJSONGroups;
			const nx_json *pJSONSwitch = nx_json_item(pJSONSwitches, i);
			if (pJSONSwitch == NULL)
			{
				continue;	
			}

			pJSONID = nx_json_get(pJSONSwitch, "id");
			if (pJSONID == NULL)
			{
				continue;
			}
		
			pJSONGroups = nx_json_get(pJSONSwitch, "groups");
			if (pJSONGroups == NULL)
			{
				continue;
			}
		
			pSwitches[nSwitchCount].nID = pJSONID->int_value;
			pSwitches[nSwitchCount].nGroups = 0;
			for(j = 0 ; j < pJSONGroups->length ; j++)
			{
				const 	nx_json	*pJSONGroupID;

				pJSONGroupID = nx_json_item(pJSONGroups, j);
				if (pJSONGroupID != NULL)
				{
					pSwitches[nSwitchCount].pGroups[pSwitches[nSwitchCount].nGroups] = pJSONGroupID->int_value;
					pSwitches[nSwitchCount].nGroups++;
				}
			}

			nSwitchCount++;
		}

		*pnSwitches = nSwitchCount;
	}
	else
	{
		goto error;	
	}

	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;

error:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}

	return	FTM_RET_OK;
}
