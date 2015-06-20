#ifndef	__LCC_CLIENT_H__
#define	__LCC_CLIENT_H__

#include "ftm.h"
#include "ftlm_msg.h"
#include "ftlm_config.h"

typedef	struct
{
	FTLM_SERVER_CFG		xConfig;
	int					bRun;
	FTM_INT				hSocket;
	pthread_t			hThread;
	FTM_ULONG			ulTimeout;

	FTM_RET 			(*CB_recv)(void *, void *);
}	FTLM_CTX, _PTR_ FTLM_CTX_PTR;

FTLM_CTX_PTR	FTLM_CLIENT_create(FTLM_SERVER_CFG_PTR pConfig, FTM_RET (*CB_recv)(void *, void *));
FTM_RET			FTLM_CLIENT_start(FTLM_CTX_PTR	pxCTX);
FTM_RET			FTLM_CLIENT_stop(FTLM_CTX_PTR	pxCTX);

FTM_RET			FTLM_CLIENT_sendFrame(FTLM_CTX_PTR pxCTX, FTLM_FRAME_PTR pFrame);

#endif
