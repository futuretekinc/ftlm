#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm.h"
#include "ftm_mem.h"
#include "ftlm_config.h"
#include "ftlm_client.h"
#include "ftlm_mqtt.h"
#include "ftlm_msg.h"
#include "nxjson.h"

static void 	FTLM_subscribe(struct mosquitto *pMOSQ, void *pObj, const struct mosquitto_message *pMessage);
static FTM_RET	FTLM_recv(void *pObj, void *pParam);
static FTM_RET 	FTLM_GROUP_set(FTLM_ID nID, unsigned char nCmd, unsigned char nLevel, unsigned char nDimmingTime);
static FTM_RET	FTLM_LIGHT_set(FTLM_ID nID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime);

FTM_MQTT_CONFIG	xMQTTConfig = 
{
	
	.pClientID 	= "test",
	.pBrokerIP	= "127.0.0.1",
	.usPort 	= 1883,
	.nKeepAlive = 60,

	.CB_message	= FTLM_subscribe,
};

FTLM_CONFIG xLCCConfig = 
{
	.pServerIP	= "10.0.1.100",
	.usPort 	= 9877,

	.CB_recv	= FTLM_recv
};

static FTLM_PTR	pLCC = NULL;
static FTM_MQTT_PTR	pMQTT = NULL;	

static FTLM_CFG	xCfg;

int main(int nArg, char *pArgs[])
{
	FTM_MEM_init();
	FTLM_CFG_init(&xCfg);

	FTLM_CFG_load(&xCfg, "./lcgw.conf");

	FTLM_CFG_print(&xCfg);
	
	FTM_DEBUG_printModeSet(MSG_ALL);

	pMQTT = FTM_MQTT_create(&xMQTTConfig);
	if (pMQTT == NULL)
	{
		return	0;
	}

	strncpy(xLCCConfig.pServerIP, xCfg.xNetwork.pServerIP, FTLM_SERVER_IP_LEN);	
	xLCCConfig.usPort = xCfg.xNetwork.usPort;
	pLCC = FTLM_create(&xLCCConfig);
	if (pLCC == NULL)
	{
		FTM_MQTT_destroy(pMQTT);
		return	0;
	}

	FTM_MQTT_start(pMQTT);
	FTLM_start(pLCC);

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
	FTLM_PTR 		pLCC = (FTLM_PTR)pObj;
	FTLM_FRAME_PTR 	pFrame = (FTLM_FRAME_PTR)pParam;

	switch(pFrame->nCmd)
	{
	case	FTLM_CMD_RESET:
		{
			pFrame->nRet = 0;
			FTLM_sendFrame(pLCC, pFrame);	
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
			FTLM_LIGHT_PTR				pLight;
			FTLM_GROUP_MAPPING_PARAM_PTR	pParam = (FTLM_GROUP_MAPPING_PARAM_PTR)pFrame->pRespParam;
			FTLM_GROUP_MAPPING_PTR		pMapping = pParam->pSets;

			pFrame->nRespParamLen = 1;

			pParam->nSets = 0;
			FTM_LIST_iteratorStart(xCfg.pLightList);
			while(FTM_LIST_iteratorNext(xCfg.pLightList, (void **)&pLight) == FTM_RET_OK)
			{
				pMapping->nID 		= pLight->nID;
				pMapping->nGroups 	= pLight->nGroupIDs;
				memcpy(pMapping->pGroups, pLight->pGroupIDs, pLight->nGroupIDs);

				pParam->nSets++;
				pMapping = (FTLM_GROUP_MAPPING_PTR)(((unsigned char *)pMapping) + sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups);
				pFrame->nRespParamLen += sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups;
			}
			FTLM_sendFrame(pLCC, pFrame);	
		}
		break;

	case FTLM_CMD_SWITCH_MAPPING_GET:
		{
			FTLM_SWITCH_PTR				pSwitch;
			FTLM_GROUP_MAPPING_PARAM_PTR	pParam = (FTLM_GROUP_MAPPING_PARAM_PTR)pFrame->pRespParam;
			FTLM_GROUP_MAPPING_PTR		pMapping = pParam->pSets;

			pFrame->nRespParamLen = 1;

			pParam->nSets = 0;
			FTM_LIST_iteratorStart(xCfg.pSwitchList);
			while(FTM_LIST_iteratorNext(xCfg.pSwitchList, (void **)&pSwitch) == FTM_RET_OK)
			{
				pMapping->nID 		= pSwitch->nID;
				pMapping->nGroups 	= pSwitch->nGroupIDs;
				memcpy(pMapping->pGroups, pSwitch->pGroupIDs, pSwitch->nGroupIDs);

				pParam->nSets++;
				pMapping = (FTLM_GROUP_MAPPING_PTR)(((unsigned char *)pMapping) + sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups);
				pFrame->nRespParamLen += sizeof(FTLM_GROUP_MAPPING) + pMapping->nGroups;
			}
			FTLM_sendFrame(pLCC, pFrame);	
		}
		break;

	case FTLM_CMD_GROUP_STATUS_GET:
		{
			FTLM_GROUP_PTR				pGroup;
			FTLM_GROUP_STATUS_PARAM_PTR	pParam = (FTLM_GROUP_STATUS_PARAM_PTR)pFrame->pRespParam;

			pParam->nGroups = 0;
			FTM_LIST_iteratorStart(xCfg.pGroupList);
			printf("%8s %8s %8s %8s\n", "ID", "STATUS", "LEVEL", "DIMMING");
			while(FTM_LIST_iteratorNext(xCfg.pGroupList, (void **)&pGroup) == FTM_RET_OK)
			{
				printf("%8d %8d %8d %16d\n", pGroup->nID, pGroup->nStatus, pGroup->nLevel, pGroup->nDimmingTime); 
				pParam->pGroups[pParam->nGroups].nID 			= pGroup->nID;
				pParam->pGroups[pParam->nGroups].nStatus		= pGroup->nStatus;
				pParam->pGroups[pParam->nGroups].nLevel			= pGroup->nLevel;
				pParam->pGroups[pParam->nGroups].nDimmingTime 	= pGroup->nDimmingTime;
				pParam->nGroups++;
			}

			pFrame->nRespParamLen = sizeof(FTLM_GROUP_STATUS_PARAM) + sizeof(FTLM_GROUP_STATUS) * pParam->nGroups;
			FTLM_sendFrame(pLCC, pFrame);	
		}
		break;

	case FTLM_CMD_LIGHT_STATUS_GET:
		{
			FTLM_LIGHT_PTR	pLight;
			FTLM_LIGHT_STATUS_PARAM_PTR	pParam = (FTLM_LIGHT_STATUS_PARAM_PTR)pFrame->pRespParam;

			pParam->nLights = 0;
			FTM_LIST_iteratorStart(xCfg.pLightList);
			printf("%8s %8s %8s %8s\n", "ID", "STATUS", "LEVEL", "DULATION");
			while(FTM_LIST_iteratorNext(xCfg.pLightList, (void **)&pLight) == FTM_RET_OK)
			{
				printf("%8d %8d %8d %16d\n", pLight->nID, pLight->nStatus, pLight->nLevel, pLight->nDulationTime); 
				pParam->pLights[pParam->nLights].nID 			= pLight->nID;
				pParam->pLights[pParam->nLights].nStatus		= pLight->nStatus;
				pParam->pLights[pParam->nLights].nLevel			= pLight->nLevel;
				pParam->pLights[pParam->nLights].nDulationTime 	= pLight->nDulationTime;
				pParam->nLights++;
			}

			pFrame->nRespParamLen = sizeof(FTLM_LIGHT_STATUS_PARAM) + sizeof(FTLM_LIGHT_STATUS) * pParam->nLights;
			FTLM_sendFrame(pLCC, pFrame);	
		}
	}

	pFrame->nRet = FTM_RET_OK;
	return	FTM_RET_OK;
}

