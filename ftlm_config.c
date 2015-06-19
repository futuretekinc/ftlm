#include <string.h>
#include "ftlm_config.h"
#include "ftm_mem.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "libconfig.h"
#include "ftlm_device.h"

FTM_RET FTLM_CFG_init(FTLM_CFG_PTR pCfg)
{
	if (pCfg == NULL)
	{
		return  FTM_RET_INVALID_ARGUMENTS;
	}

	strcpy(pCfg->xNetwork.pServerIP, "127.0.0.1");
	pCfg->xNetwork.usPort = 9877;

	pCfg->pLightList = FTM_LIST_create();
	pCfg->pGroupList = FTM_LIST_create();
	pCfg->pSwitchList = FTM_LIST_create();

	return  FTM_RET_OK;
}

FTM_RET FTLM_CFG_final(FTLM_CFG_PTR pCfg)
{
	FTLM_LIGHT_PTR	pLight;
	FTLM_GROUP_PTR	pGroup;
	FTLM_SWITCH_PTR	pSwitch;

	if (pCfg == NULL)
	{
		return  FTM_RET_INVALID_ARGUMENTS;
	}

	FTM_LIST_iteratorStart(pCfg->pLightList);
	while(FTM_LIST_iteratorNext(pCfg->pLightList, (void **)&pLight) == FTM_RET_OK)
	{
		FTM_MEM_free(pLight);
	}
	FTM_LIST_destroy(pCfg->pLightList);

	FTM_LIST_iteratorStart(pCfg->pGroupList);
	while(FTM_LIST_iteratorNext(pCfg->pGroupList, (void **)&pGroup) == FTM_RET_OK)
	{
		FTM_MEM_free(pGroup);
	}
	FTM_LIST_destroy(pCfg->pGroupList);

	FTM_LIST_iteratorStart(pCfg->pSwitchList);
	while(FTM_LIST_iteratorNext(pCfg->pSwitchList, (void **)&pSwitch) == FTM_RET_OK)
	{
		FTM_MEM_free(pSwitch);
	}
	FTM_LIST_destroy(pCfg->pSwitchList);

	return	FTM_RET_OK;
}

