#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftm.h"
#include "ftm_mem.h"
#include "ftm_mqtt.h"
#include "ftlm_config.h"
#include "ftlm_client.h"
#include "ftlm_server.h"
#include "ftlm_object.h"
#include "ftlm_client_msg.h"
#include "nxjson.h"

static FTM_VOID_PTR	FTLM_process(FTM_VOID_PTR pData);
static void 	FTLM_MQTT_messageCB(void *pObj, const struct mosquitto_message *pMessage);
static FTM_RET	FTLM_CLIENT_messageCB(void *pObj, void *pParam);
static FTM_VOID	FTLM_usage(FTM_CHAR_PTR pAppName);
static FTM_RET 	FTLM_CLIENT_CMD_groupCtrl(FTLM_GROUP_PTR pGroup, FTM_ULONG ulCmd, FTM_ULONG ulLevel, FTM_ULONG ulTime);
static FTM_RET	FTLM_CLIENT_CMD_lightCtrl(FTLM_LIGHT_PTR pLight, FTM_ULONG ulCmd, FTM_ULONG ulLevel, FTM_ULONG ulTime);

FTM_DEBUG_CFG	_debugCfg = 
{
	.ulMode = MSG_ALL,
	.xTrace =
	{
		.bToFile= FTM_FALSE,
		.pPath	= "/var/log/ftlm/",
		.pPrefix = "trace",
		.bLine	= FTM_FALSE
	},
	.xError =
	{
		.bToFile= FTM_FALSE,
		.pPath	= "/var/log/ftlm/",
		.pPrefix = "error",
		.bLine	= FTM_FALSE
	},
};


extern char * program_invocation_short_name;
static FTLM_SERVER_PTR	pServer = NULL;
static FTLM_CLIENT_PTR	pClient = NULL;
static FTM_MQTT_PTR		pMQTT = NULL;	

static FTLM_CFG	xCfg;

int main(int nArgc, char *pArgs[])
{
	FTM_INT    		nOpt;
	FTM_BOOL    	bDaemon = FTM_FALSE;
	FTM_CHAR _PTR_  pConfigFileName = program_invocation_short_name;

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgs, "c:d?")) != -1) 
	{   
		switch(nOpt)
		{   
		case    'c':
			{   
				pConfigFileName = optarg;
			}   
			break;

		case    'd':
			{   
				bDaemon = FTM_TRUE;
			}   
			break;

		case    '?':
		default:
			{
				FTLM_usage(pArgs[0]);
				return  0;  
			}
		}   
	}   

	FTM_DEBUG_configSet(&_debugCfg);

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();
	FTM_DEBUG_printModeSet(MSG_ALL);

	FTLM_CFG_init(&xCfg);
	FTLM_CFG_load(&xCfg, pConfigFileName);
#if	DEBUG
	FTLM_CFG_print(&xCfg);
#endif

	if (bDaemon)
	{
		pthread_t	xPThread;

		if (fork() == 0)
		{
			pthread_create(&xPThread, NULL, FTLM_process, &xCfg);
			pthread_join(xPThread, NULL);
		}
	}
	else
	{
		FTLM_process(&xCfg);
	}

	FTLM_CFG_final(&xCfg);

	return	0;
}

