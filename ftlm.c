#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm.h"
#include "ftm_mem.h"
#include "ftm_mqtt.h"
#include "ftlm_config.h"
#include "ftlm_client.h"
#include "ftlm_object.h"
#include "ftlm_msg.h"
#include "nxjson.h"

static void 	FTLM_subscribe(struct mosquitto *pMOSQ, void *pObj, const struct mosquitto_message *pMessage);
static FTM_RET	FTLM_recv(void *pObj, void *pParam);

FTM_MQTT_CONFIG	xMQTTConfig = 
{
	
	.pClientID 	= "test",
	.pBrokerIP	= "127.0.0.1",
	.usPort 	= 1883,
	.nKeepAlive = 60,

	.CB_message	= FTLM_subscribe,
};

FTLM_SERVER_CFG	xConfig = 
{
	.pIP		= "10.0.1.100",
	.usPort 	= 9877,

};

FTM_VOID    _showUsage(FTM_CHAR_PTR pAppName);

extern char * program_invocation_short_name;
static FTLM_CTX_PTR	pxCTX = NULL;
static FTM_MQTT_PTR	pMQTT = NULL;	

static FTLM_CFG	xCfg;

int main(int nArgc, char *pArgs[])
{
	FTM_INT    		nOpt;
	FTM_ULONG		i, ulCount;
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
				_showUsage(pArgs[0]);
				return  0;  
			}
		}   
	}   

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();
	FTM_DEBUG_printModeSet(MSG_ALL);

	FTLM_CFG_init(&xCfg);
	FTLM_CFG_load(&xCfg, pConfigFileName);
#if	DEBUG
	FTLM_CFG_print(&xCfg);
#endif

	FTLM_OBJ_init();
	
	ulCount = FTLM_CFG_LIGHT_count(&xCfg);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_LIGHT_CFG_PTR	pConfig = FTLM_CFG_LIGHT_getAt(&xCfg, i);

		if (pConfig != NULL)
		{
			FTLM_LIGHT_create(pConfig);
		}
	}

	ulCount = FTLM_CFG_GROUP_count(&xCfg);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_GROUP_CFG_PTR	pConfig = FTLM_CFG_GROUP_getAt(&xCfg, i);

		if (pConfig != NULL)
		{
			FTLM_GROUP_create(pConfig);
		}
	}

	ulCount = FTLM_CFG_SWITCH_count(&xCfg);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_SWITCH_CFG_PTR	pConfig = FTLM_CFG_SWITCH_getAt(&xCfg, i);

		if (pConfig != NULL)
		{
			FTLM_SWITCH_create(pConfig);
		}
	}

	pMQTT = FTM_MQTT_create(&xMQTTConfig);
	if (pMQTT == NULL)
	{
		return	0;
	}

	pxCTX = FTLM_CLIENT_create(&xCfg.xServer, FTLM_recv);
	if (pxCTX == NULL)
	{
		FTM_MQTT_destroy(pMQTT);
		return	0;
	}

	FTM_MQTT_start(pMQTT);
	FTLM_CLIENT_start(pxCTX);

	while(1)
	{
	
	
	}

	FTLM_CFG_final(&xCfg);
	FTM_MQTT_destroy(pMQTT);

	return	0;
}


static void FTLM_subscribe(struct mosquitto *pMOSQ, void *pObj, const struct mosquitto_message *pMessage)
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
	printf("ID : %d\n", nID);

finish:
	nx_json_free(pRoot);
}