FTM_RET FTLM_CFG_load(FTLM_CFG_PTR pCfg, FTM_CHAR_PTR pFileName)
{
	config_t            xCfg;
	config_setting_t    *pSection;

	if ((pCfg == NULL) || (pFileName == NULL))
	{
			return  FTM_RET_INVALID_ARGUMENTS;  
	}

	config_init(&xCfg);

	if (CONFIG_TRUE != config_read_file(&xCfg, pFileName))
	{
			ERROR("Configuration loading failed.[FILE = %s]\n", pFileName);
			return  FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xCfg, "server");
	if (pSection)
	{
		config_setting_t	*pIP;
		config_setting_t	*pPort;
		
		pIP = config_setting_get_member(pSection, "ip");
		if (pIP != NULL)
		{
			strncpy(pCfg->xNetwork.pServerIP, config_setting_get_string(pIP), FTLM_SERVER_IP_LEN-1);	
		}
		else
		{
			printf("can't find ip\n");	
		}

		pPort = config_setting_get_member(pSection, "port");
		if (pPort != NULL)
		{
			pCfg->xNetwork.usPort = config_setting_get_int(pPort);
		}
		else
		{
			printf("can't find port\n");	
		}
	}
	else
	{
		printf("can't find server section\n");	
	}

	pSection = config_lookup(&xCfg, "config");
	if (pSection)
	{
		int	i, j;
		config_setting_t	*pConfigs;
		config_setting_t	*pLightConfigs;
		config_setting_t	*pGroupConfigs;
		config_setting_t	*pSwitchConfigs;

		pConfigs = config_setting_get_member(pSection, "gatewayid");
		if (pConfigs != NULL)
		{
			strncpy(pCfg->pGatewayID, config_setting_get_string(pConfigs), 10);
		}

		pLightConfigs = config_setting_get_member(pSection, "lights");
		if (pLightConfigs != NULL)
		{

			for(i = 0 ; ; i++)
			{
				FTLM_LIGHT_PTR	pLight;
				FTLM_LIGHT		xLight;
				config_setting_t	*pArray;
				config_setting_t	*pMember;

				config_setting_t *pLightConfig;

				memset(&xLight, 0, sizeof(xLight));

				pLightConfig = config_setting_get_elem(pLightConfigs, i);
				if (pLightConfig == NULL)
				{
						break;	
				}

				pMember = config_setting_get_member(pLightConfig, "id");
				if (pMember == NULL)
				{
						break;	
				}
				xLight.nID = config_setting_get_int(pMember);

				pMember = config_setting_get_member(pLightConfig, "nodeid");
				if (pMember == NULL)
				{
						break;	
				}
				strncpy(xLight.pNodeID, config_setting_get_string(pMember), 32);

				pArray = config_setting_get_member(pLightConfig, "groups");
				if (pArray != NULL)
				{
					for(j = 0 ; j < FTLM_GROUP_MAX ; j++)
					{
						pMember = config_setting_get_elem(pArray, j);
						if (pMember == 0)
						{
							break;
						}

						xLight.pGroupIDs[xLight.nGroupIDs++] = config_setting_get_int(pMember);
					}
				}

				pMember = config_setting_get_member(pLightConfig, "status");
				if (pMember != NULL)
				{
					xLight.nStatus = config_setting_get_int(pMember);
				}

				pMember = config_setting_get_member(pLightConfig, "level");
				if (pMember != NULL)
				{
					xLight.nLevel = config_setting_get_int(pMember);
				}

				pMember = config_setting_get_member(pLightConfig, "dulation");
				if (pMember != NULL)
				{
					xLight.nDulationTime  = config_setting_get_int(pMember);
				}


				pLight = FTM_MEM_malloc(sizeof(FTLM_LIGHT));
				if (pLight == NULL)
				{
					break;	
				}

				memcpy(pLight, &xLight, sizeof(FTLM_LIGHT));

				FTM_LIST_append(pCfg->pLightList, pLight);
			}
		}

		pGroupConfigs = config_setting_get_member(pSection, "groups");
		if (pGroupConfigs != NULL)
		{

			for(i = 0 ; ; i++)
			{
				FTLM_GROUP_PTR	pGroup;
				FTLM_GROUP		xGroup;
				config_setting_t	*pArray;
				config_setting_t	*pMember;

				config_setting_t *pGroupConfig;

				memset(&xGroup, 0, sizeof(xGroup));

				pGroupConfig = config_setting_get_elem(pGroupConfigs, i);
				if (pGroupConfig == NULL)
				{
						break;	
				}

				pMember = config_setting_get_member(pGroupConfig, "id");
				if (pMember == NULL)
				{
						break;	
				}
				xGroup.nID = config_setting_get_int(pMember);


				pArray = config_setting_get_member(pGroupConfig, "lights");
				if (pArray != NULL)
				{
					for(j = 0 ; j < FTLM_GROUP_MAX ; j++)
					{
						pMember = config_setting_get_elem(pArray, j);
						if (pMember == 0)
						{
							break;
						}

						xGroup.pLightIDs[xGroup.nLightIDs++] = config_setting_get_int(pMember);
					}
				}

				pMember = config_setting_get_member(pGroupConfig, "status");
				if (pMember != NULL)
				{
					xGroup.nStatus = config_setting_get_int(pMember);
				}

				pMember = config_setting_get_member(pGroupConfig, "level");
				if (pMember != NULL)
				{
					xGroup.nLevel = config_setting_get_int(pMember);
				}

				pMember = config_setting_get_member(pGroupConfig, "dimming");
				if (pMember != NULL)
				{
					xGroup.nDimmingTime = config_setting_get_int(pMember);
				}


				pGroup = FTM_MEM_malloc(sizeof(FTLM_GROUP));
				if (pGroup == NULL)
				{
					break;	
				}

				memcpy(pGroup, &xGroup, sizeof(FTLM_GROUP));

				FTM_LIST_append(pCfg->pGroupList, pGroup);
			}
		}

		pSwitchConfigs = config_setting_get_member(pSection, "switchs");
		if (pSwitchConfigs != NULL)
		{

			for(i = 0 ; ; i++)
			{
				FTLM_SWITCH_PTR	pSwitch;
				FTLM_SWITCH		xSwitch;
				config_setting_t	*pArray;
				config_setting_t	*pMember;

				config_setting_t *pSwitchConfig;

				memset(&xSwitch, 0, sizeof(xSwitch));

				pSwitchConfig = config_setting_get_elem(pSwitchConfigs, i);
				if (pSwitchConfig == NULL)
				{
						break;	
				}

				pMember = config_setting_get_member(pSwitchConfig, "id");
				if (pMember == NULL)
				{
						break;	
				}
				xSwitch.nID = config_setting_get_int(pMember);

				pArray = config_setting_get_member(pSwitchConfig, "groups");
				if (pArray != NULL)
				{
					for(j = 0 ; j < FTLM_GROUP_MAX ; j++)
					{
						pMember = config_setting_get_elem(pArray, j);
						if (pMember == 0)
						{
							break;
						}

						xSwitch.pGroupIDs[xSwitch.nGroupIDs++] = config_setting_get_int(pMember);
					}
				}

				pSwitch = FTM_MEM_malloc(sizeof(FTLM_SWITCH));
				if (pSwitch == NULL)
				{
					break;	
				}

				memcpy(pSwitch, &xSwitch, sizeof(FTLM_SWITCH));

				FTM_LIST_append(pCfg->pSwitchList, pSwitch);
			}
		}
	}

	config_destroy(&xCfg);

	return  FTM_RET_OK;
}