FTM_VOID_PTR	FTLM_process(FTM_VOID_PTR pData)
{
	FTLM_CFG_PTR		pConfig = (FTLM_CFG_PTR)pData;
	FTLM_SERVER_CFG		xServerCfg = { .xMemKey = 1114, .ulSlotCount = 10};

	FTLM_OBJ_init(pConfig);

	pMQTT = FTM_MQTT_create(&pConfig->xMQTT);
	if (pMQTT == NULL)
	{
		return	0;
	}
	FTM_MQTT_setMessageCB(pMQTT, FTLM_MQTT_messageCB);

	if (pConfig->xClient.bEnable)
	{
		pClient = FTLM_CLIENT_create(&pConfig->xClient);
		if (pClient == NULL)
		{
			FTM_MQTT_destroy(pMQTT);
			return	0;
		}
		FTLM_CLIENT_setMessageCB(pClient, FTLM_CLIENT_messageCB);
	}
	
	pServer = FTLM_SERVER_create(&xServerCfg);
	if (pServer == NULL)
	{
		FTLM_CLIENT_destroy(pClient);
		FTM_MQTT_destroy(pMQTT);
	
	}

	FTM_MQTT_start(pMQTT);
	if (pClient != NULL)
	{
		FTLM_CLIENT_start(pClient);
	}
	FTLM_SERVER_start(pServer);

	while(1);


	FTLM_SERVER_stop(pServer);
	if (pClient != NULL)
	{
		FTLM_CLIENT_stop(pClient);
	}
	FTM_MQTT_stop(pMQTT);

	FTLM_SERVER_destroy(pServer);
	if (pClient != NULL)
	{
		FTLM_CLIENT_destroy(pClient);
	}
	FTM_MQTT_destroy(pMQTT);

	FTLM_OBJ_final();

	return	0;
}

static void FTLM_MQTT_messageCB(void *pObj, const struct mosquitto_message *pMessage)
{
	char	pBuff[2048];
	const nx_json *pRoot;
	const nx_json *pItem;
	unsigned char	nID;

	memcpy(pBuff, pMessage->payload, pMessage->payloadlen);
	pBuff[pMessage->payloadlen] = 0;

	pRoot = nx_json_parse_utf8((char *)pMessage->payload);
	if (pRoot == NULL)
	{
		return;
	}

	pItem = nx_json_get(pRoot, "id");
	if (pItem == NULL)
	{
		goto finish;	
	}
	
	nID = pItem->int_value;
	printf("nID = %d\n", nID);
finish:
	nx_json_free(pRoot);
}

