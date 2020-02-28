#ifndef __SIP_CONF_H
#define __SIP_CONF_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "sip_common.h"
typedef struct _SIP_CONF_OBJECT * SIP_CONF_HANDLE;

SIP_CONF_HANDLE sip_conf_create(void *user_info);
void sip_conf_destroy(SIP_CONF_HANDLE handle);
int sip_conf_start(SIP_CONF_HANDLE handle, SIP_CONFIG *pConfig);
int sip_conf_get_config(SIP_CONF_HANDLE handle, SIP_CONFIG *pConfig);
#ifdef __cplusplus
}
#endif

#endif