FTM_RET FTLM_CFG_save(FTLM_CFG_PTR pCfg, FTM_CHAR_PTR pFileName)
{
	int					j;
	config_t            xCfg;
	config_setting_t    *pRoot;
	config_setting_t    *pSection;
	config_setting_t	*pLightConfigs;
	config_setting_t	*pGroupConfigs;
	config_setting_t	*pSwitchConfigs;

	if ((pCfg == NULL) || (pFileName == NULL))
	{
			return  FTM_RET_INVALID_ARGUMENTS;  
	}

	config_init(&xCfg);

	pRoot = config_root_setting(&xCfg);
	if (pRoot == NULL)
	{
		goto error;
	}

	pSection = config_setting_add(pRoot, "config", CONFIG_TYPE_GROUP);
	if (pSection == NULL)
	{
		goto error;
	}


	pLightConfigs = config_setting_add(pSection, "lights", CONFIG_TYPE_LIST);
	if (pLightConfigs != NULL)
	{
		FTLM_LIGHT_PTR		pLight;

		FTM_LIST_iteratorStart(pCfg->pLightList);
		while(FTM_LIST_iteratorNext(pCfg->pLightList, (void **)&pLight) == FTM_RET_OK)
		{
			config_setting_t	*pArray;
			config_setting_t	*pMember;
			config_setting_t 	*pLightConfig;

			pLightConfig = config_setting_add(pLightConfigs, "", CONFIG_TYPE_GROUP);
			if (pLightConfig == NULL)
			{
				goto error;
			}
			
			pMember = config_setting_add(pLightConfig, "id", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->nID);


			pArray = config_setting_add(pLightConfig, "groups", CONFIG_TYPE_ARRAY);
			if (pArray == NULL)
			{
				goto error;
			}

			for(j = 0 ; j < pLight->nGroupIDs ; j++)
			{
				pMember = config_setting_add(pArray, "groupid", CONFIG_TYPE_INT);
				if (pMember == 0)
				{
					goto error;
				}

				config_setting_set_int(pMember, pLight->pGroupIDs[j]);
			}

			pMember = config_setting_add(pLightConfig, "Status", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->nStatus);

			pMember = config_setting_add(pLightConfig, "level", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->nLevel);

			pMember = config_setting_add(pLightConfig, "dulation", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->nDulationTime);
		}
	}

	pGroupConfigs = config_setting_add(pSection, "groups", CONFIG_TYPE_LIST);
	if (pGroupConfigs != NULL)
	{
		FTLM_GROUP_PTR		pGroup;

		FTM_LIST_iteratorStart(pCfg->pGroupList);
		while(FTM_LIST_iteratorNext(pCfg->pGroupList, (void **)&pGroup) == FTM_RET_OK)
		{
			config_setting_t	*pArray;
			config_setting_t	*pMember;
			config_setting_t 	*pGroupConfig;

			pGroupConfig = config_setting_add(pGroupConfigs, "", CONFIG_TYPE_GROUP);
			if (pGroupConfig == NULL)
			{
				goto error;
			}
			
			pMember = config_setting_add(pGroupConfig, "id", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->nID);


			pArray = config_setting_add(pGroupConfig, "lights", CONFIG_TYPE_ARRAY);
			if (pArray == NULL)
			{
				goto error;
			}

			for(j = 0 ; j < pGroup->nLightIDs ; j++)
			{
				pMember = config_setting_add(pArray, "lightid", CONFIG_TYPE_INT);
				if (pMember == 0)
				{
					goto error;
				}

				config_setting_set_int(pMember, pGroup->pLightIDs[j]);
			}

			pMember = config_setting_add(pGroupConfig, "status", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->nStatus);

			pMember = config_setting_add(pGroupConfig, "level", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->nLevel);

			pMember = config_setting_add(pGroupConfig, "dulation", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->nDimmingTime);
		}
	}

	pSwitchConfigs = config_setting_add(pSection, "switchs", CONFIG_TYPE_LIST);
	if (pSwitchConfigs != NULL)
	{
		FTLM_SWITCH_PTR		pSwitch;

		FTM_LIST_iteratorStart(pCfg->pSwitchList);
		while(FTM_LIST_iteratorNext(pCfg->pSwitchList, (void **)&pSwitch) == FTM_RET_OK)
		{
			config_setting_t	*pArray;
			config_setting_t	*pMember;
			config_setting_t 	*pSwitchConfig;

			pSwitchConfig = config_setting_add(pSwitchConfigs, "", CONFIG_TYPE_GROUP);
			if (pSwitchConfig == NULL)
			{
				goto error;
			}
			
			pMember = config_setting_add(pSwitchConfig, "id", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pSwitch->nID);


			pArray = config_setting_add(pSwitchConfig, "groups", CONFIG_TYPE_ARRAY);
			if (pArray == NULL)
			{
				goto error;
			}

			for(j = 0 ; j < pSwitch->nGroupIDs ; j++)
			{
				pMember = config_setting_add(pArray, "groupid", CONFIG_TYPE_INT);
				if (pMember == 0)
				{
					goto error;
				}

				config_setting_set_int(pMember, pSwitch->pGroupIDs[j]);
			}
		}
	}

	if (CONFIG_TRUE != config_write_file(&xCfg, pFileName))
	{
			ERROR("Configuration saving failed.[FILE = %s]\n", pFileName);
			goto error;
	}

	return	FTM_RET_OK;	

error:
	config_destroy(&xCfg);

	return	FTM_RET_ERROR;	
}

