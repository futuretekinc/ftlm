#ifndef	__FTLM_H__
#define	__FTLM_H__

#include "ftm_types.h"
#include "ftm_list.h"
#include "ftlm_device.h"
#include <time.h>

#define	FTLM_SERVER_IP_LEN	32

typedef	struct
{
	struct
	{
		FTM_CHAR	pServerIP[FTLM_SERVER_IP_LEN];	
		FTM_USHORT	usPort;
	}	xNetwork;

	char			pGatewayID[11];

	FTM_LIST_PTR	pLightList;
	FTM_LIST_PTR	pGroupList;
	FTM_LIST_PTR	pSwitchList;
}	FTLM_CFG, _PTR_ FTLM_CFG_PTR;

FTM_RET	FTLM_CFG_init(FTLM_CFG_PTR pCfg);
FTM_RET	FTLM_CFG_final(FTLM_CFG_PTR pCfg);
FTM_RET	FTLM_CFG_load(FTLM_CFG_PTR pCfg, FTM_CHAR_PTR pFileName);
FTM_RET FTLM_CFG_save(FTLM_CFG_PTR pCfg, FTM_CHAR_PTR pFileName);
FTM_RET	FTLM_CFG_print(FTLM_CFG_PTR pCfg);

FTLM_LIGHT_PTR 	FTLM_CFG_LIGHT_create(FTLM_CFG_PTR pCfg, FTLM_ID nID);
FTLM_LIGHT_PTR 	FTLM_CFG_LIGHT_get(FTLM_CFG_PTR pCfg, FTLM_ID nID);
FTLM_GROUP_PTR 	FTLM_CFG_GROUP_create(FTLM_CFG_PTR pCfg, FTLM_ID nID);
FTLM_GROUP_PTR 	FTLM_CFG_GROUP_get(FTLM_CFG_PTR pCfg, FTLM_ID nID);
FTLM_SWITCH_PTR FTLM_CFG_SWITCH_create(FTLM_CFG_PTR pCfg, FTLM_ID nID);
FTLM_SWITCH_PTR FTLM_CFG_SWITCH_get(FTLM_CFG_PTR pCfg, FTLM_ID nID);
#endif
