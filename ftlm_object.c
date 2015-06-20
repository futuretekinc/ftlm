#include <string.h>
#include "ftlm_object.h"
#include "ftm_list.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftm_mqtt.h"

static	FTM_LIST_PTR	pLightList;
static	FTM_LIST_PTR	pGroupList;
static	FTM_LIST_PTR	pSwitchList;

static FTM_INT	FTLM_OBJ_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTLM_OBJ_comparator(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTLM_OBJ_init(void)
{
	pLightList = FTM_LIST_create();
	FTM_LIST_setSeeker(pLightList, FTLM_OBJ_seeker);
	FTM_LIST_setComparator(pLightList, FTLM_OBJ_comparator);
	pGroupList = FTM_LIST_create();
	FTM_LIST_setSeeker(pSwitchList, FTLM_OBJ_seeker);
	FTM_LIST_setComparator(pSwitchList, FTLM_OBJ_comparator);
	pSwitchList = FTM_LIST_create();
	FTM_LIST_setSeeker(pSwitchList, FTLM_OBJ_seeker);
	FTM_LIST_setComparator(pSwitchList, FTLM_OBJ_comparator);

	return	FTM_RET_OK;
}

FTLM_LIGHT_PTR	FTLM_LIGHT_create(FTLM_LIGHT_CFG_PTR pConfig)
{
	FTLM_LIGHT_PTR	pLight;

	if (pConfig == NULL)
	{
		return	NULL;
	}

	pLight = (FTLM_LIGHT_PTR)FTM_MEM_malloc(sizeof(FTLM_LIGHT));
	if (pLight == NULL)
	{
		return	NULL;
	}
	memset(pLight, 0, sizeof(FTLM_LIGHT));

	pLight->xCommon.xID = pConfig->xID;
	pLight->pConfig = pConfig;

	FTM_LIST_append(pLightList, pLight);

	return	pLight;
}

FTLM_GROUP_PTR	FTLM_GROUP_create(FTLM_GROUP_CFG_PTR pConfig)
{
	FTLM_GROUP_PTR	pGroup;

	if (pConfig == NULL)
	{
		return	NULL;
	}

	pGroup = (FTLM_GROUP_PTR)FTM_MEM_malloc(sizeof(FTLM_GROUP));
	if (pGroup == NULL)
	{
		return	NULL;
	}
	memset(pGroup, 0, sizeof(FTLM_GROUP));

	pGroup->xCommon.xID = pConfig->xID;
	pGroup->pConfig = pConfig;

	FTM_LIST_append(pGroupList, pGroup);

	return	pGroup;
}

FTLM_SWITCH_PTR	FTLM_SWITCH_create(FTLM_SWITCH_CFG_PTR pConfig)
{
	FTLM_SWITCH_PTR	pSwitch;

	if (pConfig == NULL)
	{
		return	NULL;
	}

	pSwitch = (FTLM_SWITCH_PTR)FTM_MEM_malloc(sizeof(FTLM_SWITCH));
	if (pSwitch == NULL)
	{
		return	NULL;
	}
	memset(pSwitch, 0, sizeof(FTLM_SWITCH));

	pSwitch->xCommon.xID = pConfig->xID;
	pSwitch->pConfig = pConfig;

	FTM_LIST_append(pSwitchList, pSwitch);

	return	pSwitch;
}

FTM_RET FTLM_GROUP_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDimmingTime)
{
	FTLM_GROUP_PTR 	pGroup;
	FTM_ID			xLightID;

	if (FTM_LIST_get(pGroupList, (FTM_VOID_PTR)&xID, (FTM_VOID_PTR _PTR_)&pGroup) != FTM_RET_OK)
	{
		TRACE("Can't find group[%08x]\n", (unsigned int)xID);
		return	FTM_RET_ERROR;	
	}

	FTM_LIST_iteratorStart(pGroup->pConfig->pLightList);
	while(FTM_LIST_iteratorNext(pGroup->pConfig->pLightList, (FTM_VOID_PTR _PTR_)&xLightID) == FTM_RET_OK)
	{
		FTLM_LIGHT_set(xLightID, nCmd, nLevel, nDimmingTime);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_LIGHT_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime)
{
	FTLM_LIGHT_PTR pLight;
	char	pTopic[256];
	char	pMessage[256];
	int		nMessage;

	if (FTM_LIST_get(pLightList, (FTM_VOID_PTR)&xID, (FTM_VOID_PTR _PTR_)&pLight) != FTM_RET_OK)
	{
		TRACE("Can't find light[%08x]\n", (unsigned int)xID);
		return	FTM_RET_ERROR;	
	}

	sprintf(pTopic, "/v/a/g/%s/s/%08x/req", pLight->pConfig->pGatewayID, (unsigned int)xID);
	nMessage = sprintf(pMessage, "{\"cmd\":%d, \"level\":%d, \"dulation\":%d}", nCmd, nLevel, nDulationTime);

	//FTM_MQTT_publish(pMQTT, pTopic, pMessage, nMessage, 0);

	pLight->xStatus	= nCmd;
	pLight->ulLevel	= nLevel;
	pLight->ulTime	= nDulationTime;

	return	FTM_RET_OK;
}

FTM_INT	FTLM_OBJ_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT((pElement != NULL) && (pIndicator != NULL));

	return	(((FTLM_OBJECT_PTR)pElement)->xID == *(FTM_ID_PTR)pIndicator);
}

FTM_INT	FTLM_OBJ_comparator(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT((pElement != NULL) && (pIndicator != NULL));

	if (((FTLM_OBJECT_PTR)pElement)->xID < ((FTLM_OBJECT_PTR)pIndicator)->xID)
	{
		return	-1;	
	}
	else if (((FTLM_OBJECT_PTR)pElement)->xID > ((FTLM_OBJECT_PTR)pIndicator)->xID)
	{
		return	1;	
	}

	return	0;
}