FTM_RET	FTLM_CFG_print(FTLM_CFG_PTR pCfg)
{	
	int	i, j;
	FTM_ULONG	ulCount;

	printf("<Gateway Configuration>\n");
	printf("%12s : %s\n", "GATEWAY ID", pCfg->pGatewayID);

	printf("<Network Configuratoin>\n");
	printf("%12s : %s\n", "SERVER", pCfg->xNetwork.pServerIP);
	printf("%12s : %d\n", "PORT", pCfg->xNetwork.usPort);

	printf("\n<Light Configuration>\n");

	FTM_LIST_count(pCfg->pLightList, &ulCount);
	for(i = 0 ; i < ulCount; i++)
	{
		FTLM_LIGHT_PTR	pLight;
		if (FTM_LIST_getAt(pCfg->pLightList, i, (void **)&pLight) != FTM_RET_OK)
		{
			break;	
		}

		printf("%12s : %d\n", "ID", pLight->nID);
		printf("%12s : %d\n", "GROUPS", pLight->nGroupIDs);
		for(j = 0 ; j < pLight->nGroupIDs ; j++)
		{
			printf("%8d : %d\n", j+1, pLight->pGroupIDs[j]);
		}

		switch(pLight->nStatus)
		{
		case 0x00: 	printf("%12s : %s\n", "STATUS", 	"off"); break;
		case 0xFF:	printf("%12s : %s\n", "STATUS", 	"dimming"); break;
		default:	printf("%12s : %s\n", "STATUS", 	"on"); break;
		}
		printf("%12s : %d\n", "LEVEL", 	pLight->nLevel);
		printf("%12s : %d\n", "DULATION",pLight->nDulationTime);
	}

	printf("\n<Group Configuration>\n");
	FTM_LIST_count(pCfg->pGroupList, &ulCount);
	for(i = 0 ; i < ulCount; i++)
	{
		FTLM_GROUP_PTR	pGroup;
		if (FTM_LIST_getAt(pCfg->pGroupList, i, (void **)&pGroup) != FTM_RET_OK)
		{
			break;	
		}

		printf("%12s : %d\n", "ID", pGroup->nID);
		printf("%12s : %d\n", "GROUPS", pGroup->nLightIDs);
		for(j = 0 ; j < pGroup->nLightIDs ; j++)
		{
			printf("%8d : %d\n", j+1, pGroup->pLightIDs[j]);
		}
		switch(pGroup->nStatus)
		{
		case 0x00: 	printf("%12s : %s\n", "STATUS", 	"off"); break;
		case 0xFF:	printf("%12s : %s\n", "STATUS", 	"dimming"); break;
		default:	printf("%12s : %s\n", "STATUS", 	"on"); break;
		}
		printf("%12s : %d\n", "LEVEL", 	pGroup->nLevel);
		printf("%12s : %d\n", "DULATION",pGroup->nDimmingTime);
	}

	printf("\n<Switch Configuration>\n");
	FTM_LIST_count(pCfg->pSwitchList, &ulCount);
	for(i = 0 ; i < ulCount; i++)
	{
		FTLM_SWITCH_PTR	pSwitch;
		if (FTM_LIST_getAt(pCfg->pSwitchList, i, (void **)&pSwitch) != FTM_RET_OK)
		{
			break;	
		}

		printf("%12s : %d\n", "ID", pSwitch->nID);
		printf("%12s : %d\n", "GROUPS", pSwitch->nGroupIDs);
		for(j = 0 ; j < pSwitch->nGroupIDs ; j++)
		{
			printf("%8d : %d\n", j+1, pSwitch->pGroupIDs[j]);
		}
	}

	return	FTM_RET_OK;
}

