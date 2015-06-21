#ifndef	__LCC_CLIENT_H__
#define	__LCC_CLIENT_H__

#include "ftm.h"
#include "ftlm_msg.h"
#include "ftlm_config.h"

typedef	FTM_RET	(*FTLM_CLIENT_CB_MESSAGE)(void *pObj, void *pParam);

typedef	struct
{
	FTLM_CLIENT_CFG		xConfig;
	int					bRun;
	FTM_INT				hSocket;
	pthread_t			hThread;
	FTM_ULONG			ulTimeout;

	FTLM_CLIENT_CB_MESSAGE	CB_message;
}	FTLM_CLIENT, _PTR_ FTLM_CLIENT_PTR;

FTLM_CLIENT_PTR	FTLM_CLIENT_create(FTLM_CLIENT_CFG_PTR pConfig);
FTM_RET			FTLM_CLIENT_destroy(FTLM_CLIENT_PTR pClient);
FTM_RET			FTLM_CLIENT_start(FTLM_CLIENT_PTR	pClient);
FTM_RET			FTLM_CLIENT_stop(FTLM_CLIENT_PTR	pClient);

FTM_RET			FTLM_CLIENT_sendFrame(FTLM_CLIENT_PTR pClient, FTLM_FRAME_PTR pFrame);

FTM_RET			FTLM_CLIENT_setMessageCB(FTLM_CLIENT_PTR pClient, FTLM_CLIENT_CB_MESSAGE pCB);
#endif
