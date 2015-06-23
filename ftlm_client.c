#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <netdb.h>
#include "ftm.h"
#include "ftm_types.h"
#include "ftm_mem.h"
#include "ftlm_client.h"
#include "ftlm_client_msg.h"

#undef	TRACE
#define	TRACE(...) fprintf(stderr, ## __VA_ARGS__)
#undef	ERROR
#define	ERROR(...) fprintf(stderr, ## __VA_ARGS__)

//static sem_t	xSemaphore;
FTM_VOID_PTR FTLM_CLIENT_process(FTM_VOID_PTR pData);

FTM_RET	FTLM_CLIENT_receiveFrame(FTLM_CLIENT_PTR pClient, FTLM_CLIENT_FRAME_PTR pFrame);
FTM_RET	FTLM_CLIENT_sendFrame(FTLM_CLIENT_PTR pClient, FTLM_CLIENT_FRAME_PTR pFrame);
FTM_RET	FTLM_CLIENT_FRAME_dump(FTLM_CLIENT_FRAME_PTR pFrame);

FTLM_CLIENT_PTR	FTLM_CLIENT_create(FTLM_CLIENT_CFG_PTR pConfig)
{
	FTLM_CLIENT_PTR pClient = NULL;

	pClient = (FTLM_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTLM_CLIENT));
	if (pClient == NULL)
	{
		return	NULL;	
	}

	memset(pClient, 0, sizeof(FTLM_CLIENT));
	memcpy(&pClient->xConfig, pConfig, sizeof(FTLM_CLIENT_CFG));


	return	pClient;
}

