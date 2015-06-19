#ifndef	__LCGW_MQTT_H__
#define __LCGW_MQTT_H__

#include "ftm.h"
#include <mosquitto.h>
#include <pthread.h>

typedef	struct
{
	char			 	pClientID[24];
	char				pBrokerIP[256];
	unsigned short		usPort;
	int					nKeepAlive;
	void 				(*CB_connect)(struct mosquitto *pMOSQ, void *pObj, int nResult);
	void 				(*CB_disconnect)(struct mosquitto *pMOSQ, void *pObj, int nResult);
	void 				(*CB_message)(struct mosquitto *pMOSQ, void *pObj, const struct mosquitto_message *pMessage);

	int					nMessageID;
}	FTM_MQTT_CONFIG, _PTR_ FTM_MQTT_CONFIG_PTR;

typedef struct
{
	FTM_MQTT_CONFIG	xConfig;

	int					bRun;
	struct mosquitto*	pMOSQ;
	pthread_t			hThread;
}	FTM_MQTT, _PTR_ FTM_MQTT_PTR;


FTM_MQTT_PTR FTM_MQTT_create(FTM_MQTT_CONFIG_PTR pMQTT);
FTM_RET FTM_MQTT_destroy(FTM_MQTT_PTR pMQTT);
FTM_RET	FTM_MQTT_start(FTM_MQTT_PTR pMQTT);
FTM_RET	FTM_MQTT_stop(FTM_MQTT_PTR pMQTT);

FTM_RET FTM_MQTT_publish(FTM_MQTT_PTR pMQTT, char *pTopic, void *pPayload, int nPayload, int nQoS);
#endif

