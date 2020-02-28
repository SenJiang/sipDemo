#include <pthread.h>
#include "sip_common.h"
#include "sip_user.h"
#include <stdlib.h>

typedef struct _SIP_USER_OBJECT
{
    SIP_USER_INFO user[MAX_USER_CNT];
    pthread_mutex_t user_mutex;
}SIP_USER_OBJECT;

static int sip_user_parse_device_type(SIP_USER_HANDLE handle, char *call_num);

SIP_USER_HANDLE sip_user_create(void)
{
    SIP_USER_OBJECT * handle = (SIP_USER_OBJECT *)calloc(1, sizeof(SIP_USER_OBJECT));
    if (NULL == handle)
    {
        debug_err("not enough memory \n");
        return NULL;
    }

    pthread_mutex_init(&handle->user_mutex, NULL);

    return handle;
//create_failed:
    //sip_user_destroy(handle);
}
void sip_user_destroy(SIP_USER_HANDLE handle)
{
    if (NULL != handle)
    {
        pthread_mutex_destroy(&handle->user_mutex);
        free(handle);
        handle = NULL;
    }
}

int sip_user_register(SIP_USER_HANDLE handle, SIP_EXOSIP_EVENT *event, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    
    int i = 0, index = -1;
    pthread_mutex_lock(&handle->user_mutex);
    for (i = 0; i < MAX_USER_CNT; i++)
    {
        if (0 == handle->user[i].is_use)
        {
            handle->user[i].is_use = 1;
            index = i;
            break;
        }
    }
    if (index <= -1)
    {
        debug_info("exceed max user!!\n");
        pthread_mutex_unlock(&handle->user_mutex);
        return -1;
    }
 
    snprintf(handle->user[index].session.sip_call_id,
                sizeof(handle->user[index].session.sip_call_id),"%s", sip_call_id);
    if (NULL != event)
    {
        handle->user[index].session.tid = event->tid;
        handle->user[index].session.did = event->did;
        handle->user[index].session.rid = event->rid;
        handle->user[index].session.cid = event->cid;
        handle->user[index].session.sid = event->sid;
        handle->user[index].session.nid = event->nid;
    }
    pthread_mutex_unlock(&handle->user_mutex);
    debug_info("add new sip user[%d] success!!\n", index);

    return index;
}

int sip_user_unregister(SIP_USER_HANDLE handle, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    pthread_mutex_lock(&handle->user_mutex);
    debug_info("unregister sip user,index:%d call_num:%s\n", index, handle->user[index].call_num);
    memset(&handle->user[index],0x00,sizeof(handle->user[index]));
    pthread_mutex_unlock(&handle->user_mutex);
    
    return 0;
}


int sip_user_get_all_info(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int index)
{
    if ((NULL == handle) || (NULL == puser))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    pthread_mutex_lock(&handle->user_mutex);
    memcpy(puser, &handle->user[index], sizeof(SIP_USER_INFO));
    pthread_mutex_unlock(&handle->user_mutex);

    return 0;
}

int sip_user_set_all_info(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int index)
{
    if ((NULL == handle) || (NULL == puser))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    pthread_mutex_lock(&handle->user_mutex);
    memcpy(&handle->user[index], puser, sizeof(SIP_USER_INFO));
    pthread_mutex_unlock(&handle->user_mutex);

    return 0;
}

int sip_user_get_media(SIP_USER_HANDLE handle, SIP_SESSION_INFO *pSession, int index)
{
    if ((NULL == handle) || (NULL == pSession))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }

    pthread_mutex_lock(&handle->user_mutex);
    memcpy(pSession->call_num, handle->user[index].call_num, sizeof(pSession->call_num));
    memcpy(pSession->dev_ip, handle->user[index].sdp.media_addr, sizeof(pSession->dev_ip));
    pSession->dev_vport = (unsigned short)atoi(handle->user[index].sdp.video_port);
    pSession->dev_aport = (unsigned short)atoi(handle->user[index].sdp.audio_port);
    debug_info("index:%d \n", index);
    pthread_mutex_unlock(&handle->user_mutex);
    
    return 0;
}

int sip_user_find_user_from_sip_call_id(SIP_USER_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    
    int i = 0, index = -1;
    pthread_mutex_lock(&handle->user_mutex);
    for (i = 0; i < MAX_USER_CNT; i++)
    {
        if (0 == strncmp(handle->user[i].session.sip_call_id, sip_call_id, sizeof(handle->user[i].session.sip_call_id)))
        {
            index = i;
            break;
        }
    }
    if (index <= -1)
    {
        debug_err("not found user!!\n");
        pthread_mutex_unlock(&handle->user_mutex);
        return -1;
    }
    pthread_mutex_unlock(&handle->user_mutex);

    return index;
}

int sip_user_find_user_from_call_num(SIP_USER_HANDLE handle, char *call_num)
{
    if ((NULL == handle) || (NULL == call_num))
    {
        debug_err("invalid param\n");
        return -1;
    }
    
    int i = 0, index = -1;
    pthread_mutex_lock(&handle->user_mutex);
    for (i = 0; i < MAX_USER_CNT; i++)
    {
        if (0 == strncmp(handle->user[i].call_num, call_num, sizeof(handle->user[i].call_num)))
        {
            index = i;
            break;
        }
    }
    if (index <= -1)
    {
        debug_err("not found user!!\n");
        pthread_mutex_unlock(&handle->user_mutex);
        return -1;
    }
    pthread_mutex_unlock(&handle->user_mutex);

    return index;
}