FTM_RET	FTLM_CLIENT_destroy(FTLM_CLIENT_PTR pClient)
{
	if (pClient->bRun)
	{
		FTLM_CLIENT_stop(pClient);	
	}

	FTM_MEM_free(pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_start(FTLM_CLIENT_PTR pClient)
{
	if (pClient == NULL)
	{
		return	FTM_RET_ERROR;
	}

	if (pClient->hThread != 0)
	{
		return	FTM_RET_ERROR;	
	}

	pClient->bRun = 1;

	if (pthread_create(&pClient->hThread, NULL, FTLM_CLIENT_process, pClient) != 0)
	{
		pClient->bRun = 0;

		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_stop(FTLM_CLIENT_PTR pClient)
{
	if (pClient->hThread == 0)
	{
		return	FTM_RET_ERROR;	
	}

	pClient->bRun = 0;
	
	while(pClient->hThread != 0)
	{
		sleep(1);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_connect
(
	FTLM_CLIENT_PTR	pClient
)
{
	struct	sockaddr_in	xServer;

	ASSERT(pClient != NULL);
/*
	struct hostent		*he;
	he = gethostbyname(pClient->xConfig.pServerIP);
	if (he == NULL)
	{
		ERROR("Can't find host name [%s]\n", pClient->xConfig.pServerIP);
		
		return	FTM_RET_ERROR;	
	}
*/
	xServer.sin_addr.s_addr	= inet_addr(pClient->xConfig.xServer.pIP);//inet_lnaof(*((struct in_addr **)he->h_addr_list)[0]);
	xServer.sin_family		= AF_INET;
	xServer.sin_port		= htons(pClient->xConfig.xServer.usPort);
	TRACE("Connecting ... [server = %s, Port = %d]\n", pClient->xConfig.xServer.pIP, pClient->xConfig.xServer.usPort);

	pClient->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pClient->hSocket == -1)
	{
		ERROR("Could net create socket.\n");
		return	FTM_RET_ERROR;
	}

	if (connect(pClient->hSocket, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		close(pClient->hSocket);
		pClient->hSocket = 0;

		TRACE("Not connected.\n");

		return	FTM_RET_ERROR;	
	}
	TRACE("Connected.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_disconnect
(
	FTLM_CLIENT_PTR	pClient
)
{
	if ((pClient == NULL) || (pClient->hSocket <= 0))
	{
		return  FTM_RET_CLIENT_HANDLE_INVALID;
	}

	pthread_kill(pClient->hThread, 1);
	close(pClient->hSocket);
	pClient->hSocket = 0;
	
	return  FTM_RET_OK;

}

FTM_VOID_PTR FTLM_CLIENT_process(FTM_VOID_PTR pData)
{
	FTLM_CLIENT_PTR  	pClient = (FTLM_CLIENT_PTR)pData;
	FTM_RET			nRet = 0;
	FTLM_CLIENT_FRAME	xFrame;


	nRet = FTLM_CLIENT_connect(pClient);
	while(pClient->bRun)
	{
		nRet = FTLM_CLIENT_receiveFrame(pClient, &xFrame);
		switch(nRet)
		{
		case	FTM_RET_COMM_DISCONNECTED:
			{
				sleep(20);	
				nRet = FTLM_CLIENT_connect(pClient);
			}
			break;

		case	FTM_RET_OK:
			{
				if (pClient->CB_message != NULL)
				{
					pClient->CB_message(pClient, &xFrame);	
				}
			}
		}
	}

	pClient->hThread = 0;

	FTLM_CLIENT_disconnect(pClient);

	TRACE("The session(%08x) was closed\n", pClient->hSocket);
	close(pClient->hSocket);
	pClient->hSocket = 0;
 
 	return  0;
 }

FTM_RET	FTLM_CLIENT_receiveFrame(FTLM_CLIENT_PTR pClient, FTLM_CLIENT_FRAME_PTR pFrame)
{
	unsigned short nLen;

	pFrame->nRecvLen = recv(pClient->hSocket, pFrame->pRecvBuff, sizeof(pFrame->pRecvBuff), 0);
	if (pFrame->nRecvLen <= 0)
	{
		TRACE("The connection is terminated.\n");
		return	FTM_RET_COMM_DISCONNECTED;
	}

	nLen = ((unsigned short)pFrame->pRecvBuff[1] << 8) | ((unsigned short)pFrame->pRecvBuff[2]) ;

	if ((pFrame->nRecvLen != (nLen + 5)) ||
		(pFrame->pRecvBuff[0] != FTLM_CLIENT_MSG_STX) ||
		(pFrame->pRecvBuff[pFrame->nRecvLen - 2] != FTLM_CLIENT_MSG_ETX1) ||
		(pFrame->pRecvBuff[pFrame->nRecvLen - 1] != FTLM_CLIENT_MSG_ETX2))
	{
		FTLM_CLIENT_FRAME_dump(pFrame);
		return	FTM_RET_COMM_INVALID_FRAME;	
	}

	if (pFrame->nRecvLen == 19)
	{
		pFrame->nCmd 		= FTLM_CLIENT_CMD_RESET;
		pFrame->pGatewayID 	= &pFrame->pRecvBuff[4];
	}
	else if (pFrame->nRecvLen > 19) 
	{
		pFrame->nCmd 		= ((unsigned short)pFrame->pRecvBuff[3] << 8) | (pFrame->pRecvBuff[14] << 4);
		pFrame->pGatewayID 	= &pFrame->pRecvBuff[4];
		pFrame->pReqParam	= (FTLM_CLIENT_REQUEST_PARAM_PTR)&pFrame->pRecvBuff[15];
		pFrame->nRespLen    = 0;
		pFrame->pRespParam	= (FTLM_CLIENT_RESPONSE_PARAM_PTR)&pFrame->pRespBuff[16];

		switch(pFrame->nCmd)
		{
		case	FTLM_CLIENT_CMD_GROUP_CTRL:
			{
				if (pFrame->nRecvLen != 17 + sizeof(FTLM_CLIENT_GROUP_CTRL_PARAM) +  pFrame->pReqParam->xGroupCtrl.nGroups * sizeof(FTLM_CLIENT_GROUP_CTRL))
				{
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CLIENT_CMD_LIGHT_CTRL:
			{
				if (pFrame->nRecvLen != 17 + sizeof(FTLM_CLIENT_LIGHT_CTRL_PARAM) + pFrame->pReqParam->xLightCtrl.nLights * sizeof(FTLM_CLIENT_LIGHT_CTRL))
				{
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CLIENT_CMD_GROUP_SET:
			{
				int	i;
				unsigned char nLen = 17 + sizeof(FTLM_CLIENT_GROUP_SET_PARAM);

				unsigned char nSets = ((FTLM_CLIENT_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
				FTLM_CLIENT_GROUP_SET_PTR	pSet = ((FTLM_CLIENT_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

				for(i = 0 ; i < nSets ; i++)
				{
					nLen += sizeof(FTLM_CLIENT_GROUP_SET) + pSet->nGroups;
					pSet = (FTLM_CLIENT_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
				}

				if (pFrame->nRecvLen != nLen)
				{
					printf("Invlaid Frame\n");
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CLIENT_CMD_SWITCH_GROUP_SET:
			{
				int	i;
				unsigned char nLen = 17 + sizeof(FTLM_CLIENT_SWITCH_GROUP_SET_PARAM);

				unsigned char nSets = ((FTLM_CLIENT_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
				FTLM_CLIENT_SWITCH_GROUP_SET_PTR	pSet = ((FTLM_CLIENT_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

				for(i = 0 ; i < nSets ; i++)
				{
					nLen += sizeof(FTLM_CLIENT_SWITCH_GROUP_SET) + pSet->nGroups;
					pSet = (FTLM_CLIENT_SWITCH_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
				}

				if (pFrame->nRecvLen != nLen)
				{
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CLIENT_CMD_STATUS_GET:
			{

				if (pFrame->nRecvLen != 17 + sizeof(FTLM_CLIENT_STATUS_GET_PARAM))
				{
					FTLM_CLIENT_FRAME_dump(pFrame);
					return	FTM_RET_COMM_INVALID_FRAME;	
				}

				pFrame->nCmd |= (((FTLM_CLIENT_STATUS_GET_PARAM_PTR)pFrame->pReqParam)->nMode & 0x0F);
			}
			break;

		default:
			return	FTM_RET_COMM_INVALID_LEN;	
		}
	}
	else
	{
		pFrame->nCmd = FTLM_CLIENT_CMD_INVALID;
		printf("pFrame->nLen = %d\n", pFrame->nRecvLen);

		return	FTM_RET_COMM_INVALID_LEN;	
	}

	FTLM_CLIENT_FRAME_dump(pFrame);

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_sendFrame(FTLM_CLIENT_PTR pClient, FTLM_CLIENT_FRAME_PTR pFrame)
{
	int	i;

	if (pClient == NULL || pClient->hSocket == 0)
	{
		return	FTM_RET_ERROR;	
	}

	switch(pFrame->nCmd)
	{
	case	FTLM_CLIENT_CMD_RESET:
		{
			pFrame->nRespLen = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_STX;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = 14;
			memcpy(&pFrame->pRespBuff[pFrame->nRespLen], pFrame->pGatewayID, FTLM_CLIENT_GWID_LEN);
			pFrame->nRespLen += FTLM_CLIENT_GWID_LEN;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x55;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xaa;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xff;
			pFrame->pRespBuff[pFrame->nRespLen++] = pFrame->nRet;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_ETX1;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_ETX2;
		}
		break;

	case	FTLM_CLIENT_CMD_GROUP_CTRL:
	case	FTLM_CLIENT_CMD_LIGHT_CTRL:
	case	FTLM_CLIENT_CMD_GROUP_SET:
	case	FTLM_CLIENT_CMD_SWITCH_GROUP_SET:
		{
			pFrame->nRespLen = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_STX;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = 14;
			memcpy(&pFrame->pRespBuff[pFrame->nRespLen], pFrame->pGatewayID, FTLM_CLIENT_GWID_LEN);
			pFrame->nRespLen += FTLM_CLIENT_GWID_LEN;
			pFrame->pRespBuff[pFrame->nRespLen++] = (pFrame->nCmd >> 4) & 0x0F;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xaa;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x55;
			pFrame->pRespBuff[pFrame->nRespLen++] = pFrame->nRet;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_ETX1;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_ETX2;
		}
		break;

	case	FTLM_CLIENT_CMD_GROUP_MAPPING_GET:
	case	FTLM_CLIENT_CMD_SWITCH_MAPPING_GET:
	case	FTLM_CLIENT_CMD_GROUP_STATUS_GET:
	case	FTLM_CLIENT_CMD_LIGHT_STATUS_GET:
		{
			int	nDataLen = pFrame->nRespParamLen + 13;
			pFrame->nRespLen = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_STX;
			pFrame->pRespBuff[pFrame->nRespLen++] = (nDataLen >> 8) & 0xFF;
			pFrame->pRespBuff[pFrame->nRespLen++] = (nDataLen     ) & 0xFF;
			memcpy(&pFrame->pRespBuff[pFrame->nRespLen], pFrame->pGatewayID, FTLM_CLIENT_GWID_LEN);
			pFrame->nRespLen += FTLM_CLIENT_GWID_LEN;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x04;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xaa;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x55;
			pFrame->nRespLen += pFrame->nRespParamLen;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_ETX1;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_CLIENT_MSG_ETX2;
		}
		break;

	default:
		{
			return	FTM_RET_ERROR;	
		}
	}

	printf("SEND FRAME : %d\n", pFrame->nRespLen);
	for(i = 0 ; i < pFrame->nRespLen ; i++)
	{
		printf("%02x ", pFrame->pRespBuff[i]);	
		if ((i+1) % 16 == 0)
		{
			printf("\n");	
		}
	}
	printf("\n");	

	if (send(pClient->hSocket, pFrame->pRespBuff, pFrame->nRespLen, 0) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CLIENT_setMessageCB(FTLM_CLIENT_PTR pClient, FTLM_CLIENT_CB_MESSAGE pCB)
{
	ASSERT(pClient != NULL);

	pClient->CB_message = pCB;

	return	FTM_RET_OK;
}
