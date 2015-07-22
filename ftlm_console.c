#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm_types.h"
#include "ftm_console.h"
#include "ftlm_server.h"
#include "ftlm_server_api.h"

FTM_RET	FTLM_CONSOLE_light(FTM_INT nArgc, FTM_CHAR_PTR pArgs[]);
FTM_RET	FTLM_CONSOLE_group(FTM_INT nArgc, FTM_CHAR_PTR pArgs[]);
FTM_RET	FTLM_CONSOLE_switch(FTM_INT nArgc, FTM_CHAR_PTR pArgs[]);

static FTM_CONSOLE_CMD	_pCmdList[] =
{
	{
		.pString	= "light",
		.function	= FTLM_CONSOLE_light,
		.pShortHelp	= "light management",
		.pHelp		= "light management"
	},
	{
		.pString	= "group",
		.function	= FTLM_CONSOLE_group,
		.pShortHelp	= "group management",
		.pHelp		= "group management"
	},
	{
		.pString	= "switch",
		.function	= FTLM_CONSOLE_switch,
		.pShortHelp	= "switch management",
		.pHelp		= "switch management"
	}
};

FTM_DEBUG_CFG	_debugCfg = 
{
	.ulMode = MSG_ALL,
	.xTrace =
	{
		.bToFile= FTM_TRUE,
		.pPath	= "./log/",
		.pPrefix = "trace",
		.bLine	= FTM_FALSE
	},
	.xError =
	{
		.bToFile= FTM_TRUE,
		.pPath	= "./log/",
		.pPrefix = "error",
		.bLine	= FTM_FALSE
	},
};

int	main(FTM_INT nArgc, FTM_CHAR_PTR pArgs[])
{
	FTLM_API_init();
	FTM_DEBUG_configSet(&_debugCfg);

	FTM_CONSOLE_run(_pCmdList, sizeof(_pCmdList) / sizeof(FTM_CONSOLE_CMD));

	return	0;
}