FTM_RET	FTLM_recv(void *pObj, void *pParam)
{
	FTLM_CTX_PTR 		pxCTX = (FTLM_CTX_PTR)pObj;
	FTLM_FRAME_PTR 	pFrame = (FTLM_FRAME_PTR)pParam;

	switch(pFrame->nCmd)
	{
	case	FTLM_CMD_RESET:
		{
			pFrame->nRet = 0;
			FTLM_CLIENT_sendFrame(pxCTX, pFrame);	
		}
		break;

	case	FTLM_CMD_GROUP_CTRL:
		{
			int	i;
			int	nGroups = ((FTLM_GROUP_CTRL_PARAM_PTR)pFrame->pReqParam)->nGroups;
			FTLM_GROUP_CTRL_PTR	pGroups = ((FTLM_GROUP_CTRL_PARAM_PTR)pFrame->pReqParam)->pGroups;

			for(i = 0 ; i < nGroups; i++)
			{
				FTLM_GROUP_set(pGroups[i].nID, pGroups[i].nCmd, pGroups[i].nLevel, pGroups[i].nDimmingTime);
			}
		}
		break;

	case	FTLM_CMD_LIGHT_CTRL:
		{
			int	i;

			FTLM_LIGHT_CTRL_PARAM_PTR pParam = (FTLM_LIGHT_CTRL_PARAM_PTR)pFrame->pReqParam;

			for(i = 0 ; i < pParam->nLights ; i++)
			{
				FTLM_LIGHT_set(pParam->pLights[i].nID, pParam->pLights[i].nCmd, pParam->pLights[i].nLevel, pParam->pLights[i].nDulationTime);	
			}
		}
		break;

	case	FTLM_CMD_GROUP_SET:
		{
			int	i;
			unsigned char nSets = ((FTLM_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
			FTLM_GROUP_SET_PTR	pSet = ((FTLM_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

			for(i = 0 ; i < nSets ; i++)
			{
				int	j;

				printf("%8s : %d\n", "ID",	pSet->nID);
				for(j = 0 ; j < pSet->nGroups ; j++)
				{
					printf("    %4d : %d\n", j+1, pSet->pGroups[j]);
				}

				pSet = (FTLM_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
			}
		}
		break;

	case	FTLM_CMD_SWITCH_GROUP_SET:
		{
			int	i;
			unsigned char nSets = ((FTLM_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
			FTLM_SWITCH_GROUP_SET_PTR	pSet = ((FTLM_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

			for(i = 0 ; i < nSets ; i++)
			{
				int	j;

				printf("%8s : %d\n", "ID",	pSet->nID);
				for(j = 0 ; j < pSet->nGroups ; j++)
				{
					printf("    %4d : %d\n", j+1, pSet->pGroups[j]);
				}

				pSet = (FTLM_SWITCH_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
			}
		}
		break;

	case FTLM_CMD_GROUP_MAPPING_GET:
		{
			FTLM_LIGHT_CFG_PTR				pLight;
			FTLM_GROUP_MAPPING_PARAM_PTR	pParam = (FTLM_GROUP_MAPPING_PARAM_PTR)pFrame->pRespParam;
			FTLM_GROUP_MAPPING_PTR		pMapping = pParam->pSets;

			pFrame->nRespParamLen = 1;

			pParam->nSets = 0;
			FTM_LIST_iteratorStart(xCfg.pLightList);
			while(FTM_LIST_iteratorNext(xCfg.pLightList, (void **)&pLight) == FTM_RET_OK)
			{
				FTM_ULONG	i;
				FTM_ULONG	ulAllGroupCount;

				pMapping->nID 		= pLight->xID;
				pMapping->nGroups	= 0;
				FTM_LIST_count(xCfg.pGroupList, &ulAllGroupCount);
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
				pMapping = (FTLM_GROUP_MAPPING_PTR)(((unsigned char *)pMapping) + sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups);
				pFrame->nRespParamLen += sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups;
			}
			FTLM_CLIENT_sendFrame(pxCTX, pFrame);	
		}
		break;

	case FTLM_CMD_SWITCH_MAPPING_GET:
		{
			FTLM_SWITCH_CFG_PTR				pSwitch;
			FTLM_GROUP_MAPPING_PARAM_PTR	pParam = (FTLM_GROUP_MAPPING_PARAM_PTR)pFrame->pRespParam;
			FTLM_GROUP_MAPPING_PTR		pMapping = pParam->pSets;

			pFrame->nRespParamLen = 1;

			pParam->nSets = 0;
			FTM_LIST_iteratorStart(xCfg.pSwitchList);
			while(FTM_LIST_iteratorNext(xCfg.pSwitchList, (void **)&pSwitch) == FTM_RET_OK)
			{
				FTM_ULONG	i;
				FTM_ULONG	ulCount;

				pMapping->nGroups = 0;

				FTM_LIST_count(pSwitch->pGroupList, &ulCount);
				for(i = 0 ; i < ulCount ; i++)
				{
					FTLM_GROUP_CFG_PTR pGroup;

					if (FTM_LIST_getAt(xCfg.pGroupList, i, (FTM_VOID_PTR _PTR_)&pGroup) == FTM_RET_OK)
					{
						pMapping->pGroups[pMapping->nGroups++] = pGroup->xID;
					}
				}

				pParam->nSets++;
				pMapping = (FTLM_GROUP_MAPPING_PTR)(((unsigned char *)pMapping) + sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups);
				pFrame->nRespParamLen += sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups;
			}
			FTLM_CLIENT_sendFrame(pxCTX, pFrame);	
		}
		break;

	case FTLM_CMD_GROUP_STATUS_GET:
		{
			FTLM_GROUP_CFG_PTR				pGroup;
			FTLM_GROUP_STATUS_PARAM_PTR	pParam = (FTLM_GROUP_STATUS_PARAM_PTR)pFrame->pRespParam;

			pParam->nGroups = 0;
			FTM_LIST_iteratorStart(xCfg.pGroupList);
			TRACE("%8s %8s %8s %8s\n", "ID", "STATUS", "LEVEL", "DIMMING");
			while(FTM_LIST_iteratorNext(xCfg.pGroupList, (void **)&pGroup) == FTM_RET_OK)
			{
				TRACE("%08x %8d %8lu %16lu\n", (unsigned int)pGroup->xID, pGroup->xStatus, pGroup->ulLevel, pGroup->ulTime); 
				pParam->pGroups[pParam->nGroups].nID 			= pGroup->xID;
				pParam->pGroups[pParam->nGroups].nStatus		= pGroup->xStatus;
				pParam->pGroups[pParam->nGroups].nLevel			= pGroup->ulLevel;
				pParam->pGroups[pParam->nGroups].nDimmingTime 	= pGroup->ulTime;
				pParam->nGroups++;
			}

			pFrame->nRespParamLen = sizeof(FTLM_GROUP_STATUS_PARAM) + sizeof(FTLM_GROUP_STATUS) * pParam->nGroups;
			FTLM_CLIENT_sendFrame(pxCTX, pFrame);	
		}
		break;

	case FTLM_CMD_LIGHT_STATUS_GET:
		{
			FTLM_LIGHT_CFG_PTR	pLight;
			FTLM_LIGHT_STATUS_PARAM_PTR	pParam = (FTLM_LIGHT_STATUS_PARAM_PTR)pFrame->pRespParam;

			pParam->nLights = 0;
			FTM_LIST_iteratorStart(xCfg.pLightList);
			TRACE("%8s %8s %8s %8s\n", "ID", "STATUS", "LEVEL", "DULATION");
			while(FTM_LIST_iteratorNext(xCfg.pLightList, (void **)&pLight) == FTM_RET_OK)
			{
				TRACE("%08x %8d %8lu %16lu\n", (unsigned int)pLight->xID, pLight->xStatus, pLight->ulLevel, pLight->ulTime); 
				pParam->pLights[pParam->nLights].nID 			= pLight->xID;
				pParam->pLights[pParam->nLights].nStatus		= pLight->xStatus;
				pParam->pLights[pParam->nLights].nLevel			= pLight->ulLevel;
				pParam->pLights[pParam->nLights].nDulationTime 	= pLight->ulTime;
				pParam->nLights++;
			}

			pFrame->nRespParamLen = sizeof(FTLM_LIGHT_STATUS_PARAM) + sizeof(FTLM_LIGHT_STATUS) * pParam->nLights;
			FTLM_CLIENT_sendFrame(pxCTX, pFrame);	
		}
	}

	pFrame->nRet = FTM_RET_OK;
	return	FTM_RET_OK;
}

FTM_RET FTLM_CMD_groupCtrl(int nGroupNum, int nCmd, int nLevel, int nDimmingTime)
{

	return	FTM_RET_OK;

}


FTM_VOID    _showUsage(FTM_CHAR_PTR pAppName)
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

