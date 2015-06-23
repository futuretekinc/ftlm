#ifndef	FTLM_SERVER_API_H
#define	FTLM_SERVER_API_H

typedef	struct
{
	unsigned char	nID;
	char			pName[256];
	unsigned char	nCmd;
	unsigned char	nLevel;
	unsigned char	nTime;
}	FTLM_LIGHT_INFO, * FTLM_LIGHT_INFO_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned char	nCmd;
	unsigned char	nLevel;
	unsigned char	nTime;
}	FTLM_LIGHT_CTRL, * FTLM_LIGHT_CTRL_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned int	nGroups;
	unsigned char	pGroups[8];
}	FTLM_LIGHT_GROUP, * FTLM_LIGHT_GROUP_PTR;

typedef	struct
{
	unsigned char	nID;
	char			pName[256];
	unsigned char	nCmd;
	unsigned char	nLevel;
	unsigned char	nTime;

	unsigned char	nLight;
	unsigned char	pLightIDs[255];
}	FTLM_GROUP_INFO, * FTLM_GROUP_INFO_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned char	nCmd;
	unsigned char	nLevel;
	unsigned char	nTime;
}	FTLM_GROUP_CTRL, * FTLM_GROUP_CTRL_PTR;

typedef	struct
{
	unsigned char	nID;
	char			pName[256];

	unsigned char	nGroup;
	unsigned char	pGroupIDs[255];
}	FTLM_SWITCH_INFO, * FTLM_SWITCH_INFO_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned int	nGroups;
	unsigned char	pGroups[8];
}	FTLM_SWITCH_GROUP, * FTLM_SWITCH_GROUP_PTR;

int	FTLM_API_init(void);

int	FTLM_API_CFG_save(char *pName);

int	FTLM_API_LIGHT_getList(unsigned long * pLightIDs, unsigned long ulMaxCount, unsigned long * pulCount);
int	FTLM_API_LIGHT_getInfo(unsigned long xID, FTLM_LIGHT_INFO_PTR pInfo);
int	FTLM_API_LIGHT_setCtrls(FTLM_LIGHT_CTRL_PTR pLightCtrls, int nLightCtrls);
int	FTLM_API_LIGHT_getCtrls(FTLM_LIGHT_CTRL_PTR pLightCtrls, int nLightCtrls, int *pnLightCtrls);
int	FTLM_API_LIGHT_setGroups(FTLM_LIGHT_GROUP_PTR pLightGroups, int nLightGroups);
int	FTLM_API_LIGHT_getGroups(FTLM_LIGHT_GROUP_PTR pLightgroupss, int nMaxLightGroups, int *pnLightGroups);

int	FTLM_API_GROUP_getList(unsigned long * pGroupIDs, unsigned long ulMaxCount, unsigned long * pulCount);
int	FTLM_API_GROUP_getInfo(unsigned long xID, FTLM_GROUP_INFO_PTR pInfo);
int	FTLM_API_GROUP_setCtrls(FTLM_GROUP_CTRL_PTR pGroupCtrls, int nGroupCtrls);
int	FTLM_API_GROUP_getCtrls(FTLM_GROUP_CTRL_PTR pGroupCtrls, int nMaxGroupCtrls, int *pnGroupCtrls);

int	FTLM_API_SWITCH_getList(unsigned long * pSwitchIDs, unsigned long ulMaxCount, unsigned long * pulCount);
int	FTLM_API_sWITCH_getInfo(unsigned long xID, FTLM_SWITCH_INFO_PTR pInfo);
int	FTLM_API_SWITCH_setGroups(FTLM_SWITCH_GROUP_PTR pSwitchGroups, int nSwitches);
int	FTLM_API_SWITCH_getGroups(FTLM_SWITCH_GROUP_PTR pSwitchGroups, int nMaxSwitches, int *pnSwitches);

#endif
