#ifndef	__LCC_CLIENT_H__
#define	__LCC_CLIENT_H__

#include "ftm.h"
#include "ftlm_msg.h"

typedef	struct
{
	char				pServerIP[256];
	unsigned short		usPort;

	FTM_RET				(*CB_recv)(void *pObj, void *pFrame);
}	FTLM_CONFIG, _PTR_ FTLM_CONFIG_PTR;


typedef	struct
{
	FTLM_CONFIG	xConfig;

	int				bRun;
	FTM_INT			hSocket;
	pthread_t		hThread;
	FTM_ULONG		ulTimeout;
}	FTLM, _PTR_ FTLM_PTR;

FTLM_PTR	FTLM_create(FTLM_CONFIG_PTR pConfig);
FTM_RET		FTLM_start(FTLM_PTR	pLCC);
FTM_RET		FTLM_stop(FTLM_PTR	pLCC);

FTM_RET		FTLM_sendFrame(FTLM_PTR pLCC, FTLM_FRAME_PTR pFrame);

#endif
