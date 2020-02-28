#ifndef __SIP_H
#define __SIP_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "sip_common.h"

typedef struct _SIP_SESSION_OBJECT * SIP_SESSION_HANDLE;

SIP_SESSION_HANDLE sip_session_create(void *user_info);
void sip_session_destroy(SIP_SESSION_HANDLE handle);
int sip_session_start(SIP_SESSION_HANDLE handle, SIP_CONFIG *pConfig);
int sip_session_set_cb(SIP_SESSION_HANDLE handle, MEDIA_CMD_CB cmd_cb, MEDIA_DATA_CB data_cb);
int sip_session_stop(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user);

int sip_session_call(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user);
int sip_session_answer(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user);
int sip_session_handle_up(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user);

#ifdef __cplusplus
}
#endif

#endif

