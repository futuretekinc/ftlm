#include <string.h>
#include "ftm_types.h"
#include "ftm_mem.h"
#include "ftlm_config.h"
#include "ftlm_object.h"
#include "libconfig.h"

static FTM_INT	FTLM_CFG_ID_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTLM_CFG_LIGHT_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTLM_CFG_GROUP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTLM_CFG_SWITCH_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET FTLM_CFG_init(FTLM_CFG_PTR pCfg)
{
	if (pCfg == NULL)
	{
		return  FTM_RET_INVALID_ARGUMENTS;
	}

	strcpy(pCfg->xMQTT.pClientID, "12345678");
	strcpy(pCfg->xMQTT.pBrokerIP, "127.0.0.1");
	pCfg->xMQTT.usPort = 1883;
	pCfg->xMQTT.nKeepAlive = 60;

	strcpy(pCfg->xClient.xServer.pIP, "127.0.0.1");
	pCfg->xClient.xServer.usPort = 9877;

	pCfg->pLightList = FTM_LIST_create();
	FTM_LIST_setSeeker(pCfg->pLightList, FTLM_CFG_LIGHT_seeker);
	pCfg->pGroupList = FTM_LIST_create();
	FTM_LIST_setSeeker(pCfg->pGroupList, FTLM_CFG_GROUP_seeker);
	pCfg->pSwitchList = FTM_LIST_create();
	FTM_LIST_setSeeker(pCfg->pSwitchList, FTLM_CFG_SWITCH_seeker);

	return  FTM_RET_OK;
}

