#include <string.h>
#include "ftm.h"
#include "ftlm_client_msg.h"

FTM_RET	FTLM_CLIENT_FRAME_dump(FTLM_CLIENT_FRAME_PTR pFrame)
{
	int	i;

	switch (pFrame->nCmd)
	{
	case	FTLM_CLIENT_CMD_RESET:
		{
			printf("COMMAND ID : RESET\n");
			printf("GATEWAY ID : ");

			for(i = 0 ; i < FTLM_CLIENT_GWID_LEN ; i++)
			{
				printf("%02x ", pFrame->pGatewayID[i]);
			}
			printf("\n");	
		}
		break;

	case	FTLM_CLIENT_CMD_GROUP_CTRL:
		{
			
			int	i;

			printf("%d\n", 	pFrame->pReqParam->xGroupCtrl.nGroups);
			for(i = 0 ; i < pFrame->pReqParam->xGroupCtrl.nGroups; i++)
			{
				printf("%8s : %d\n", "ID",	 	pFrame->pReqParam->xGroupCtrl.pGroups[i].nID);
				printf("%8s : %d\n", "CMD", 	pFrame->pReqParam->xGroupCtrl.pGroups[i].nCmd);
				printf("%8s : %d\n", "LEVEL", 	pFrame->pReqParam->xGroupCtrl.pGroups[i].nLevel);
				printf("%8s : %d\n", "DIMMING",pFrame->pReqParam->xGroupCtrl.pGroups[i].nDimmingTime);
			}
		}
		break;

	case	FTLM_CLIENT_CMD_LIGHT_CTRL:
		{
			int	i;

			FTLM_CLIENT_LIGHT_CTRL_PARAM_PTR pParam = (FTLM_CLIENT_LIGHT_CTRL_PARAM_PTR)pFrame->pReqParam;

			for(i = 0 ; i < pParam->nLights ; i++)
			{
				printf("%8s : %d\n", "ID",		pParam->pLights[i].nID);
				printf("%8s : %d\n", "CMD", 	pParam->pLights[i].nCmd);
				printf("%8s : %d\n", "LEVEL", 	pParam->pLights[i].nLevel);
				printf("%8s : %d\n", "DULATION",pParam->pLights[i].nDulationTime); 
			}
		}
	break;

	case	FTLM_CLIENT_CMD_GROUP_SET:
		{
			int	i;
			unsigned char nSets = ((FTLM_CLIENT_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
			FTLM_CLIENT_GROUP_SET_PTR	pSet = ((FTLM_CLIENT_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

			for(i = 0 ; i < nSets ; i++)
			{
				int	j;

				printf("%8s : %d\n", "ID",	pSet->nID);
				for(j = 0 ; j < pSet->nGroups ; j++)
				{
					printf("    %4d : %d\n", j+1, pSet->pGroups[j]);
				}

				pSet = (FTLM_CLIENT_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
			}
		}
		break;

	case	FTLM_CLIENT_CMD_SWITCH_GROUP_SET:
		{
			int	i;
			unsigned char nSets = ((FTLM_CLIENT_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->nSets;
			FTLM_CLIENT_SWITCH_GROUP_SET_PTR	pSet = ((FTLM_CLIENT_SWITCH_GROUP_SET_PARAM_PTR)pFrame->pReqParam)->pSets;

			for(i = 0 ; i < nSets ; i++)
			{
				int	j;

				printf("%8s : %d\n", "ID",	pSet->nID);
				for(j = 0 ; j < pSet->nGroups ; j++)
				{
					printf("    %4d : %d\n", j+1, pSet->pGroups[j]);
				}

				pSet = (FTLM_CLIENT_SWITCH_GROUP_SET_PTR)((unsigned char *)pSet + 2 + pSet->nGroups);
			}
		}
		break;

	case FTLM_CLIENT_CMD_LIGHT_STATUS_GET:
		{
		}
		break;
	}

	for(i = 0 ; i < pFrame->nRecvLen ; i++)
	{
		printf("%02x ", pFrame->pRecvBuff[i]);	
		if (((i+1) % 16) == 0)
		{
			printf("\n");	
		}
	}

	if ((i % 16) != 0)
	{
		printf("\n");	
	}

	return	FTM_RET_OK;
}
