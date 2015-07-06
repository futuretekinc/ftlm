#include <string.h>
#include "ftlm_object.h"
#include "ftm_list.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftm_mqtt.h"
#include "ftlm.h"

static	FTM_LIST_PTR	pLightList;
static	FTM_LIST_PTR	pGroupList;
static	FTM_LIST_PTR	pSwitchList;

static FTM_INT	FTLM_ID_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTLM_OBJ_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTLM_OBJ_comparator(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTLM_OBJ_init(FTLM_CFG_PTR pConfig)
{
	FTM_ULONG		i, ulCount;

	pLightList = FTM_LIST_create();
	FTM_LIST_setSeeker(pLightList, FTLM_OBJ_seeker);
	FTM_LIST_setComparator(pLightList, FTLM_OBJ_comparator);

	pGroupList = FTM_LIST_create();
	FTM_LIST_setSeeker(pGroupList, FTLM_OBJ_seeker);
	FTM_LIST_setComparator(pGroupList, FTLM_OBJ_comparator);
	
	pSwitchList = FTM_LIST_create();
	FTM_LIST_setSeeker(pSwitchList, FTLM_OBJ_seeker);
	FTM_LIST_setComparator(pSwitchList, FTLM_OBJ_comparator);

	ulCount = FTLM_CFG_LIGHT_count(pConfig);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_LIGHT_CFG_PTR	pLightCfg = FTLM_CFG_LIGHT_getAt(pConfig, i);

		if (pLightCfg != NULL)
		{
			FTLM_LIGHT_create(pLightCfg);
		}
	}

	ulCount = FTLM_CFG_GROUP_count(pConfig);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_GROUP_CFG_PTR	pGroupCfg = FTLM_CFG_GROUP_getAt(pConfig, i);

		if (pGroupCfg != NULL)
		{
			FTLM_GROUP_create(pGroupCfg);
		}
	}

	ulCount = FTLM_CFG_SWITCH_count(pConfig);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_SWITCH_CFG_PTR	pSwitchCfg = FTLM_CFG_SWITCH_getAt(pConfig, i);

		if (pSwitchCfg != NULL)
		{
			FTLM_SWITCH_create(pSwitchCfg);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTLM_OBJ_final(void)
{
	FTLM_LIGHT_PTR	pLight;
	FTLM_GROUP_PTR	pGroup;
	FTLM_SWITCH_PTR	pSwitch;
	FTM_ULONG		i, ulCount;

	ulCount = FTM_LIST_count(pLightList);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_LIST_getAt(pLightList, i, (FTM_VOID_PTR _PTR_)&pLight) == FTM_RET_OK)
		{
			FTM_MEM_free(pLight);	
		}
	}
	FTM_LIST_destroy(pLightList);

	ulCount = FTM_LIST_count(pGroupList);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_LIST_getAt(pGroupList, i, (FTM_VOID_PTR _PTR_)&pGroup) == FTM_RET_OK)
		{
			FTM_MEM_free(pGroup);	
		}
	}
	FTM_LIST_destroy(pGroupList);

	ulCount = FTM_LIST_count(pSwitchList);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_LIST_getAt(pSwitchList, i, (FTM_VOID_PTR _PTR_)&pSwitch) == FTM_RET_OK)
		{
			FTM_MEM_free(pSwitch);	
		}
	}
	FTM_LIST_destroy(pSwitchList);

	return	FTM_RET_OK;
}