FTM_RET FTLM_GROUP_set(FTLM_ID nID, unsigned char nCmd, unsigned char nLevel, unsigned char nDimmingTime)
{
	FTLM_LIGHT_PTR pLight;

	FTM_LIST_iteratorStart(xCfg.pLightList);
	while(FTM_LIST_iteratorNext(xCfg.pLightList, (void **)&pLight) == FTM_RET_OK)
	{
		int	i;
		for(i = 0 ; i < pLight->nGroupIDs; i++)
		{
			if (pLight->pGroupIDs[i] == nID)
			{
				pLight->nStatus			= nCmd;
				pLight->nLevel			= nLevel;
				pLight->nDulationTime 	= nDimmingTime;
				break;
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_LIGHT_set(FTLM_ID nID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime)
{
	FTLM_LIGHT_PTR pLight;
	char	pTopic[256];
	char	pMessage[256];
	int		nMessage;

	pLight = FTLM_CFG_LIGHT_get(&xCfg, nID);
	if (pLight == NULL)
	{
		printf("Can't find light[%d]\n", nID);
		return	FTM_RET_ERROR;	
	}

	sprintf(pTopic, "ftm2m/%s/%s/lc", pLight->pNodeID, "0");
	nMessage = sprintf(pMessage, "{\"id\":%d, \"cmd\":%d, \"level\":%d, \"dulation\":%d}", nID, nCmd, nLevel, nDulationTime);

	FTM_MQTT_publish(pMQTT, pTopic, pMessage, nMessage, 0);

	pLight->nStatus			= nCmd;
	pLight->nLevel			= nLevel;
	pLight->nDulationTime 	= nDulationTime;

	return	FTM_RET_OK;
}

FTM_RET FTLM_CMD_groupCtrl(int nGroupNum, int nCmd, int nLevel, int nDimmingTime)
{
	char	pBuff[1024];
	int		nBuff = 0;

	nBuff += snprintf(&pBuff[nBuff], sizeof(pBuff) - nBuff, "{");


	nBuff += snprintf(&pBuff[nBuff], sizeof(pBuff) - nBuff, "}");


	return	FTM_RET_OK;

}
