#ifndef	__FTLM_MSG_H__
#define	__FTLM_MSG_H__

#define	FTLM_GWID_LEN	10

#define	FTLM_MSG_STX		0x58
#define	FTLM_MSG_ETX1	0x64
#define	FTLM_MSG_ETX2	0x36

#define	FTLM_CMD_INVALID				0xFFFF
#define	FTLM_CMD_RESET				0x0000
#define	FTLM_CMD_GROUP_CTRL			0x0700
#define	FTLM_CMD_LIGHT_CTRL			0x0710
#define	FTLM_CMD_GROUP_SET			0x0720
#define	FTLM_CMD_SWITCH_GROUP_SET	0x0730
#define	FTLM_CMD_STATUS_GET			0x0740
#define	FTLM_CMD_GROUP_MAPPING_GET	0x0741
#define	FTLM_CMD_SWITCH_MAPPING_GET	0x0742
#define	FTLM_CMD_GROUP_STATUS_GET	0x0743
#define	FTLM_CMD_LIGHT_STATUS_GET	0x0744

typedef struct
{
	unsigned char	nID;
	unsigned char	nCmd;
	unsigned char	nLevel;
	unsigned char	nDimmingTime; 
}	FTLM_GROUP_CTRL, _PTR_ FTLM_GROUP_CTRL_PTR;	

typedef	struct
{
	unsigned char		nGroups;
	FTLM_GROUP_CTRL	pGroups[];
}	FTLM_GROUP_CTRL_PARAM, _PTR_ FTLM_GROUP_CTRL_PARAM_PTR;

typedef struct
{
	unsigned char	nID;
	unsigned char	nCmd;
	unsigned char	nLevel;
	unsigned char	nDulationTime; 
}	FTLM_LIGHT_CTRL, _PTR_ FTLM_LIGHT_CTRL_PTR;	

typedef	struct
{
	unsigned char		nLights;
	FTLM_LIGHT_CTRL	pLights[];
}	FTLM_LIGHT_CTRL_PARAM, _PTR_ FTLM_LIGHT_CTRL_PARAM_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned char	nGroups;
	unsigned char	pGroups[];
}	FTLM_GROUP_SET, _PTR_ FTLM_GROUP_SET_PTR;

typedef	struct
{
	unsigned char		nSets;
	FTLM_GROUP_SET	pSets[];
}	FTLM_GROUP_SET_PARAM, _PTR_ FTLM_GROUP_SET_PARAM_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned char	nGroups;
	unsigned char	pGroups[];
}	FTLM_SWITCH_GROUP_SET, _PTR_ FTLM_SWITCH_GROUP_SET_PTR;

typedef	struct
{
	unsigned char	nSets;
	FTLM_SWITCH_GROUP_SET	pSets[];
}	FTLM_SWITCH_GROUP_SET_PARAM, _PTR_ FTLM_SWITCH_GROUP_SET_PARAM_PTR;

typedef	struct
{
	unsigned char	pGatewayID[FTLM_GWID_LEN];
	unsigned char	nMode;
} FTLM_STATUS_GET_PARAM, _PTR_ FTLM_STATUS_GET_PARAM_PTR;

typedef union	
{
	FTLM_GROUP_CTRL_PARAM	xGroupCtrl;
	FTLM_LIGHT_CTRL_PARAM	xLightCtrl;
	FTLM_STATUS_GET_PARAM	xStatusGet;
}	FTLM_REQUEST_PARAM, _PTR_ FTLM_REQUEST_PARAM_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned char	nGroups;
	unsigned char	pGroups[];
} FTLM_GROUP_MAPPING, _PTR_ FTLM_GROUP_MAPPING_PTR;

typedef	struct
{
	unsigned char			nSets;
	FTLM_GROUP_MAPPING	pSets[];	
} FTLM_GROUP_MAPPING_PARAM, _PTR_ FTLM_GROUP_MAPPING_PARAM_PTR;

typedef	struct
{
	unsigned char	nID;
	unsigned char	nStatus;
	unsigned char	nLevel;
	unsigned char	nDimmingTime; 
} FTLM_GROUP_STATUS, _PTR_ FTLM_GROUP_STATUS_PTR;	

typedef	struct
{
	unsigned char			nGroups;
	FTLM_GROUP_STATUS	pGroups[];
} FTLM_GROUP_STATUS_PARAM, _PTR_ FTLM_GROUP_STATUS_PARAM_PTR;

typedef struct
{
	unsigned char	nID;
	unsigned char	nStatus;
	unsigned char	nLevel;
	unsigned char	nDulationTime; 
}	_FTLM_LIGHT_STATUS, _PTR_ _FTLM_LIGHT_STATUS_PTR;	

typedef	struct
{
	unsigned char			nLights;
	_FTLM_LIGHT_STATUS	pLights[];
}	FTLM_LIGHT_STATUS_PARAM, _PTR_ FTLM_LIGHT_STATUS_PARAM_PTR;

typedef union
{
	FTLM_GROUP_MAPPING_PARAM	xGroupMapping;
	FTLM_LIGHT_STATUS_PARAM	xLightStatus;
	FTLM_GROUP_STATUS_PARAM	xGroupStatus;
}	FTLM_RESPONSE_PARAM, _PTR_ FTLM_RESPONSE_PARAM_PTR;

typedef	struct
{
	int				nID;

	int				nRecvLen;
	unsigned char	pRecvBuff[2048];

	int				nRespLen;
	unsigned char	pRespBuff[2048];

	unsigned char	*pGatewayID;

	unsigned short	nCmd;
	FTLM_REQUEST_PARAM_PTR	pReqParam;

	unsigned char	nRet;
	int				nRespParamLen;
	FTLM_RESPONSE_PARAM_PTR	pRespParam;
}	FTLM_FRAME, _PTR_ FTLM_FRAME_PTR;

FTM_RET	FTLM_FRAME_parser(FTLM_FRAME_PTR pFrame);
FTM_RET	FTLM_FRAME_dump(FTLM_FRAME_PTR pFrame);

#endif
