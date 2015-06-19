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
#include "ftlm_msg.h"

#undef	TRACE
#define	TRACE(...) fprintf(stderr, ## __VA_ARGS__)
#undef	ERROR
#define	ERROR(...) fprintf(stderr, ## __VA_ARGS__)

//static sem_t	xSemaphore;
FTM_VOID_PTR FTLM_process(FTM_VOID_PTR pData);

FTM_RET	FTLM_receiveFrame(FTLM_PTR pLCC, FTLM_FRAME_PTR pFrame);
FTM_RET	FTLM_sendFrame(FTLM_PTR pLCC, FTLM_FRAME_PTR pFrame);
FTM_RET	FTLM_FRAME_dump(FTLM_FRAME_PTR pFrame);

FTLM_PTR	FTLM_create(FTLM_CONFIG_PTR pConfig)
{
	FTLM_PTR pLCC = NULL;
	pLCC = (FTLM_PTR)FTM_MEM_malloc(sizeof(FTLM));
	if (pLCC == NULL)
	{
		return	NULL;	
	}
	
	memset(pLCC, 0, sizeof(FTLM));
	memcpy(&pLCC->xConfig, pConfig, sizeof(FTLM_CONFIG));


	return	pLCC;
}

FTM_RET	FTLM_start(FTLM_PTR pLCC)
{
	if (pLCC == NULL)
	{
		return	FTM_RET_ERROR;
	}

	if (pLCC->hThread != 0)
	{
		return	FTM_RET_ERROR;	
	}

	pLCC->bRun = 1;

	if (pthread_create(&pLCC->hThread, NULL, FTLM_process, pLCC) != 0)
	{
		pLCC->bRun = 0;

		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_stop(FTLM_PTR pLCC)
{
	if (pLCC->hThread == 0)
	{
		return	FTM_RET_ERROR;	
	}

	pLCC->bRun = 0;
	
	while(pLCC->hThread != 0)
	{
		sleep(1);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_connect
(
	FTLM_PTR	pLCC
)
{
	struct	sockaddr_in	xServer;

	ASSERT(pLCC != NULL);
/*
	struct hostent		*he;
	he = gethostbyname(pLCC->xConfig.pServerIP);
	if (he == NULL)
	{
		ERROR("Can't find host name [%s]\n", pLCC->xConfig.pServerIP);
		
		return	FTM_RET_ERROR;	
	}
*/
	xServer.sin_addr.s_addr	= inet_addr(pLCC->xConfig.pServerIP);//inet_lnaof(*((struct in_addr **)he->h_addr_list)[0]);
	xServer.sin_family		= AF_INET;
	xServer.sin_port		= htons(pLCC->xConfig.usPort);
	TRACE("Connecting ... [server = %s, Port = %d]\n", pLCC->xConfig.pServerIP, pLCC->xConfig.usPort);

	pLCC->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pLCC->hSocket == -1)
	{
		ERROR("Could net create socket.\n");
		return	FTM_RET_ERROR;
	}

	if (connect(pLCC->hSocket, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		close(pLCC->hSocket);
		pLCC->hSocket = 0;

		TRACE("Not connected.\n");

		return	FTM_RET_ERROR;	
	}
	TRACE("Connected.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTLM_disconnect
(
	FTLM_PTR	pLCC
)
{
	if ((pLCC == NULL) || (pLCC->hSocket <= 0))
	{
		return  FTM_RET_CLIENT_HANDLE_INVALID;
	}

	pthread_kill(pLCC->hThread, 1);
	close(pLCC->hSocket);
	pLCC->hSocket = 0;
	
	return  FTM_RET_OK;

}

FTM_VOID_PTR FTLM_process(FTM_VOID_PTR pData)
{
	FTLM_PTR  	pLCC = (FTLM_PTR)pData;
	FTM_RET			nRet = 0;
	FTLM_FRAME	xFrame;


	nRet = FTLM_connect(pLCC);
	while(pLCC->bRun)
	{
		nRet = FTLM_receiveFrame(pLCC, &xFrame);
		switch(nRet)
		{
		case	FTM_RET_COMM_DISCONNECTED:
			{
				sleep(20);	
				nRet = FTLM_connect(pLCC);
			}
			break;

		case	FTM_RET_OK:
			{
				if (pLCC->xConfig.CB_recv != NULL)
				{
					pLCC->xConfig.CB_recv(pLCC, &xFrame);	
				}
			}
		}
	}

	pLCC->hThread = 0;

	FTLM_disconnect(pLCC);

	TRACE("The session(%08x) was closed\n", pLCC->hSocket);
	close(pLCC->hSocket);
	pLCC->hSocket = 0;
 
 	return  0;
 }

FTM_RET	FTLM_receiveFrame(FTLM_PTR pLCC, FTLM_FRAME_PTR pFrame)
{
	unsigned short nLen;

	pFrame->nRecvLen = recv(pLCC->hSocket, pFrame->pRecvBuff, sizeof(pFrame->pRecvBuff), 0);
	if (pFrame->nRecvLen <= 0)
	{
		TRACE("The connection is terminated.\n");
		return	FTM_RET_COMM_DISCONNECTED;
	}

	nLen = ((unsigned short)pFrame->pRecvBuff[1] << 8) | ((unsigned short)pFrame->pRecvBuff[2]) ;

	if ((pFrame->nRecvLen != (nLen + 5)) ||
		(pFrame->pRecvBuff[0] != FTLM_MSG_STX) ||
		(pFrame->pRecvBuff[pFrame->nRecvLen - 2] != FTLM_MSG_ETX1) ||
		(pFrame->pRecvBuff[pFrame->nRecvLen - 1] != FTLM_MSG_ETX2))
	{
		FTLM_FRAME_dump(pFrame);
		return	FTM_RET_COMM_INVALID_FRAME;	
	}

	if (pFrame->nRecvLen == 19)
	{
		pFrame->nCmd 		= FTLM_CMD_RESET;
		pFrame->pGatewayID 	= &pFrame->pRecvBuff[4];
	}
	else if (pFrame->nRecvLen > 19) 
	{
		pFrame->nCmd 		= ((unsigned short)pFrame->pRecvBuff[3] << 8) | (pFrame->pRecvBuff[14] << 4);
		pFrame->pGatewayID 	= &pFrame->pRecvBuff[4];
		pFrame->pReqParam	= (FTLM_REQUEST_PARAM_PTR)&pFrame->pRecvBuff[15];
		pFrame->nRespLen    = 0;
		pFrame->pRespParam	= (FTLM_RESPONSE_PARAM_PTR)&pFrame->pRespBuff[16];

		switch(pFrame->nCmd)
		{
		case	FTLM_CMD_GROUP_CTRL:
			{
				if (pFrame->nRecvLen != 17 + sizeof(FTLM_GROUP_CTRL_PARAM) +  pFrame->pReqParam->xGroupCtrl.nGroups * sizeof(FTLM_GROUP_CTRL))
				{
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CMD_LIGHT_CTRL:
			{
				if (pFrame->nRecvLen != 17 + sizeof(FTLM_LIGHT_CTRL_PARAM) + pFrame->pReqParam->xLightCtrl.nLights * sizeof(FTLM_LIGHT_CTRL))
				{
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CMD_GROUP_SET:
			{
				int	i;
				unsigned char nLen = 17 + sizeof(FTLM_GROUP_SET_PARAM);

				unsigned char nSets = ((FTLM_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
				FTLM_GROUP_SET_PTR	pSet = ((FTLM_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

				for(i = 0 ; i < nSets ; i++)
				{
					nLen += sizeof(FTLM_GROUP_SET) + pSet->nGroups;
					pSet = (FTLM_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
				}

				if (pFrame->nRecvLen != nLen)
				{
					printf("Invlaid Frame\n");
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CMD_SWITCH_GROUP_SET:
			{
				int	i;
				unsigned char nLen = 17 + sizeof(FTLM_SWITCH_GROUP_SET_PARAM);

				unsigned char nSets = ((FTLM_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
				FTLM_SWITCH_GROUP_SET_PTR	pSet = ((FTLM_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

				for(i = 0 ; i < nSets ; i++)
				{
					nLen += sizeof(FTLM_SWITCH_GROUP_SET) + pSet->nGroups;
					pSet = (FTLM_SWITCH_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
				}

				if (pFrame->nRecvLen != nLen)
				{
					return	FTM_RET_COMM_INVALID_FRAME;	
				}
			}
			break;

		case	FTLM_CMD_STATUS_GET:
			{

				if (pFrame->nRecvLen != 17 + sizeof(FTLM_STATUS_GET_PARAM))
				{
					FTLM_FRAME_dump(pFrame);
					return	FTM_RET_COMM_INVALID_FRAME;	
				}

				pFrame->nCmd |= (((FTLM_STATUS_GET_PARAM_PTR)pFrame->pReqParam)->nMode & 0x0F);
			}
			break;

		default:
			return	FTM_RET_COMM_INVALID_LEN;	
		}
	}
	else
	{
		pFrame->nCmd = FTLM_CMD_INVALID;
		printf("pFrame->nLen = %d\n", pFrame->nRecvLen);

		return	FTM_RET_COMM_INVALID_LEN;	
	}

	FTLM_FRAME_dump(pFrame);

	return	FTM_RET_OK;
}

FTM_RET	FTLM_sendFrame(FTLM_PTR pLCC, FTLM_FRAME_PTR pFrame)
{
	int	i;

	if (pLCC == NULL || pLCC->hSocket == 0)
	{
		return	FTM_RET_ERROR;	
	}

	switch(pFrame->nCmd)
	{
	case	FTLM_CMD_RESET:
		{
			pFrame->nRespLen = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_STX;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = 14;
			memcpy(&pFrame->pRespBuff[pFrame->nRespLen], pFrame->pGatewayID, FTLM_GWID_LEN);
			pFrame->nRespLen += FTLM_GWID_LEN;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x55;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xaa;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xff;
			pFrame->pRespBuff[pFrame->nRespLen++] = pFrame->nRet;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_ETX1;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_ETX2;
		}
		break;

	case	FTLM_CMD_GROUP_CTRL:
	case	FTLM_CMD_LIGHT_CTRL:
	case	FTLM_CMD_GROUP_SET:
	case	FTLM_CMD_SWITCH_GROUP_SET:
		{
			pFrame->nRespLen = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_STX;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = 14;
			memcpy(&pFrame->pRespBuff[pFrame->nRespLen], pFrame->pGatewayID, FTLM_GWID_LEN);
			pFrame->nRespLen += FTLM_GWID_LEN;
			pFrame->pRespBuff[pFrame->nRespLen++] = (pFrame->nCmd >> 4) & 0x0F;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xaa;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x55;
			pFrame->pRespBuff[pFrame->nRespLen++] = pFrame->nRet;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_ETX1;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_ETX2;
		}
		break;

	case	FTLM_CMD_GROUP_MAPPING_GET:
	case	FTLM_CMD_SWITCH_MAPPING_GET:
	case	FTLM_CMD_GROUP_STATUS_GET:
	case	FTLM_CMD_LIGHT_STATUS_GET:
		{
			int	nDataLen = pFrame->nRespParamLen + 13;
			pFrame->nRespLen = 0;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_STX;
			pFrame->pRespBuff[pFrame->nRespLen++] = (nDataLen >> 8) & 0xFF;
			pFrame->pRespBuff[pFrame->nRespLen++] = (nDataLen     ) & 0xFF;
			memcpy(&pFrame->pRespBuff[pFrame->nRespLen], pFrame->pGatewayID, FTLM_GWID_LEN);
			pFrame->nRespLen += FTLM_GWID_LEN;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x04;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0xaa;
			pFrame->pRespBuff[pFrame->nRespLen++] = 0x55;
			pFrame->nRespLen += pFrame->nRespParamLen;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_ETX1;
			pFrame->pRespBuff[pFrame->nRespLen++] = FTLM_MSG_ETX2;
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

	if (send(pLCC->hSocket, pFrame->pRespBuff, pFrame->nRespLen, 0) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}
