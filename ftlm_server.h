#ifndef	__FTLM_SERVER_H__
#define	__FTLM_SERVER_H__
#include <sys/ipc.h>
#include <sys/shm.h>
#include "ftm.h"
#include "ftlm_client_msg.h"
#include "ftlm_config.h"
#define	FTLM_MSG_FRAME_SIZE	2048

typedef	FTM_RET	(*FTLM_SERVER_CB_MESSAGE)(void *pObj, void *pParam);

typedef	struct
{
	FTM_BOOL	bReserved;
	FTM_BOOL	bReq;
	FTM_BOOL	bResp;
	FTM_ULONG	ulReqBuffLen;
	FTM_BYTE	pReqBuff[FTLM_MSG_FRAME_SIZE];
	FTM_ULONG	ulRespBuffLen;
	FTM_BYTE	pRespBuff[FTLM_MSG_FRAME_SIZE];
}	FTLM_MSG_SLOT, _PTR_ FTLM_MSG_SLOT_PTR;

typedef struct
{
	key_t				xMemKey;
	FTM_ULONG			ulSlotCount;
}	FTLM_SERVER_CFG, _PTR_ FTLM_SERVER_CFG_PTR;

typedef	struct
{
	int					bRun;
	FTM_INT				hSocket;
	pthread_t			hThread;
	FTM_ULONG			ulTimeout;

	key_t				xMemKey;
	FTM_INT				xMemID;
	FTM_ULONG			ulSlotCount;
	FTLM_MSG_SLOT_PTR	pMsgSlot;
	FTLM_SERVER_CB_MESSAGE	CB_message;
}	FTLM_SERVER, _PTR_ FTLM_SERVER_PTR;

FTLM_SERVER_PTR	FTLM_SERVER_create(FTLM_SERVER_CFG_PTR pConfig);
FTM_RET			FTLM_SERVER_destroy(FTLM_SERVER_PTR pClient);
FTM_RET			FTLM_SERVER_start(FTLM_SERVER_PTR	pClient);
FTM_RET			FTLM_SERVER_stop(FTLM_SERVER_PTR	pClient);

//FTM_RET			FTLM_SERVER_sendFrame(FTLM_SERVER_PTR pClient, FTLM_FRAME_PTR pFrame);

FTM_RET			FTLM_SERVER_setMessageCB(FTLM_SERVER_PTR pClient, FTLM_SERVER_CB_MESSAGE pCB);
#endif
