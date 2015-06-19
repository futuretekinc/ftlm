#ifndef	__FTLM_DEVICE_H__
#define	__FTLM_DEVICE_H__

#include "ftm.h"
#include "ftm_list.h"

#define	FTLM_GROUP_MAX	8
#define	FTLM_LIGHT_MAX	255

typedef	unsigned char	FTLM_ID;

typedef	struct
{
	FTLM_ID			nID;
	char			pNodeID[33];
	unsigned char	nGroupIDs;
	FTLM_ID			pGroupIDs[FTLM_GROUP_MAX];	

	unsigned char	nStatus;
	unsigned char	nLevel;
	unsigned char	nDulationTime;
}	FTLM_LIGHT, _PTR_ FTLM_LIGHT_PTR;

typedef	struct
{
	FTLM_ID			nID;
	int				nGroupIDs;
	FTLM_ID			pGroupIDs[FTLM_GROUP_MAX];	
}	FTLM_SWITCH, _PTR_ FTLM_SWITCH_PTR;

typedef	struct
{
	FTLM_ID			nID;
	unsigned char	nLightIDs;
	FTLM_ID			pLightIDs[255];

	unsigned char	nStatus;
	unsigned char	nLevel;
	unsigned char	nDimmingTime;
}	FTLM_GROUP, _PTR_ FTLM_GROUP_PTR;

#endif