FTM_RET	FTLM_OBJ_save(FTLM_CFG_PTR pConfig)
{
	FTM_ULONG		i, ulCount;

	ulCount = FTLM_OBJ_getLightCount();
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_LIGHT_PTR	pLight = FTLM_OBJ_getLightAt(i);
		FTLM_LIGHT_CFG_PTR	pLightCfg = FTLM_CFG_LIGHT_get(pConfig, pLight->xCommon.xID);
		if (pLightCfg == NULL)
		{
			pLightCfg = FTLM_CFG_LIGHT_create(pConfig, pLight->xCommon.xID);
			if (pLightCfg == NULL)
			{
				goto error;	
			}
		}

		pLightCfg->xID = pLight->xCommon.xID;
		strcpy(pLightCfg->pName, pLight->xCommon.pName);
		pLightCfg->ulCmd 	= pLight->ulCmd;
		pLightCfg->ulLevel 	= pLight->ulLevel;
		pLightCfg->ulTime 	= pLight->ulTime;
	}

	ulCount = FTLM_OBJ_getGroupCount();
	for(i = 0 ; i < ulCount ; i++)
	{
		int	j;

		FTLM_GROUP_PTR	pGroup = FTLM_OBJ_getGroupAt(i);
		FTLM_GROUP_CFG_PTR	pGroupCfg = FTLM_CFG_GROUP_get(pConfig, pGroup->xCommon.xID);
		if (pGroupCfg == NULL)
		{
			pGroupCfg = FTLM_CFG_GROUP_create(pConfig, pGroup->xCommon.xID);
			if (pGroupCfg == NULL)
			{
				goto error;	
			}
		}

		pGroupCfg->xID = pGroup->xCommon.xID;
		strcpy(pGroupCfg->pName, pGroup->xCommon.pName);
		pGroupCfg->ulCmd 	= pGroup->ulCmd;
		pGroupCfg->ulLevel 	= pGroup->ulLevel;
		pGroupCfg->ulTime 	= pGroup->ulTime;

		for(j = 0 ; j < FTM_LIST_count(pGroup->pLightList) ; j++)
		{
			FTM_ID	xLightID;

			if (FTM_LIST_getAt(pGroup->pLightList, j, (FTM_VOID_PTR _PTR_)&xLightID) == FTM_RET_OK)
			{
				if (FTM_LIST_get(pGroupCfg->pLightList, (FTM_VOID_PTR)xLightID, NULL) != FTM_RET_OK)
				{
					FTM_LIST_append(pGroupCfg->pLightList, (FTM_VOID_PTR)xLightID);	
				}
			}
		}
	}


	ulCount = FTLM_OBJ_getSwitchCount();
	for(i = 0 ; i < ulCount ; i++)
	{
		int	j;
		FTLM_SWITCH_PTR	pSwitch = FTLM_OBJ_getSwitchAt(i);
		FTLM_SWITCH_CFG_PTR	pSwitchCfg = FTLM_CFG_SWITCH_get(pConfig, pSwitch->xCommon.xID);
		if (pSwitchCfg == NULL)
		{
			pSwitchCfg = FTLM_CFG_SWITCH_create(pConfig, pSwitch->xCommon.xID);
			if (pSwitchCfg == NULL)
			{
				goto error;	
			}
		}

		pSwitchCfg->xID = pSwitch->xCommon.xID;
		strcpy(pSwitchCfg->pName, pSwitch->xCommon.pName);

		for(j = 0 ; j < FTM_LIST_count(pSwitch->pGroupList) ; j++)
		{
			FTM_ID	xGroupID;

			if (FTM_LIST_getAt(pSwitch->pGroupList, j, (FTM_VOID_PTR _PTR_)&xGroupID) == FTM_RET_OK)
			{
				if (FTM_LIST_get(pSwitchCfg->pGroupList, (FTM_VOID_PTR)xGroupID, NULL) != FTM_RET_OK)
				{
					FTM_LIST_append(pSwitchCfg->pGroupList, (FTM_VOID_PTR)xGroupID);	
				}
			}
		}
	}


	
	FTLM_CFG_LIGHT_count(pConfig);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_LIGHT_CFG_PTR	pLightCfg = FTLM_CFG_LIGHT_getAt(pConfig, i);

		if (pLightCfg != NULL)
		{
			FTLM_LIGHT_create(pLightCfg);
		}
	}

	ulCount = FTLM_CFG_GROUP_count(pConfig);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_GROUP_CFG_PTR	pGroupCfg = FTLM_CFG_GROUP_getAt(pConfig, i);

		if (pGroupCfg != NULL)
		{
			FTLM_GROUP_create(pGroupCfg);
		}
	}

	ulCount = FTLM_CFG_SWITCH_count(pConfig);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTLM_SWITCH_CFG_PTR	pSwitchCfg = FTLM_CFG_SWITCH_getAt(pConfig, i);

		if (pSwitchCfg != NULL)
		{
			FTLM_SWITCH_create(pSwitchCfg);
		}
	}

	return	FTM_RET_OK;
error:
	return	FTM_RET_ERROR;
}

FTLM_LIGHT_PTR	FTLM_LIGHT_create(FTLM_LIGHT_CFG_PTR pConfig)
{
	FTLM_LIGHT_PTR	pLight;

	ASSERT((pConfig != NULL) && (pLightList != NULL));

	pLight = (FTLM_LIGHT_PTR)FTM_MEM_malloc(sizeof(FTLM_LIGHT));
	if (pLight == NULL)
	{
		return	NULL;
	}
	memset(pLight, 0, sizeof(FTLM_LIGHT));

	pLight->xCommon.xID = pConfig->xID;
	strcpy(pLight->xCommon.pName, pConfig->pName); 
	strcpy(pLight->pGatewayID, pConfig->pGatewayID);

	pLight->ulCmd	= pConfig->ulCmd;
	pLight->ulLevel = pConfig->ulLevel;
	pLight->ulTime	= pConfig->ulTime;

	FTM_LIST_append(pLightList, pLight);

	return	pLight;
}