FTM_RET	FTLM_CONSOLE_light(FTM_INT nArgc, FTM_CHAR_PTR pArgs[])
{
	FTM_ID	pLightIDs[256];
	FTM_ULONG	ulCount = 0;

	switch(nArgc)
	{
	case	1:
		{
			if (FTLM_API_LIGHT_getList(pLightIDs, 256, &ulCount) != FTM_RET_OK)
			{
				printf("command error!\n");	
			}
			else
			{
				FTM_INT	i;

				printf("%8s %16s %8s %8s %8s\n", "ID", "NAME", "CMD", "LEVEL", "TIME");
				for(i = 0 ; i < ulCount ; i++)
				{
					FTLM_LIGHT_INFO	xLightInfo;

					if (FTLM_API_LIGHT_getInfo(pLightIDs[i], &xLightInfo) == FTM_RET_OK)
					{
						printf("%8d %16s %8d %8d %8d\n", xLightInfo.nID, xLightInfo.pName, xLightInfo.nCmd, xLightInfo.nLevel, xLightInfo.nTime);
					}
				}
				printf("\n");
			}
		}
		break;

	case	2:
		{
			if (strcmp(pArgs[1], "stat") == 0)
			{
				FTLM_LIGHT_CTRL		pLightCtrls[100];	
				int					i, nLightCtrls = 0;
				if (FTLM_API_LIGHT_getCtrls(pLightCtrls, 100, &nLightCtrls) == FTM_RET_OK)
				{
					printf("%8s %8s %8s %8s\n", "ID", "CMD", "LEVEL", "TIME");
					for(i = 0 ; i < nLightCtrls ; i++)
					{
						printf("%8d %8d %8d %8d\n", pLightCtrls[i].nID, pLightCtrls[i].nCmd, pLightCtrls[i].nLevel, pLightCtrls[i].nTime);
					}
				}
			}
			else if (strcmp(pArgs[1], "groups") == 0)
			{
				FTLM_LIGHT_GROUP	pLightGroups[100];	
				int					i, j, nLightGroups = 0;
				if (FTLM_API_LIGHT_getGroups(pLightGroups, 100, &nLightGroups) == FTM_RET_OK)
				{
					printf("%8s %s\n", "LIGHT", "GROUPS");
					for(i = 0 ; i < nLightGroups ; i++)
					{
						printf("%-8d : ", pLightGroups[i].nID);
						for(j = 0 ; j < pLightGroups[i].nGroups ; j++)
						{
							printf("%-4d ", pLightGroups[i].pGroups[j]);
						}
						printf("\n");
					}
				}
			}
			else
			{
				FTLM_LIGHT_INFO	xLightInfo;
				int				nID = atoi(pArgs[1]);
				if (FTLM_API_LIGHT_getInfo(nID, &xLightInfo) != FTM_RET_OK)
				{
					printf("command error!\n");	
				}
				else
				{
					printf("< Light Information >\n");
					printf("%8s : %d\n", "ID", 		xLightInfo.nID);
					printf("%8s : %s\n", "NAME", 	xLightInfo.pName);
					printf("%8s : %d\n", "CMD", 	xLightInfo.nCmd);
					printf("%8s : %d\n", "LEVEL", 	xLightInfo.nLevel);
					printf("%8s : %d\n", "TIME", 	xLightInfo.nTime);
				}
			}
		}
		break;

	default:
		{
			if (strcmp(pArgs[1], "on") == 0)
			{
				int	i, nLightCount = 0;
				FTLM_LIGHT_CTRL	pLights[256];	
				
				if ((nArgc -2) % 2 != 0)
				{
					printf("Invalid parameters\n");
					return	FTM_RET_OK;	
				}


				for(i = 2 ; i < nArgc ; i+=2)
				{
					pLights[nLightCount].nID = atoi(pArgs[i]);
					pLights[nLightCount].nCmd = atoi(pArgs[i+1]);
					pLights[nLightCount].nLevel = 0;
					pLights[nLightCount].nTime = 0;
					nLightCount++;
				}

				FTLM_API_LIGHT_setCtrls(pLights, nLightCount);
			}
			else if (strcmp(pArgs[1], "off") == 0)
			{
				int	i, nLightCount = 0;
				FTLM_LIGHT_CTRL	pLights[256];	

				for(i = 2 ; i < nArgc ; i++)
				{
					pLights[nLightCount].nID = atoi(pArgs[i]);
					pLights[nLightCount].nCmd = 0;
					pLights[nLightCount].nLevel = 0;
					pLights[nLightCount].nTime = 0;
					nLightCount++;
				}

				FTLM_API_LIGHT_setCtrls(pLights, nLightCount);
			}
			else if (strcmp(pArgs[1], "dimm") == 0)
			{
				int	i, nLightCount = 0;
				FTLM_LIGHT_CTRL	pLights[256];	

				if ((nArgc - 2) % 3 != 0)
				{
					printf("Invalid parameters\n");
					return	FTM_RET_OK;	
				}


				for(i = 2 ; i < nArgc ; i+=3)
				{
					pLights[nLightCount].nID = atoi(pArgs[i]);
					pLights[nLightCount].nCmd = 255;
					pLights[nLightCount].nLevel = atoi(pArgs[i+1]);
					pLights[nLightCount].nTime = atoi(pArgs[i+2]);
					nLightCount++;
				}

				FTLM_API_LIGHT_setCtrls(pLights, nLightCount);
			}
		
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CONSOLE_group(FTM_INT nArgc, FTM_CHAR_PTR pArgs[])
{
	FTM_ID	pGroupIDs[256];
	FTM_ULONG	ulCount = 0;

	switch(nArgc)
	{
	case	1:
		{
			if (FTLM_API_GROUP_getList(pGroupIDs, 256, &ulCount) != FTM_RET_OK)
			{
				printf("command error!\n");	
			}
			else
			{
				FTM_INT	i, j;

				printf("%8s %16s %8s %8s %8s %s\n", "ID", "NAME", "CMD", "LEVEL", "TIME", "LIGHT");
				for(i = 0 ; i < ulCount ; i++)
				{
					FTLM_GROUP_INFO	xGroupInfo;

					if (FTLM_API_GROUP_getInfo(pGroupIDs[i], &xGroupInfo) == FTM_RET_OK)
					{
						printf("%8d %16s %8d %8d %8d %d", xGroupInfo.nID, xGroupInfo.pName, xGroupInfo.nCmd, xGroupInfo.nLevel, xGroupInfo.nTime, xGroupInfo.nLight);

						for(j = 0 ; j < xGroupInfo.nLight ; j++)
						{
							printf("%3d ", xGroupInfo.pLightIDs[j]);
						}
						printf("\n");
					}
				}
				printf("\n");
			}
		}
		break;

	case	2:
		{
			if (strcmp(pArgs[1], "stat") == 0)
			{
				FTLM_GROUP_CTRL		pGroupCtrls[100];	
				int					i, nGroupCtrls = 0;
				if (FTLM_API_GROUP_getCtrls(pGroupCtrls, 100, &nGroupCtrls) == FTM_RET_OK)
				{
					printf("%8s %8s %8s %8s\n", "ID", "CMD", "LEVEL", "TIME");
					for(i = 0 ; i < nGroupCtrls ; i++)
					{
						printf("%8d %8d %8d %8d\n", pGroupCtrls[i].nID, pGroupCtrls[i].nCmd, pGroupCtrls[i].nLevel, pGroupCtrls[i].nTime);
					}
				}
			}
			else
			{
				FTLM_GROUP_INFO	xGroupInfo;
				int				nID = atoi(pArgs[1]);
				if (FTLM_API_GROUP_getInfo(nID, &xGroupInfo) != FTM_RET_OK)
				{
					printf("command error!\n");	
				}
				else
				{
					printf("< Group Information >\n");
					printf("%8s : %d\n", "ID", 		xGroupInfo.nID);
					printf("%8s : %s\n", "NAME", 	xGroupInfo.pName);
					printf("%8s : %d\n", "CMD", 	xGroupInfo.nCmd);
					printf("%8s : %d\n", "LEVEL", 	xGroupInfo.nLevel);
					printf("%8s : %d\n", "TIME", 	xGroupInfo.nTime);
				}
			}
		}
		break;

	default:
		{
			if (strcmp(pArgs[1], "on") == 0)
			{
				int	i, nGroupCount = 0;
				FTLM_GROUP_CTRL	pGroups[256];	
				
				if ((nArgc -2) % 2 != 0)
				{
					printf("Invalid parameters\n");
					return	FTM_RET_OK;	
				}


				for(i = 2 ; i < nArgc ; i+=2)
				{
					pGroups[nGroupCount].nID = atoi(pArgs[i]);
					pGroups[nGroupCount].nCmd = atoi(pArgs[i+1]);
					pGroups[nGroupCount].nLevel = 0;
					pGroups[nGroupCount].nTime = 0;
					nGroupCount++;
				}

				FTLM_API_GROUP_setCtrls(pGroups, nGroupCount);
			}
			else if (strcmp(pArgs[1], "off") == 0)
			{
				int	i, nGroupCount = 0;
				FTLM_GROUP_CTRL	pGroups[256];	

				for(i = 2 ; i < nArgc ; i++)
				{
					pGroups[nGroupCount].nID = atoi(pArgs[i]);
					pGroups[nGroupCount].nCmd = 0;
					pGroups[nGroupCount].nLevel = 0;
					pGroups[nGroupCount].nTime = 0;
					nGroupCount++;
				}

				FTLM_API_GROUP_setCtrls(pGroups, nGroupCount);
			}
			else if (strcmp(pArgs[1], "dimm") == 0)
			{
				int	i, nGroupCount = 0;
				FTLM_GROUP_CTRL	pGroups[256];	

				if ((nArgc - 2) % 3 != 0)
				{
					printf("Invalid parameters\n");
					return	FTM_RET_OK;	
				}


				for(i = 2 ; i < nArgc ; i+=3)
				{
					pGroups[nGroupCount].nID = atoi(pArgs[i]);
					pGroups[nGroupCount].nCmd = 255;
					pGroups[nGroupCount].nLevel = atoi(pArgs[i+1]);
					pGroups[nGroupCount].nTime = atoi(pArgs[i+2]);
					nGroupCount++;
				}

				FTLM_API_GROUP_setCtrls(pGroups, nGroupCount);
			}
		
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CONSOLE_switch(FTM_INT nArgc, FTM_CHAR_PTR pArgs[])
{
	FTM_ID	pSwitchIDs[256];
	FTM_ULONG	ulCount = 0;

	switch(nArgc)
	{
	case	1:
		{
			if (FTLM_API_SWITCH_getList(pSwitchIDs, 256, &ulCount) != FTM_RET_OK)
			{
				printf("command error!\n");	
			}
			else
			{
				FTM_INT	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					printf("%08lx ", (FTM_ULONG)pSwitchIDs[i]);
				}
				printf("\n");
			}
		}
		break;

	case	2:
		{
			if (strcmp(pArgs[1], "groups") == 0)
			{
				FTLM_SWITCH_GROUP	pSwitchGroups[100];	
				int					i, j, nSwitchGroups = 0;
				if (FTLM_API_SWITCH_getGroups(pSwitchGroups, 100, &nSwitchGroups) == FTM_RET_OK)
				{
					printf("%8s %s\n", "SWITCH", "GROUPS");
					for(i = 0 ; i < nSwitchGroups ; i++)
					{
						printf("%-8d : ", pSwitchGroups[i].nID);
						for(j = 0 ; j < pSwitchGroups[i].nGroups ; j++)
						{
							printf("%-4d ", pSwitchGroups[i].pGroups[j]);
						}
						printf("\n");
					}
				}
			}
		}
		break;
	}

	return	FTM_RET_OK;
}
