#ifndef	__FTLM_H__
#define	__FTLM_H__

FTM_RET FTLM_groupCtrl(FTLM_GROUP_PTR pGroup, FTM_ULONG ulCmd, FTM_ULONG ulLevel, FTM_ULONG ulTime);
FTM_RET	FTLM_lightCtrl(FTLM_LIGHT_PTR pLight, FTM_ULONG ulCmd, FTM_ULONG ulLevel, FTM_ULONG ulTime);
FTM_RET	FTLM_lightCtrls(FTLM_LIGHT_CTRL_PTR pLights, FTM_ULONG ulCount);

#endif