int sip_user_find_user_from_server_timer_id(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int timer_id)
{
    if ((NULL == handle) || (NULL == puser))
    {
        debug_err("invalid param\n");
        return -1;
    }
    
    int i = 0, index = -1;
    pthread_mutex_lock(&handle->user_mutex);
    for (i = 0; i < MAX_USER_CNT; i++)
    {
        if (handle->user[i].timer_id_server_req == timer_id)
        {
            index = i;
            break;
        }
    }
    if (index <= -1)
    {
        debug_err("not found user!!\n");
        pthread_mutex_unlock(&handle->user_mutex);
        return -1;
    }
    memcpy(puser, &handle->user[index], sizeof(SIP_USER_INFO));
    pthread_mutex_unlock(&handle->user_mutex);

    return index;
}

int sip_user_find_user_from_calling_timer_id(SIP_USER_HANDLE handle, SIP_USER_INFO *puser, int timer_id)
{
    if ((NULL == handle) || (NULL == puser))
    {
        debug_err("invalid param\n");
        return -1;
    }
    
    int i = 0, index = -1;
    pthread_mutex_lock(&handle->user_mutex);
    for (i = 0; i < MAX_USER_CNT; i++)
    {
        if (handle->user[i].timer_id_calling == timer_id)
        {
            index = i;
            break;
        }
    }
    if (index <= -1)
    {
        debug_err("not found user!!\n");
        pthread_mutex_unlock(&handle->user_mutex);
        return -1;
    }
    memcpy(puser, &handle->user[index], sizeof(SIP_USER_INFO));
    pthread_mutex_unlock(&handle->user_mutex);

    return index;
}

int sip_user_save_call_num(SIP_USER_HANDLE handle, int index, char *user_call_num, char *src_call_num)
{
    if ((NULL == handle)  || (NULL == user_call_num)  || (NULL == src_call_num))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    pthread_mutex_lock(&handle->user_mutex);
    snprintf(handle->user[index].call_num, sizeof(handle->user[index].call_num),
                             "%s", user_call_num);
    snprintf(handle->user[index].to_call_num, sizeof(handle->user[index].to_call_num),
                             "%s", user_call_num);
    snprintf(handle->user[index].from_call_num, sizeof(handle->user[index].from_call_num),
                             "%s", src_call_num);
    debug_info("sip user call num:%s from_call_num:%s to_call_num:%s\n",
                    handle->user[index].call_num,
                    handle->user[index].from_call_num,
                    handle->user[index].to_call_num);
    pthread_mutex_unlock(&handle->user_mutex);


    return 0;
}

int sip_user_save_addr(SIP_USER_HANDLE handle, int index, char *ip, int port)
{
    if ((NULL == handle) || (NULL == ip))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    pthread_mutex_lock(&handle->user_mutex);
    snprintf(handle->user[index].ip, sizeof(handle->user[index].ip),"%s", ip);
    handle->user[index].port = port;
    pthread_mutex_unlock(&handle->user_mutex);
    //debug_info("sip user[%d] addr:%s:%d\n", index, handle->user[index].ip, handle->user[index].port);
    
    return 0;
}

int sip_user_generate_sdp_msg(SIP_USER_HANDLE handle, char *send_buf, int len, char *ip, char *device_id, unsigned short vport, unsigned short aport)
{
    if ((NULL == handle) || (NULL == send_buf) || (NULL == ip)  || (NULL == device_id))
    {
        debug_err("invalid param\n");
        return -1;
    }

    int ret = 0;
    /*
    char local_ip[20] = {'\0'};
    ret = config_msg_get_wan_ip(handle->config_msg_handle, local_ip, sizeof(local_ip));
    if(0 != ret)
    {
        snprintf(local_ip, sizeof(local_ip), "%s", DEFAULT_WAN_IP);
    }
    */
// 符合SDP格式，其中属性a是自定义格式，也就是说可以存放自己的信息，
// 但是只能有两列，比如帐户信息
// 但是经过测试，格式vot必不可少，原因未知，估计是协议栈在传输时需要检查的
    snprintf(send_buf,len,
             "v=0\r\n"
             "o=%s 400 3347 IN IP4 %s\r\n"
             "s=Talk\r\n"
             "c=IN IP4 %s\r\n"
             "b=AS:3072\r\n"
             "t=0 0\r\n"
             "m=audio %d RTP/AVP %d\r\n"
             "a=rtpmap:%d %s/8000\r\n"
             "a=fmtp:%d 0-15\r\n"
             "m=video %d RTP/AVP %d\r\n"
             "a=rtpmap:%d H264/90000\r\n"
             "a=fmtp:%d\r\n",
             device_id,
             ip,
             ip,
             aport,
             VI_G711_PAYLOAD,
             VI_G711_PAYLOAD,
             VI_G711_PAYLOAD_STRING,
             VI_G711_PAYLOAD,
             vport,
             VI_H264_PAYLOAD,
             VI_H264_PAYLOAD,
             VI_H264_PAYLOAD);

    debug_info("send_buf:%s\n", send_buf);
    return 0;
}

int sip_user_update_event(SIP_USER_HANDLE handle, SIP_EXOSIP_EVENT *event, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int ret = -1;
    int index = -1;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    
    index = sip_user_find_user_from_sip_call_id(handle, sip_call_id);
    if (-1 == index)
    {
        debug_err("not found sip_call_id sip user\n");
        return -1;
    }
    if (NULL != event)
    {
        ret = sip_user_get_all_info(handle, &dev_user, index);
        if (0 == ret)
        {
            dev_user.session.tid = event->tid;
            dev_user.session.did = event->did;
            dev_user.session.rid = event->rid;
            dev_user.session.cid = event->cid;
            dev_user.session.sid = event->sid;
            dev_user.session.nid = event->nid;
            sip_user_set_all_info(handle, &dev_user, index);
            debug_info("update event sucess,index:%d\n", index);
        }
    }
    pthread_mutex_unlock(&handle->user_mutex);

    return index;
}