FTLM_LIGHT_PTR FTLM_CFG_LIGHT_get(FTLM_CFG_PTR pCfg, FTLM_ID nLightID)
{
	FTLM_LIGHT_PTR	pLight = NULL;
	
	if (pCfg == NULL)
	{
		return	NULL;
	}

	FTM_LIST_iteratorStart(pCfg->pLightList);
	while(FTM_LIST_iteratorNext(pCfg->pLightList, (void **)&pLight) == FTM_RET_OK)
	{
		if (pLight->nID == nLightID)
		{
			return	pLight;
		}
	}

	return	NULL;
}

FTLM_GROUP_PTR FTLM_CFG_GROUP_create(FTLM_CFG_PTR pCfg, FTLM_ID nGroupID)
{
	FTLM_GROUP_PTR pGroup;

	if (pCfg == NULL)
	{
		return	NULL;
	}
	
	pGroup = FTLM_CFG_GROUP_get(pCfg, nGroupID);
	if (pGroup == NULL)
	{
		pGroup = (FTLM_GROUP_PTR)FTM_MEM_malloc(sizeof(FTLM_GROUP));
		if (pGroup != NULL)
		{
			pGroup->nID = nGroupID;
			pGroup->nLightIDs = 0;
		}
	}

	if (pGroup != NULL)
	{
		FTM_LIST_append(pCfg->pGroupList, pGroup);
	}

	return	pGroup;
}

FTLM_GROUP_PTR FTLM_CFG_GROUP_get(FTLM_CFG_PTR pCfg, FTLM_ID nGroupID)
{
	FTLM_GROUP_PTR	pGroup = NULL;
	
	if (pCfg == NULL)
	{
		return	NULL;
	}

	FTM_LIST_iteratorStart(pCfg->pGroupList);
	while(FTM_LIST_iteratorNext(pCfg->pGroupList, (void **)&pGroup) == FTM_RET_OK)
	{
		if (pGroup->nID == nGroupID)
		{
			return	pGroup	;
		}
	}

	return	NULL;
}


