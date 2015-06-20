#ifndef	__FTLM_OBJECT_H__
#define	__FTLM_OBJECT_H__

#include "ftm.h"
#include "ftm_list.h"
#include "ftlm_config.h"

typedef	struct
{
	FTM_ID		xID;
}	FTLM_OBJECT, _PTR_ FTLM_OBJECT_PTR;

typedef	struct
{
	FTLM_OBJECT			xCommon;
	FTLM_LIGHT_CFG_PTR	pConfig;

	FTLM_LIGHT_STATUS	xStatus;
	FTM_ULONG			ulLevel;
	FTM_ULONG			ulTime;
}	FTLM_LIGHT, _PTR_ FTLM_LIGHT_PTR;

typedef	struct
{
	FTLM_OBJECT			xCommon;
	FTLM_GROUP_CFG_PTR	pConfig;

	FTLM_LIGHT_STATUS	xStatus;
	FTM_ULONG			ulLevel;
	FTM_ULONG			ulTime;
}	FTLM_GROUP,	_PTR_ FTLM_GROUP_PTR;

typedef	struct
{
	FTLM_OBJECT			xCommon;
	FTLM_SWITCH_CFG_PTR	pConfig;
}	FTLM_SWITCH, _PTR_ FTLM_SWITCH_PTR;

FTM_RET	FTLM_OBJ_init(void);

FTLM_LIGHT_PTR	FTLM_LIGHT_create(FTLM_LIGHT_CFG_PTR pConfig);
FTLM_GROUP_PTR	FTLM_GROUP_create(FTLM_GROUP_CFG_PTR pConfig);
FTLM_SWITCH_PTR	FTLM_SWITCH_create(FTLM_SWITCH_CFG_PTR pConfig);

FTM_RET FTLM_GROUP_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDimmingTime);
FTM_RET	FTLM_LIGHT_set(FTM_ID xID, unsigned char nCmd, unsigned char nLevel, unsigned char nDulationTime);

#endif

