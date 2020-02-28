#ifndef __SIP_EVENT_H
#define __SIP_EVENT_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "sip_common.h"

typedef struct _SIP_EVENT_OBJECT * SIP_EVENT_HANDLE;

SIP_EVENT_HANDLE sip_event_create(void *user_info);
void sip_event_destroy(SIP_EVENT_HANDLE handle);
int sip_event_start(SIP_EVENT_HANDLE handle, SIP_CONFIG *pConfig);
int sip_event_set_cb(SIP_EVENT_HANDLE handle, MEDIA_CMD_CB ui_cb, MEDIA_CMD_CB ses_cb);

int sip_event_invite_user(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession);
int sip_event_session_terminate(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession);
int sip_event_reply_180_ring(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession);
int sip_event_reply_200_OK(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession);
int sip_event_reply_ack(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession);
int sip_event_open_door_lock(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession);
int sip_event_call_lift(SIP_EVENT_HANDLE handle, int mode, SIP_SESSION_INFO *pSession);

#ifdef __cplusplus
}
#endif

#endif