FTM_RET FTLM_CFG_final(FTLM_CFG_PTR pCfg)
{
	FTLM_LIGHT_CFG_PTR	pLight;
	FTLM_GROUP_CFG_PTR	pGroup;
	FTLM_SWITCH_CFG_PTR	pSwitch;

	if (pCfg == NULL)
	{
		return  FTM_RET_INVALID_ARGUMENTS;
	}

	if (pCfg->ulRefCount > 0)
	{
		return	FTM_RET_OK;	
	}

	printf("%s\n", __func__);
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

FTM_RET	FTLM_CFG_reference(FTLM_CFG_PTR pCfg)
{
	ASSERT(pCfg != NULL);
	
	pCfg->ulRefCount++;

	return	FTM_RET_OK;
}

FTM_RET	FTLM_CFG_unreference(FTLM_CFG_PTR pCfg)
{
	ASSERT(pCfg != NULL);

	if (pCfg->ulRefCount > 0)
	{
		pCfg->ulRefCount--;
	}

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

	pSection = config_lookup(&xCfg, "mqtt");
	if (pSection)
	{
		config_setting_t	*pIP;
		config_setting_t	*pPort;
		config_setting_t	*pKeepAlive;
		
		pIP = config_setting_get_member(pSection, "ip");
		if (pIP != NULL)
		{
			strncpy(pCfg->xMQTT.pBrokerIP, config_setting_get_string(pIP), FTLM_SERVER_IP_LEN-1);	
		}

		pPort = config_setting_get_member(pSection, "port");
		if (pPort != NULL)
		{
			pCfg->xMQTT.usPort = config_setting_get_int(pPort);
		}

		pKeepAlive = config_setting_get_member(pSection, "keepalive");
		if (pKeepAlive != NULL)
		{
			pCfg->xMQTT.nKeepAlive = config_setting_get_int(pKeepAlive);
		}
	}
	else
	{
		printf("can't find server section\n");	
	}

	pSection = config_lookup(&xCfg, "server");
	if (pSection)
	{
		config_setting_t	*pIP;
		config_setting_t	*pPort;
		
		pIP = config_setting_get_member(pSection, "ip");
		if (pIP != NULL)
		{
			strncpy(pCfg->xClient.xServer.pIP, config_setting_get_string(pIP), FTLM_SERVER_IP_LEN-1);	
		}
		else
		{
			printf("can't find ip\n");	
		}

		pPort = config_setting_get_member(pSection, "port");
		if (pPort != NULL)
		{
			pCfg->xClient.xServer.usPort = config_setting_get_int(pPort);
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
				FTLM_LIGHT_CFG_PTR		pLight;
				config_setting_t 	*pConfig;
				config_setting_t	*pID;
				config_setting_t	*pGWID;
				config_setting_t	*pName;
				config_setting_t	*pStatus;
				config_setting_t	*pLevel;
				config_setting_t	*pTime;


				pConfig = config_setting_get_elem(pLightConfigs, i);
				if (pConfig == NULL)
				{
						break;	
				}

				pID = config_setting_get_member(pConfig, "id");
				if (pID == NULL)
				{
						break;	
				}

				pGWID = config_setting_get_member(pConfig, "gwid");
				if (pGWID == NULL)
				{
						break;	
				}

				pLight = FTM_MEM_malloc(sizeof(FTLM_LIGHT_CFG));
				if (pLight == NULL)
				{
					break;	
				}
				memset(pLight, 0, sizeof(FTLM_LIGHT_CFG));

				pLight->xID = config_setting_get_int(pID);
				strncpy(pLight->pGatewayID, config_setting_get_string(pGWID), FTM_GATEWAY_ID_LEN);

				pName = config_setting_get_member(pConfig, "name");
				if (pName != NULL)
				{
					strncpy(pLight->pName, config_setting_get_string(pName), FTLM_NAME_MAX);
				}

				pStatus = config_setting_get_member(pConfig, "status");
				if (pStatus != NULL)
				{
					pLight->xStatus = config_setting_get_int(pStatus);
				}

				pLevel = config_setting_get_member(pConfig, "level");
				if (pLevel != NULL)
				{
					pLight->ulLevel = config_setting_get_int(pLevel);
				}

				pTime = config_setting_get_member(pConfig, "time");
				if (pTime != NULL)
				{
					pLight->ulTime = config_setting_get_int(pTime);
				}

				FTM_LIST_append(pCfg->pLightList, pLight);
			}
		}

		pGroupConfigs = config_setting_get_member(pSection, "groups");
		if (pGroupConfigs != NULL)
		{

			for(i = 0 ; ; i++)
			{
				FTLM_GROUP_CFG_PTR		pGroup;
				config_setting_t	*pArray;
				config_setting_t	*pID;
				config_setting_t	*pName;
				config_setting_t	*pStatus;
				config_setting_t	*pLevel;
				config_setting_t	*pTime;

				config_setting_t *pConfig;

				pConfig = config_setting_get_elem(pGroupConfigs, i);
				if (pConfig == NULL)
				{
						break;	
				}

				pID = config_setting_get_member(pConfig, "id");
				if (pID == NULL)
				{
						break;	
				}

				pGroup = FTM_MEM_malloc(sizeof(FTLM_GROUP_CFG));
				if (pGroup == NULL)
				{
					break;	
				}
				memset(pGroup, 0, sizeof(FTLM_GROUP_CFG));
				
				pGroup->xID = config_setting_get_int(pID);
				pGroup->pLightList = FTM_LIST_create();
				if (pGroup->pLightList == NULL)
				{
					FTM_MEM_free(pGroup);
					printf("Error : Can't allocate memory!\n");
					break;
				}
				FTM_LIST_setSeeker(pGroup->pLightList, FTLM_CFG_ID_seeker);


				pArray = config_setting_get_member(pConfig, "lights");
				if (pArray != NULL)
				{
					for(j = 0 ; j < FTLM_GROUP_MAX ; j++)
					{
						config_setting_t	*pLightID;
						FTM_ID				xLightID;
						
						pLightID = config_setting_get_elem(pArray, j);
						if (pLightID == 0)
						{
							break;
						}

						xLightID = config_setting_get_int(pLightID);
						if (FTM_LIST_get(pGroup->pLightList, (FTM_VOID_PTR)xLightID, NULL) != FTM_RET_OK)
						{
							FTM_LIST_append(pGroup->pLightList, (FTM_VOID_PTR)xLightID);
						}
					}
				}

				pName = config_setting_get_member(pConfig, "name");
				if (pName != NULL)
				{
					strncpy(pGroup->pName, config_setting_get_string(pName), FTLM_NAME_MAX);
				}

				pStatus = config_setting_get_member(pConfig, "status");
				if (pStatus != NULL)
				{
					pGroup->xStatus = config_setting_get_int(pStatus);
				}

				pLevel = config_setting_get_member(pConfig, "level");
				if (pLevel != NULL)
				{
					pGroup->ulLevel = config_setting_get_int(pLevel);
				}

				pTime = config_setting_get_member(pConfig, "time");
				if (pTime != NULL)
				{
					pGroup->ulTime = config_setting_get_int(pTime);
				}



				FTM_LIST_append(pCfg->pGroupList, pGroup);
			}
		}

		pSwitchConfigs = config_setting_get_member(pSection, "switchs");
		if (pSwitchConfigs != NULL)
		{

			for(i = 0 ; ; i++)
			{
				FTLM_SWITCH_CFG_PTR	pSwitch;
				config_setting_t 	*pConfig;
				config_setting_t	*pArray;
				config_setting_t	*pID;
				config_setting_t	*pName;


				pConfig = config_setting_get_elem(pSwitchConfigs, i);
				if (pConfig == NULL)
				{
						break;	
				}

				pID = config_setting_get_member(pConfig, "id");
				if (pID == NULL)
				{
						break;	
				}
				pSwitch = FTM_MEM_malloc(sizeof(FTLM_SWITCH_CFG));
				if (pSwitch == NULL)
				{
					break;	
				}
				memset(pSwitch, 0, sizeof(FTLM_SWITCH_CFG));
				pSwitch->pGroupList = FTM_LIST_create();
				if (pSwitch->pGroupList == NULL)
				{
					FTM_MEM_free(pSwitch);
					printf("Error : Can't allocate memory!\n");
					break;
				}

				pName = config_setting_get_member(pConfig, "name");
				if (pName != NULL)
				{
					strncpy(pSwitch->pName, config_setting_get_string(pName), FTLM_NAME_MAX);
				}

				FTM_LIST_setSeeker(pSwitch->pGroupList, FTLM_CFG_ID_seeker);

				pSwitch->xID = config_setting_get_int(pID);

				pArray = config_setting_get_member(pConfig, "groups");
				if (pArray != NULL)
				{
					for(j = 0 ; j < FTLM_GROUP_MAX ; j++)
					{
						FTM_ID				xGroupID;
						config_setting_t	*pGroupID;

						pGroupID= config_setting_get_elem(pArray, j);
						if (pGroupID == 0)
						{
							break;
						}

						xGroupID = config_setting_get_int(pGroupID);
						if (FTM_LIST_get(pSwitch->pGroupList, (FTM_VOID_PTR)xGroupID, NULL) != FTM_RET_OK)
						{
							FTM_LIST_append(pSwitch->pGroupList, (FTM_VOID_PTR)xGroupID);
						}
					}
				}

				FTM_LIST_append(pCfg->pSwitchList, pSwitch);
			}
		}
	}

	config_destroy(&xCfg);

	return  FTM_RET_OK;
}

