#ifndef	__FTLM_OBJECT_H__
#define	__FTLM_OBJECT_H__

#include "ftm.h"
#include "ftm_list.h"
#include "ftlm_config.h"

typedef	struct
{
	FTM_ID		xID;
	FTM_CHAR	pName[FTLM_NAME_MAX+1];
}	FTLM_OBJECT, _PTR_ FTLM_OBJECT_PTR;

typedef	struct
{
	FTLM_OBJECT			xCommon;

	FTM_CHAR			pGatewayID[FTM_GATEWAY_ID_LEN + 1];
	
	FTLM_LIGHT_STATUS	xStatus;
	FTM_ULONG			ulLevel;
	FTM_ULONG			ulTime;
}	FTLM_LIGHT, _PTR_ FTLM_LIGHT_PTR;

typedef	struct
{
	FTLM_OBJECT			xCommon;

	FTLM_LIGHT_STATUS	xStatus;
	FTM_ULONG			ulLevel;
	FTM_ULONG			ulTime;

	FTM_LIST_PTR		pLightList;
}	FTLM_GROUP,	_PTR_ FTLM_GROUP_PTR;

typedef	struct
{
	FTLM_OBJECT			xCommon;

	FTM_LIST_PTR		pGroupList;
}	FTLM_SWITCH, _PTR_ FTLM_SWITCH_PTR;

FTM_RET			FTLM_OBJ_init(FTLM_CFG_PTR pConfig);
FTM_RET			FTLM_OBJ_final(void);

FTM_ULONG		FTLM_OBJ_getLightCount(void);
FTLM_LIGHT_PTR	FTLM_OBJ_getLight(FTM_ID xID);
FTLM_LIGHT_PTR	FTLM_OBJ_getLightAt(FTM_ULONG ulIndex);
FTM_ULONG		FTLM_OBJ_getGroupCount(void);
FTLM_GROUP_PTR	FTLM_OBJ_getGroup(FTM_ID xID);
FTLM_GROUP_PTR	FTLM_OBJ_getGroupAt(FTM_ULONG ulIndex);
FTM_ULONG		FTLM_OBJ_getSwitchCount(void);
FTLM_SWITCH_PTR	FTLM_OBJ_getSwitch(FTM_ID xID);
FTLM_SWITCH_PTR	FTLM_OBJ_getSwitchAt(FTM_ULONG ulIndex);

FTLM_LIGHT_PTR	FTLM_LIGHT_create(FTLM_LIGHT_CFG_PTR pConfig);

FTLM_GROUP_PTR	FTLM_GROUP_create(FTLM_GROUP_CFG_PTR pConfig);
FTM_ULONG		FTLM_GROUP_getLightCount(FTLM_GROUP_PTR pGroup);
FTLM_LIGHT_PTR 	FTLM_GROUP_getLight(FTLM_GROUP_PTR pGroup, FTM_ID xID);
FTLM_LIGHT_PTR 	FTLM_GROUP_getLightAt(FTLM_GROUP_PTR pGroup, FTM_ULONG ulIndex);
FTM_RET			FTLM_GROUP_addLight(FTLM_GROUP_PTR pGroup, FTM_ID xID);

FTLM_SWITCH_PTR	FTLM_SWITCH_create(FTLM_SWITCH_CFG_PTR pConfig);
FTM_ULONG		FTLM_SWITCH_getGroupCount(FTLM_SWITCH_PTR pSwitch);
FTLM_GROUP_PTR 	FTLM_SWITCH_getGroup(FTLM_SWITCH_PTR pSwitch, FTM_ID xID);
FTLM_GROUP_PTR 	FTLM_SWITCH_getGroupAt(FTLM_SWITCH_PTR pSwitch, FTM_ULONG ulIndex);
FTM_RET			FTLM_SWITCH_addGroup(FTLM_SWITCH_PTR pSwitch, FTM_ID xID);

FTM_RET FTLM_GROUP_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDimmingTime);
FTM_RET	FTLM_LIGHT_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime);

#endif

