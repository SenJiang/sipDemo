#ifndef __SIP_USER_H
#define __SIP_USER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "sip_common.h"

typedef struct _SIP_USER_OBJECT * SIP_USER_HANDLE;

SIP_USER_HANDLE sip_user_create(void);
void sip_user_destroy(SIP_USER_HANDLE handle);
int sip_user_start(SIP_USER_HANDLE handle);

int sip_user_register(SIP_USER_HANDLE handle, SIP_EXOSIP_EVENT *event, char *sip_call_id);
int sip_user_unregister(SIP_USER_HANDLE handle, int index);

int sip_user_get_all_info(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int index);
int sip_user_set_all_info(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int index);
int sip_user_get_media(SIP_USER_HANDLE handle, SIP_SESSION_INFO *pSession, int index);
int sip_user_find_user_from_sip_call_id(SIP_USER_HANDLE handle, char *sip_call_id);
int sip_user_find_user_from_call_num(SIP_USER_HANDLE handle, char *call_num);
int sip_user_find_user_from_call_num(SIP_USER_HANDLE handle, char *call_num);
int sip_user_find_user_from_call_num(SIP_USER_HANDLE handle, char *call_num);
int sip_user_find_user_from_calling_timer_id(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int timer_id);
int sip_user_find_user_from_server_timer_id(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int timer_id);
int sip_user_save_call_num(SIP_USER_HANDLE handle, int index, char *user_call_num, char *src_call_num);
int sip_user_save_addr(SIP_USER_HANDLE handle, int index, char *ip, int port);
int sip_user_generate_sdp_msg(SIP_USER_HANDLE handle, char *send_buf, int len, char *ip,
                                char *device_id, unsigned short vport, unsigned short aport);
int sip_user_update_event(SIP_USER_HANDLE handle, SIP_EXOSIP_EVENT *event, char *sip_call_id);

#ifdef __cplusplus
}
#endif

#endif