FTM_RET FTLM_CFG_save(FTLM_CFG_PTR pCfg, FTM_CHAR_PTR pFileName)
{
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
		FTLM_LIGHT_CFG_PTR		pLight;

		FTM_LIST_iteratorStart(pCfg->pLightList);
		while(FTM_LIST_iteratorNext(pCfg->pLightList, (void **)&pLight) == FTM_RET_OK)
		{
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
			config_setting_set_int(pMember, pLight->xID);

			pMember = config_setting_add(pLightConfig, "name", CONFIG_TYPE_STRING);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_string(pMember, pLight->pName);

			pMember = config_setting_add(pLightConfig, "gwid", CONFIG_TYPE_STRING);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_string(pMember, pLight->pGatewayID);


			pMember = config_setting_add(pLightConfig, "Status", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->xStatus);

			pMember = config_setting_add(pLightConfig, "level", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->ulLevel);

			pMember = config_setting_add(pLightConfig, "time", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pLight->ulTime);
		}
	}

	pGroupConfigs = config_setting_add(pSection, "groups", CONFIG_TYPE_LIST);
	if (pGroupConfigs != NULL)
	{
		FTLM_GROUP_CFG_PTR		pGroup;

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
			config_setting_set_int(pMember, pGroup->xID);

			pMember = config_setting_add(pGroupConfig, "name", CONFIG_TYPE_STRING);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_string(pMember, pGroup->pName);


			pArray = config_setting_add(pGroupConfig, "lights", CONFIG_TYPE_ARRAY);
			if (pArray == NULL)
			{
				goto error;
			}

			if (pGroup->pLightList != NULL)
			{
				FTM_ID	xLightID;

				FTM_LIST_iteratorStart(pGroup->pLightList);
				while(FTM_LIST_iteratorNext(pGroup->pLightList, (FTM_VOID_PTR _PTR_)&xLightID) == FTM_RET_OK)
				{
					pMember = config_setting_add(pArray, "", CONFIG_TYPE_INT);
					if (pMember == 0)
					{
						goto error;
					}
				
					config_setting_set_int(pMember, xLightID);
				}
			}

			pMember = config_setting_add(pGroupConfig, "status", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->xStatus);

			pMember = config_setting_add(pGroupConfig, "level", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->ulLevel);

			pMember = config_setting_add(pGroupConfig, "time", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pGroup->ulTime);
		}
	}

	pSwitchConfigs = config_setting_add(pSection, "switchs", CONFIG_TYPE_LIST);
	if (pSwitchConfigs != NULL)
	{
		FTLM_SWITCH_CFG_PTR		pSwitch;

		FTM_LIST_iteratorStart(pCfg->pSwitchList);
		while(FTM_LIST_iteratorNext(pCfg->pSwitchList, (void **)&pSwitch) == FTM_RET_OK)
		{
			config_setting_t	*pArray;
			config_setting_t	*pMember;
			config_setting_t 	*pConfig;

			pConfig = config_setting_add(pSwitchConfigs, "", CONFIG_TYPE_GROUP);
			if (pConfig == NULL)
			{
				goto error;
			}
			
			pMember = config_setting_add(pConfig, "id", CONFIG_TYPE_INT);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_int(pMember, pSwitch->xID);

			pMember = config_setting_add(pConfig, "name", CONFIG_TYPE_STRING);
			if (pMember == NULL)
			{
				goto error;
			}
			config_setting_set_string(pMember, pSwitch->pName);


			pArray = config_setting_add(pConfig, "groups", CONFIG_TYPE_ARRAY);
			if (pArray == NULL)
			{
				goto error;
			}

			if (pSwitch->pGroupList != NULL)
			{
				FTM_ID	xGroupID;

				FTM_LIST_iteratorStart(pSwitch->pGroupList);
				while(FTM_LIST_iteratorNext(pSwitch->pGroupList, (FTM_VOID_PTR _PTR_)&xGroupID) == FTM_RET_OK)
				{
					pMember = config_setting_add(pArray, "", CONFIG_TYPE_INT);
					if (pMember == 0)
					{
						goto error;
					}
				
					config_setting_set_int(pMember, xGroupID);
				}
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
	FTM_ULONG		ulCount;
	FTLM_LIGHT_CFG_PTR	pLight;
	FTLM_GROUP_CFG_PTR	pGroup;
	FTLM_SWITCH_CFG_PTR	pSwitch;

	printf("\n<Gateway Configuration>\n");
	printf("%12s : %s\n", "ID", pCfg->pGatewayID);

	printf("\n<Server Configuratoin>\n");
	printf("%12s : %s\n", "IP Address", pCfg->xClient.xServer.pIP);
	printf("%12s : %d\n", "Port", pCfg->xClient.xServer.usPort);

	printf("\n<MQTT Configuratoin>\n");
	printf("%12s : %s\n", "Client ID", pCfg->xMQTT.pClientID);
	printf("%12s : %s\n", "IP Address", pCfg->xMQTT.pBrokerIP);
	printf("%12s : %d\n", "Port", pCfg->xMQTT.usPort);
	printf("%12s : %d\n", "Keep Alive", pCfg->xMQTT.usPort);

	printf("\n<Light Configuration>\n");
	ulCount = FTM_LIST_count(pCfg->pLightList);
	printf("%12s : %lu\n", "Count", ulCount);

	printf("%12s %12s %12s %12s %16s\n", "ID", "STATUS", "LEVEL", "DULATION", "NAME");
	FTM_LIST_iteratorStart(pCfg->pLightList);
	while(FTM_LIST_iteratorNext(pCfg->pLightList, (FTM_VOID_PTR _PTR_)&pLight) == FTM_RET_OK)
	{
		printf("    %08x", (unsigned int)pLight->xID);
		switch(pLight->xStatus)
		{
		case FTLM_LIGHT_STATUS_OFF: 	printf(" %12s",	"off"); 	break;
		case FTLM_LIGHT_STATUS_ON:		printf(" %12s",	"on"); 		break;
		default:						printf(" %12s",	"dimming"); break;
		}
		printf(" %12lu %12lu %16s\n", 	pLight->ulLevel, pLight->ulTime, pLight->pName);
	}

	printf("\n<Group Configuration>\n");
	ulCount = FTM_LIST_count(pCfg->pGroupList);
	printf("%12s : %lu\n", "count", ulCount);

	printf("%12s %12s %12s %12s %16s %12s\n", "ID", "STATUS", "LEVEL", "DULATION", "NAME", "LIGHTS");
	FTM_LIST_iteratorStart(pCfg->pGroupList);
	while(FTM_LIST_iteratorNext(pCfg->pGroupList, (FTM_VOID_PTR _PTR_)&pGroup) == FTM_RET_OK)
	{
		FTM_ID			xLightID;
		FTM_ULONG		ulCount;

		printf("    %08x", (unsigned int)pGroup->xID);
		switch(pGroup->xStatus)
		{
		case FTLM_LIGHT_STATUS_OFF: 	printf(" %12s",	"off"); 	break;
		case FTLM_LIGHT_STATUS_ON:		printf(" %12s",	"on"); 		break;
		default:						printf(" %12s",	"dimming"); break;
		}
		printf(" %12lu %12lu %16s ", 	pGroup->ulLevel, pGroup->ulTime, pGroup->pName);

		ulCount = FTM_LIST_count(pGroup->pLightList);
		printf(" %4lu ", ulCount);

		FTM_LIST_iteratorStart(pGroup->pLightList);
		while(FTM_LIST_iteratorNext(pGroup->pLightList, (FTM_VOID_PTR _PTR_)&xLightID) == FTM_RET_OK)
		{
			printf(" %08x", (unsigned int)xLightID);
		}
		printf("\n");
	}

	printf("\n<Switch Configuration>\n");
	ulCount = FTM_LIST_count(pCfg->pSwitchList);
	printf("%12s : %lu\n", "Count", ulCount);

	printf("%12s %16s %12s\n", "ID", "NAME", "GROUPS");
	FTM_LIST_iteratorStart(pCfg->pSwitchList);
	while(FTM_LIST_iteratorNext(pCfg->pSwitchList, (FTM_VOID_PTR _PTR_)&pSwitch) == FTM_RET_OK)
	{
		FTM_ID			xGroupID;
		FTM_ULONG		ulCount;

		printf("    %08x %16s", (unsigned int)pSwitch->xID, pSwitch->pName);
		ulCount = FTM_LIST_count(pSwitch->pGroupList);
		printf(" %4lu", ulCount);
		FTM_LIST_iteratorStart(pSwitch->pGroupList);
		while(FTM_LIST_iteratorNext(pSwitch->pGroupList, (FTM_VOID_PTR _PTR_)&xGroupID) == FTM_RET_OK)
		{
			printf(" %08x", (unsigned int)xGroupID);
		}
		printf("\n");
	}

	return	FTM_RET_OK;
}

FTM_ULONG	FTLM_CFG_LIGHT_count(FTLM_CFG_PTR pCfg)
{
	ASSERT(pCfg != NULL);

	return	FTM_LIST_count(pCfg->pLightList);
}

FTLM_LIGHT_CFG_PTR FTLM_CFG_LIGHT_get(FTLM_CFG_PTR pCfg, FTM_ID xLightID)
{
	FTLM_LIGHT_CFG_PTR	pLight = NULL;
	
	ASSERT(pCfg != NULL);

	if (FTM_LIST_get(pCfg->pLightList, (FTM_VOID_PTR)&xLightID, (FTM_VOID_PTR _PTR_)&pLight) != FTM_RET_OK)
	{
		return	NULL;
	}

	return	pLight;
}

FTLM_LIGHT_CFG_PTR FTLM_CFG_LIGHT_getAt(FTLM_CFG_PTR pCfg, FTM_ULONG ulIndex)
{
	FTLM_LIGHT_CFG_PTR	pLight = NULL;
	
	ASSERT(pCfg != NULL);

	if (FTM_LIST_getAt(pCfg->pLightList, ulIndex, (FTM_VOID_PTR _PTR_)&pLight) != FTM_RET_OK)
	{
		return	NULL;
	}

	return	pLight;
}

FTM_ULONG	FTLM_CFG_GROUP_count(FTLM_CFG_PTR pCfg)
{
	ASSERT(pCfg != NULL);

	return	FTM_LIST_count(pCfg->pGroupList);
}

FTLM_GROUP_CFG_PTR FTLM_CFG_GROUP_create(FTLM_CFG_PTR pCfg, FTM_ID xGroupID)
{
	FTLM_GROUP_CFG_PTR pGroup;

	if (pCfg == NULL)
	{
		return	NULL;
	}
	
	pGroup = FTLM_CFG_GROUP_get(pCfg, xGroupID);
	if (pGroup == NULL)
	{
		pGroup = (FTLM_GROUP_CFG_PTR)FTM_MEM_malloc(sizeof(FTLM_GROUP_CFG));
		if (pGroup != NULL)
		{
			pGroup->xID = xGroupID;
			pGroup->pLightList = FTM_LIST_create();
		}
	}

	if (pGroup != NULL)
	{
		FTM_LIST_append(pCfg->pGroupList, pGroup);
	}

	return	pGroup;
}

FTLM_GROUP_CFG_PTR FTLM_CFG_GROUP_get(FTLM_CFG_PTR pCfg, FTM_ID xGroupID)
{
	FTLM_GROUP_CFG_PTR	pGroup = NULL;
	
	if (pCfg == NULL)
	{
		return	NULL;
	}

	if (FTM_LIST_get(pCfg->pGroupList, (FTM_VOID_PTR)&xGroupID, (FTM_VOID_PTR _PTR_)&pGroup) != FTM_RET_OK)
	{
		return	NULL;
	}
	
	return	pGroup;
}

FTLM_GROUP_CFG_PTR FTLM_CFG_GROUP_getAt(FTLM_CFG_PTR pCfg, FTM_ULONG ulIndex)
{
	FTLM_GROUP_CFG_PTR	pGroup = NULL;
	
	ASSERT(pCfg != NULL);

	if (FTM_LIST_getAt(pCfg->pGroupList, ulIndex, (FTM_VOID_PTR _PTR_)&pGroup) != FTM_RET_OK)
	{
		return	NULL;
	}

	return	pGroup;
}

FTM_ULONG	FTLM_CFG_SWITCH_count(FTLM_CFG_PTR pCfg)
{
	ASSERT(pCfg != NULL);

	return	FTM_LIST_count(pCfg->pSwitchList);
}

FTLM_SWITCH_CFG_PTR FTLM_CFG_SWITCH_get(FTLM_CFG_PTR pCfg, FTM_ID xSwitchID)
{
	FTLM_SWITCH_CFG_PTR	pSwitch = NULL;
	
	if (pCfg == NULL)
	{
		return	NULL;
	}

	if (FTM_LIST_get(pCfg->pSwitchList, (FTM_VOID_PTR)&xSwitchID, (FTM_VOID_PTR _PTR_)&pSwitch) != FTM_RET_OK)
	{
		return	NULL;
	}
	
	return	pSwitch;
}

FTLM_SWITCH_CFG_PTR FTLM_CFG_SWITCH_getAt(FTLM_CFG_PTR pCfg, FTM_ULONG ulIndex)
{
	FTLM_SWITCH_CFG_PTR	pSwitch = NULL;
	
	ASSERT(pCfg != NULL);

	if (FTM_LIST_getAt(pCfg->pSwitchList, ulIndex, (FTM_VOID_PTR _PTR_)&pSwitch) != FTM_RET_OK)
	{
		return	NULL;
	}

	return	pSwitch;
}

FTM_INT	FTLM_CFG_ID_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	return	((FTM_ID)pElement == (FTM_ID)pIndicator);
}

FTM_INT	FTLM_CFG_LIGHT_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT((pElement != NULL) && (pIndicator != NULL));

	return	(((FTLM_LIGHT_CFG_PTR)pElement)->xID == *(FTM_ID_PTR)pIndicator);
}

FTM_INT	FTLM_CFG_GROUP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT((pElement != NULL) && (pIndicator != NULL));

	return	(((FTLM_GROUP_CFG_PTR)pElement)->xID == *(FTM_ID_PTR)pIndicator);
}

FTM_INT	FTLM_CFG_SWITCH_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT((pElement != NULL) && (pIndicator != NULL));

	return	(((FTLM_SWITCH_CFG_PTR)pElement)->xID == *(FTM_ID_PTR)pIndicator);
}