FTM_RET	FTLM_LIGHT_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime)
{
	FTLM_LIGHT_PTR	pLight;

	pLight = FTLM_OBJ_getLight(xID);
	if (pLight == NULL)
	{
		return	FTM_RET_ERROR;	
	}

	FTLM_lightCtrl(pLight, (FTM_ULONG)nCmd, (FTM_ULONG)nLevel, (FTM_ULONG) nDulationTime);

	pLight->ulCmd	= nCmd;
	pLight->ulLevel = nLevel;
	pLight->ulTime  = nDulationTime;

	return	FTM_RET_OK;
}


FTLM_GROUP_PTR	FTLM_GROUP_create(FTLM_GROUP_CFG_PTR pConfig)
{
	FTM_INT			i;
	FTLM_GROUP_PTR	pGroup;

	ASSERT((pConfig != NULL) && (pGroupList != NULL));

	pGroup = (FTLM_GROUP_PTR)FTM_MEM_malloc(sizeof(FTLM_GROUP));
	if (pGroup == NULL)
	{
		return	NULL;
	}
	memset(pGroup, 0, sizeof(FTLM_GROUP));

	pGroup->xCommon.xID = pConfig->xID;
	strcpy(pGroup->xCommon.pName, pConfig->pName); 

	pGroup->ulCmd = pConfig->ulCmd;
	pGroup->ulLevel = pConfig->ulLevel;
	pGroup->ulTime	= pConfig->ulTime;

	pGroup->pLightList = FTM_LIST_create();
	FTM_LIST_setSeeker(pGroup->pLightList, FTLM_ID_seeker);

	for(i = 0 ; i < FTM_LIST_count(pConfig->pLightList); i++)
	{
		FTM_ID	xLightID;
		if (FTM_LIST_getAt(pConfig->pLightList, i, (FTM_VOID_PTR)&xLightID) == FTM_RET_OK)
		{
			FTM_LIST_append(pGroup->pLightList, (FTM_VOID_PTR)xLightID);	
		}
	}

	FTM_LIST_append(pGroupList, pGroup);

	return	pGroup;
}

FTM_RET	FTLM_GROUP_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime)
{
	FTLM_GROUP_PTR	pGroup;

	pGroup = FTLM_OBJ_getGroup(xID);
	if (pGroup == NULL)
	{
		return	FTM_RET_ERROR;	
	}

	pGroup->ulCmd	= nCmd;
	pGroup->ulLevel = nLevel;
	pGroup->ulTime  = nDulationTime;

	return	FTM_RET_OK;
}

FTLM_SWITCH_PTR	FTLM_SWITCH_create(FTLM_SWITCH_CFG_PTR pConfig)
{
	FTM_INT			i;
	FTLM_SWITCH_PTR	pSwitch;

	ASSERT((pConfig != NULL) && (pSwitchList != NULL));

	pSwitch = (FTLM_SWITCH_PTR)FTM_MEM_malloc(sizeof(FTLM_SWITCH));
	if (pSwitch == NULL)
	{
		return	NULL;
	}
	memset(pSwitch, 0, sizeof(FTLM_SWITCH));

	pSwitch->xCommon.xID = pConfig->xID;
	strcpy(pSwitch->xCommon.pName, pConfig->pName); 

	pSwitch->pGroupList = FTM_LIST_create();
	FTM_LIST_setSeeker(pSwitch->pGroupList, FTLM_ID_seeker);

	for(i = 0 ; i < FTM_LIST_count(pConfig->pGroupList); i++)
	{
		FTM_ID	xGroupID;
		if (FTM_LIST_getAt(pConfig->pGroupList, i, (FTM_VOID_PTR)&xGroupID) == FTM_RET_OK)
		{
			FTM_LIST_append(pSwitch->pGroupList, (FTM_VOID_PTR)xGroupID);	
		}
	}

	FTM_LIST_append(pSwitchList, pSwitch);

	return	pSwitch;
}

FTM_ULONG	FTLM_OBJ_getLightCount(void)
{
	return	FTM_LIST_count(pLightList);
}

FTLM_LIGHT_PTR FTLM_OBJ_getLight(FTM_ID xID)
{
	FTLM_LIGHT_PTR	pLight = NULL;

	if (FTM_LIST_get(pLightList, (FTM_VOID_PTR)xID, (FTM_VOID_PTR _PTR_)&pLight) != FTM_RET_OK)
	{
		TRACE("Can't find light[%08x]\n", (unsigned int)xID);
		return	NULL;	
	}

	return	pLight;
}