FTM_RET	FTLM_CLIENT_messageCB(void *pObj, void *pParam)
{
	FTLM_CLIENT_PTR 		pClient = (FTLM_CLIENT_PTR)pObj;
	FTLM_CLIENT_FRAME_PTR 	pFrame = (FTLM_CLIENT_FRAME_PTR)pParam;

	switch(pFrame->nCmd)
	{
	case	FTLM_CLIENT_CMD_RESET:
		{
			pFrame->nRet = 0;
			FTLM_CLIENT_sendFrame(pClient, pFrame);	
		}
		break;

	case	FTLM_CLIENT_CMD_GROUP_CTRL:
		{
			int	i;
			int	nGroups = ((FTLM_CLIENT_GROUP_CTRL_PARAM_PTR)pFrame->pReqParam)->nGroups;
			FTLM_CLIENT_GROUP_CTRL_PTR	pGroups = ((FTLM_CLIENT_GROUP_CTRL_PARAM_PTR)pFrame->pReqParam)->pGroups;

			for(i = 0 ; i < nGroups; i++)
			{
				FTLM_GROUP_PTR	pGroup;

				pGroup = FTLM_OBJ_getGroup(pGroups[i].nID);
				if (pGroup != NULL)
				{
						TRACE("Can't find group[%08x]\n", (unsigned int)pGroups[i].nID);
						return	FTM_RET_ERROR;	
				}

				FTLM_CLIENT_CMD_groupCtrl(pGroup, pGroups[i].nCmd, pGroups[i].nLevel, pGroups[i].nDimmingTime);
			}
		}
		break;

	case	FTLM_CLIENT_CMD_LIGHT_CTRL:
		{
			int	i;

			int	nLights = ((FTLM_CLIENT_LIGHT_CTRL_PARAM_PTR)pFrame->pReqParam)->nLights;
			FTLM_CLIENT_LIGHT_CTRL_PTR	pLights = ((FTLM_CLIENT_LIGHT_CTRL_PARAM_PTR)pFrame->pReqParam)->pLights;

			for(i = 0 ; i < nLights ; i++)
			{
				FTLM_LIGHT_PTR	pLight;

				pLight = FTLM_OBJ_getLight(pLights[i].nID);
				if (pLight != NULL)
				{
					TRACE("Can't find light[%08x]\n", (unsigned int)pLights[i].nID);
					return	FTM_RET_ERROR;	
				}

				FTLM_CLIENT_CMD_lightCtrl(pLight, pLights[i].nCmd, pLights[i].nLevel, pLights[i].nDulationTime);	
			}
		}
		break;

	case	FTLM_CLIENT_CMD_GROUP_SET:
		{
			int	i, j;
			unsigned char nSets = ((FTLM_CLIENT_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
			FTLM_CLIENT_GROUP_SET_PTR	pSet = ((FTLM_CLIENT_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

			for(i = 0 ; i < nSets ; i++)
			{
				for(j = 0 ; j < pSet->nGroups ; j++)
				{
					FTLM_GROUP_PTR	pGroup;

					pGroup = FTLM_OBJ_getGroup(pSet->pGroups[j]);
					if (pGroup != NULL)
					{
						FTLM_GROUP_addLight(pGroup, pSet->nID);
					}
				}

				pSet = (FTLM_CLIENT_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
			}
		}
		break;

	case	FTLM_CLIENT_CMD_SWITCH_GROUP_SET:
		{
			int	i, j;
			unsigned char nSets = ((FTLM_CLIENT_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
			FTLM_CLIENT_SWITCH_GROUP_SET_PTR	pSet = ((FTLM_CLIENT_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

			for(i = 0 ; i < nSets ; i++)
			{
				FTLM_SWITCH_PTR	pSwitch;

				pSwitch = FTLM_OBJ_getSwitch(pSet->nID);
				if (pSwitch != NULL)
				{
					for(j = 0 ; j < pSet->nGroups ; j++)
					{
						FTLM_SWITCH_addGroup(pSwitch, pSet->pGroups[j]);
					}
				}
				pSet = (FTLM_CLIENT_SWITCH_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
			}
		}
		break;

	case FTLM_CLIENT_CMD_GROUP_MAPPING_GET:
		{
			FTLM_LIGHT_CFG_PTR				pLight;
			FTLM_CLIENT_GROUP_MAPPING_PARAM_PTR	pParam = (FTLM_CLIENT_GROUP_MAPPING_PARAM_PTR)pFrame->pRespParam;
			FTLM_CLIENT_GROUP_MAPPING_PTR		pMapping = pParam->pSets;

			pFrame->nRespParamLen = 1;

			pParam->nSets = 0;
			FTM_LIST_iteratorStart(xCfg.pLightList);
			while(FTM_LIST_iteratorNext(xCfg.pLightList, (void **)&pLight) == FTM_RET_OK)
			{
				FTM_ULONG	i;
				FTM_ULONG	ulAllGroupCount;

				pMapping->nID 		= pLight->xID;
				pMapping->nGroups	= 0;
				ulAllGroupCount = FTM_LIST_count(xCfg.pGroupList);
				for(i = 0 ; i < ulAllGroupCount ; i++)
				{
					FTLM_GROUP_CFG_PTR pGroup;

					if (FTM_LIST_getAt(xCfg.pGroupList, i, (FTM_VOID_PTR _PTR_)&pGroup) == FTM_RET_OK)
					{
						if (FTM_LIST_get(pGroup->pLightList, (FTM_VOID_PTR)pLight->xID, NULL) == FTM_RET_OK)
						{
							pMapping->pGroups[pMapping->nGroups++] = pGroup->xID;
						}
					}
				}

				pParam->nSets++;
				pMapping = (FTLM_CLIENT_GROUP_MAPPING_PTR)(((unsigned char *)pMapping) + sizeof(FTLM_CLIENT_GROUP_MAPPING) + pMapping->nGroups);
				pFrame->nRespParamLen += sizeof(FTLM_CLIENT_GROUP_MAPPING) + pMapping->nGroups;
			}
			FTLM_CLIENT_sendFrame(pClient, pFrame);	
		}
		break;

	case FTLM_CLIENT_CMD_SWITCH_MAPPING_GET:
		{
			FTLM_SWITCH_CFG_PTR				pSwitch;
			FTLM_CLIENT_GROUP_MAPPING_PARAM_PTR	pParam = (FTLM_CLIENT_GROUP_MAPPING_PARAM_PTR)pFrame->pRespParam;
			FTLM_CLIENT_GROUP_MAPPING_PTR		pMapping = pParam->pSets;

			pFrame->nRespParamLen = 1;

			pParam->nSets = 0;
			FTM_LIST_iteratorStart(xCfg.pSwitchList);
			while(FTM_LIST_iteratorNext(xCfg.pSwitchList, (void **)&pSwitch) == FTM_RET_OK)
			{
				FTM_ULONG	i;
				FTM_ULONG	ulCount;

				pMapping->nGroups = 0;

				ulCount = FTM_LIST_count(pSwitch->pGroupList);
				for(i = 0 ; i < ulCount ; i++)
				{
					FTLM_GROUP_CFG_PTR pGroup;

					if (FTM_LIST_getAt(xCfg.pGroupList, i, (FTM_VOID_PTR _PTR_)&pGroup) == FTM_RET_OK)
					{
						pMapping->pGroups[pMapping->nGroups++] = pGroup->xID;
					}
				}

				pParam->nSets++;
				pMapping = (FTLM_CLIENT_GROUP_MAPPING_PTR)(((unsigned char *)pMapping) + sizeof(FTLM_CLIENT_GROUP_MAPPING) + pMapping->nGroups);
				pFrame->nRespParamLen += sizeof(FTLM_CLIENT_GROUP_MAPPING) + pMapping->nGroups;
			}
			FTLM_CLIENT_sendFrame(pClient, pFrame);	
		}
		break;

	case FTLM_CLIENT_CMD_GROUP_STATUS_GET:
		{
			FTM_ULONG			i, ulCount;
			FTLM_CLIENT_GROUP_STATUS_PARAM_PTR	pParam = (FTLM_CLIENT_GROUP_STATUS_PARAM_PTR)pFrame->pRespParam;

			TRACE("%8s %8s %8s %8s\n", "ID", "STATUS", "LEVEL", "DIMMING");
			pParam->nGroups = 0;
			ulCount = FTLM_OBJ_getGroupCount();
			for(i = 0 ; i < ulCount ; i++)
			{
				FTLM_GROUP_PTR pGroup;
			
				pGroup = FTLM_OBJ_getGroupAt(i);
				if (pGroup != NULL)
				{
						TRACE("%08x %8d %8lu %16lu\n", (unsigned int)pGroup->xCommon.xID, pGroup->ulCmd, pGroup->ulLevel, pGroup->ulTime); 
						pParam->pGroups[pParam->nGroups].nID 			= pGroup->xCommon.xID;
						pParam->pGroups[pParam->nGroups].nCmd			= pGroup->ulCmd;
						pParam->pGroups[pParam->nGroups].nLevel			= pGroup->ulLevel;
						pParam->pGroups[pParam->nGroups].nDimmingTime 	= pGroup->ulTime;
						pParam->nGroups++;
				}
			}
			pFrame->nRespParamLen = sizeof(FTLM_CLIENT_GROUP_STATUS_PARAM) + sizeof(FTLM_CLIENT_GROUP_STATUS) * pParam->nGroups;
			FTLM_CLIENT_sendFrame(pClient, pFrame);	
		}
		break;

	case FTLM_CLIENT_CMD_LIGHT_STATUS_GET:
		{
			FTM_ULONG			i, ulCount;
			FTLM_CLIENT_LIGHT_STATUS_PARAM_PTR	pParam = (FTLM_CLIENT_LIGHT_STATUS_PARAM_PTR)pFrame->pRespParam;

			TRACE("%8s %8s %8s %8s\n", "ID", "STATUS", "LEVEL", "DULATION");
			pParam->nLights = 0;
			ulCount = FTLM_OBJ_getLightCount();
			for(i = 0 ; i < ulCount ; i++)
			{
				FTLM_LIGHT_PTR pLight;
				
				pLight = FTLM_OBJ_getLightAt(i);
				if (pLight != NULL)
				{
					TRACE("%08x %8d %8lu %16lu\n", (unsigned int)pLight->xCommon.xID, pLight->ulCmd, pLight->ulLevel, pLight->ulTime); 
					pParam->pLights[pParam->nLights].nID 			= pLight->xCommon.xID;
					pParam->pLights[pParam->nLights].nCmd			= pLight->ulCmd;
					pParam->pLights[pParam->nLights].nLevel			= pLight->ulLevel;
					pParam->pLights[pParam->nLights].nDulationTime 	= pLight->ulTime;
					pParam->nLights++;
				}
			}

			pFrame->nRespParamLen = sizeof(FTLM_CLIENT_LIGHT_STATUS_PARAM) + sizeof(FTLM_LIGHT_STATUS) * pParam->nLights;
			FTLM_CLIENT_sendFrame(pClient, pFrame);	
		}
	}

	pFrame->nRet = FTM_RET_OK;
	return	FTM_RET_OK;
}

FTM_RET FTLM_CLIENT_CMD_groupCtrl(FTLM_GROUP_PTR pGroup, FTM_ULONG ulCmd, FTM_ULONG ulLevel, FTM_ULONG ulTime)
{
	FTM_ULONG		i, ulCount;

	ASSERT(pGroup != NULL);

	ulCount = FTLM_GROUP_getLightCount(pGroup);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_LIGHT_PTR	pLight;

		pLight = FTLM_GROUP_getLightAt(pGroup, i);
		if (pLight != NULL)
		{
			FTLM_CLIENT_CMD_lightCtrl(pLight, ulCmd, ulLevel, ulTime);	
		}
	}

	pGroup->ulCmd 	= ulCmd;
	pGroup->ulLevel	= ulLevel;
	pGroup->ulTime	= ulTime;


	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_CMD_lightCtrl(FTLM_LIGHT_PTR pLight, FTM_ULONG ulCmd, FTM_ULONG ulLevel, FTM_ULONG ulTime)
{
	char			pTopic[256];
	char			pMessage[256];
	int				nMessage;

	sprintf(pTopic, "/v/a/g/%s/s/%08x/req", pMQTT->xConfig.pClientID, (unsigned int)pLight->xCommon.xID);
	nMessage = sprintf(pMessage, "{\"cmd\":\"%lu\", \"level\":%lu, \"time\":%lu}", ulCmd, ulLevel, ulTime);

	FTM_MQTT_publish(pMQTT, pTopic, pMessage, nMessage, 0);

	pLight->ulCmd	= ulCmd;
	pLight->ulLevel	= ulLevel;
	pLight->ulTime	= ulTime;

	return	FTM_RET_OK;
}

FTM_VOID    FTLM_usage(FTM_CHAR_PTR pAppName)
{
 	MESSAGE("Usage : %s [-d] [-m 0|1|2]\n", pAppName);
	MESSAGE("\tFutureTek LED Light Manger.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("    -d      Run as a daemon\n");
	MESSAGE("    -m <n>  Set message output mode.\n");
	MESSAGE("            0 - Don't output any messages.\n");
	MESSAGE("            1 - Only general message\n");
	MESSAGE("            2 - All message(include debugging message).\n");
}

