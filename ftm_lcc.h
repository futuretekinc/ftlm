#ifndef	__FTM_LCC_H__
#define	__FTM_LCC_H__

#include "ftm_types.h"
#include "simclist.h"
#include <time.h>

#define	FTM_LCC_SERVER_IP_LEN	32

typedef	struct
{
	struct
	{
		FTM_CHAR	pServerIP[FTM_LCC_SERVER_IP_LEN];	
		FTM_USHORT	usPort;
	}	xNetwork;
}	FTM_LCC_CFG, _PTR_ FTM_LCC_CFG_PTR;

FTM_RET	FTM_LCC_CFG_init(FTM_LCC_CFG_PTR pCfg);
FTM_RET	FTM_LCC_CFG_final(FTM_LCC_CFG_PTR pCfg);
FTM_RET	FTM_LCC_CFG_load(FTM_LCC_CFG_PTR pCfg, FTM_CHAR_PTR pFileName);

#endif