FTLM_LIGHT_PTR FTLM_OBJ_getLightAt(FTM_ULONG ulIndex)
{
	FTLM_LIGHT_PTR	pLight = NULL;

	if (FTM_LIST_getAt(pLightList, ulIndex, (FTM_VOID_PTR _PTR_)&pLight) != FTM_RET_OK)
	{
		TRACE("Can't find light at %lu\n", ulIndex);
		return	NULL;	
	}

	return	pLight;
}

FTM_ULONG	FTLM_OBJ_getGroupCount(void)
{
	return	FTM_LIST_count(pGroupList);
}

FTLM_GROUP_PTR FTLM_OBJ_getGroup(FTM_ID xID)
{
	FTLM_GROUP_PTR	pGroup = NULL;

	if (FTM_LIST_get(pGroupList, (FTM_VOID_PTR)xID, (FTM_VOID_PTR _PTR_)&pGroup) != FTM_RET_OK)
	{
		TRACE("Can't find group[%08x]\n", (unsigned int)xID);
		return	NULL;	
	}

	return	pGroup;
}

FTLM_GROUP_PTR FTLM_OBJ_getGroupAt(FTM_ULONG ulIndex)
{
	FTLM_GROUP_PTR	pGroup = NULL;

	if (FTM_LIST_getAt(pGroupList, ulIndex, (FTM_VOID_PTR _PTR_)&pGroup) != FTM_RET_OK)
	{
		TRACE("Can't find group at %d\n", ulIndex);
		return	NULL;	
	}

	return	pGroup;
}

FTM_ULONG	FTLM_OBJ_getSwitchCount(void)
{
	return	FTM_LIST_count(pSwitchList);
}

FTLM_SWITCH_PTR FTLM_OBJ_getSwitch(FTM_ID xID)
{
	FTLM_SWITCH_PTR	pSwitch = NULL;

	if (FTM_LIST_get(pSwitchList, (FTM_VOID_PTR)xID, (FTM_VOID_PTR _PTR_)&pSwitch) != FTM_RET_OK)
	{
		TRACE("Can't find switch[%08x]\n", (unsigned int)xID);
		return	NULL;	
	}

	return	pSwitch;
}

FTLM_SWITCH_PTR FTLM_OBJ_getSwitchAt(FTM_ULONG ulIndex)
{
	FTLM_SWITCH_PTR	pSwitch = NULL;

	if (FTM_LIST_getAt(pSwitchList, ulIndex, (FTM_VOID_PTR _PTR_)&pSwitch) != FTM_RET_OK)
	{
		TRACE("Can't find switch at %d\n", ulIndex);
		return	NULL;	
	}

	return	pSwitch;
}

FTM_ULONG	FTLM_GROUP_getLightCount(FTLM_GROUP_PTR pGroup)
{
	ASSERT(pGroup != NULL);

	return	FTM_LIST_count(pGroup->pLightList);
}

FTLM_LIGHT_PTR FTLM_GROUP_getLight(FTLM_GROUP_PTR pGroup, FTM_ID xID)
{
	ASSERT(pGroup != NULL);

	if (FTM_LIST_get(pGroup->pLightList, (FTM_VOID_PTR)xID, NULL) == FTM_RET_OK)
	{
		return	FTLM_OBJ_getLight(xID);
	}
	return	NULL;
}

FTLM_LIGHT_PTR FTLM_GROUP_getLightAt(FTLM_GROUP_PTR pGroup, FTM_ULONG ulIndex)
{
	FTM_ID			xID;
	ASSERT(pGroup != NULL);

	if (FTM_LIST_getAt(pGroup->pLightList, ulIndex, (FTM_VOID_PTR _PTR_)&xID) == FTM_RET_OK)
	{
		return	FTLM_OBJ_getLight(xID);
	}

	return	NULL;
}

FTM_RET	FTLM_GROUP_addLight(FTLM_GROUP_PTR pGroup, FTM_ID xID)
{
	ASSERT(pGroup != NULL);

	return	FTM_LIST_append(pGroup->pLightList, (FTM_VOID_PTR)xID);
}

FTM_RET	FTLM_SWITCH_addGroup(FTLM_SWITCH_PTR pSwitch, FTM_ID xID)
{
	ASSERT(pSwitch != NULL);

	return	FTM_LIST_append(pSwitch->pGroupList, (FTM_VOID_PTR)xID);
}

static FTM_INT	FTLM_ID_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	return	((FTM_ID)pElement == (FTM_ID)pIndicator);
}

FTM_INT	FTLM_OBJ_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT((pElement != NULL) && (pIndicator != NULL));

	return	(((FTLM_OBJECT_PTR)pElement)->xID == (FTM_ID)pIndicator);
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

