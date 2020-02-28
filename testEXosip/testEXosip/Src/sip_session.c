#include "sip_event.h"
#include "sip_session.h"
#include <stdlib.h>

typedef struct _SIP_SESSION_OBJECT
{
    SIP_EVENT_HANDLE sip_event_handle;
    MEDIA_DATA_CB data_cb;
    void *user_info;
} SIP_SESSION_OBJECT;

static int media_session_cmd_cb(void * arg, int cmd, void *param);

SIP_SESSION_HANDLE sip_session_create(void *user_info)
{
    calloc(1, 2);
    SIP_SESSION_OBJECT * handle = (SIP_SESSION_OBJECT *)calloc(1, sizeof(SIP_SESSION_OBJECT));
    if (NULL == handle)
    {
        debug_err("not enough memory \n");
        return NULL;
    }
    
    handle->sip_event_handle = sip_event_create((void *)handle);
    if (NULL == handle->sip_event_handle)
    {
        debug_err("create sip_event_handle failed!!\n");
        goto create_failed;
    }
    if (NULL != user_info)
    {
        handle->user_info = user_info;
    }
    return handle;
create_failed:
    sip_session_destroy(handle);
    
    return NULL;
}

void sip_session_destroy(SIP_SESSION_HANDLE handle)
{
    if (NULL != handle)
    {
        if (NULL != handle->sip_event_handle)
        {
            sip_event_destroy(handle->sip_event_handle);
        }
        free(handle);
    }
}

int sip_session_start(SIP_SESSION_HANDLE handle, SIP_CONFIG *pConfig)
{
    if ((NULL == handle) || (NULL == pConfig))
    {
        debug_err("invalid param");
        return -1;
    }
    sip_event_start(handle->sip_event_handle, pConfig);
    
    return 0;
}
int sip_session_set_cb(SIP_SESSION_HANDLE handle, MEDIA_CMD_CB cmd_cb, MEDIA_DATA_CB data_cb)
{
    if ((NULL == handle) || (NULL == data_cb))
    {
        debug_err("invalid param");
        return -1;
    }
    handle->data_cb = data_cb;
    sip_event_set_cb(handle->sip_event_handle, cmd_cb, media_session_cmd_cb);
    
    return 0;
}


int sip_session_stop(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user)
{
    if ((NULL == handle) || (NULL == pEvent_user))
    {
        debug_err("invalid param");
        return -1;
    }
    
    return 0;
}

int sip_session_call(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user)
{
    if ((NULL == handle) || (NULL == pEvent_user))
    {
        debug_err("invalid param");
        return -1;
    }
    SIP_SESSION_INFO sip_session;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memcpy(sip_session.call_num, pEvent_user->call_num, sizeof(sip_session.call_num));
    sip_event_invite_user(handle->sip_event_handle, &sip_session);

    return 0;
}

int sip_session_answer(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user)
{
    if ((NULL == handle) || (NULL == pEvent_user))
    {
        debug_err("invalid param");
        return -1;
    }
    SIP_SESSION_INFO sip_session;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memcpy(sip_session.call_num, pEvent_user->call_num, sizeof(sip_session.call_num));
    sip_event_reply_200_OK(handle->sip_event_handle, &sip_session);
    
    return 0;
}

int sip_session_handle_up(SIP_SESSION_HANDLE handle, SIP_USER_EVENT_INFO *pEvent_user)
{
    if ((NULL == handle) || (NULL == pEvent_user))
    {
        debug_err("invalid param");
        return -1;
    }
    SIP_SESSION_INFO sip_session;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memcpy(sip_session.call_num, pEvent_user->call_num, sizeof(sip_session.call_num));
    sip_event_session_terminate(handle->sip_event_handle, &sip_session);

    return 0;
}

static int media_session_cmd_cb(void * arg, int cmd, void *param)
{
    if ((NULL == arg) || (NULL == param))
    {
        debug_err("invalid param");
        return -1;
    }
    SIP_SESSION_HANDLE handle = (SIP_SESSION_OBJECT *)arg;
    SIP_SESSION_INFO *pSession = (SIP_SESSION_INFO *)param;
    SIP_SESSION_INFO sip_session;
    memset(&sip_session, 0x00, sizeof(sip_session));
    debug_info("cmd:%d call_num:%s dev_ip:%s dev_vport:%d dev_aport:%d\n",
                cmd,
                pSession->call_num,
                pSession->dev_ip,
                pSession->dev_vport,
                pSession->dev_aport);
    switch (cmd)
    {
        case SIP_EVENT_INVITE:
        {
            memcpy(&sip_session, pSession, sizeof(sip_session));
            sip_event_reply_180_ring(handle->sip_event_handle, pSession);
        };
        case SIP_EVENT_ANSWER:
        {
            memcpy(&sip_session, pSession, sizeof(sip_session));
            sip_event_reply_ack(handle->sip_event_handle, pSession);
            break;
        }

    }

    return 0;
}


