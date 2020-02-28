#include <netdb.h>
#include <sys/socket.h>
#include "eXosip.h"
#include "sip_common.h"
#include "sip_user.h"
#include "sip_conf.h"
#include "sip_timer.h"
#include "sip_thread.h"
#include "sip_event.h"
#include <stdlib.h>

#define LOCAL_PORT  (7900)
#define UA_STRING "TinyCall V1.0"

/*远程呼叫响铃超时时间*/
#define REMOTE_RINGING_TIME_OUT  45000
/*远程呼叫接听超时时间*/
#define REMOTE_CALLING_TIME_OUT  90000
/*本地呼叫响铃超时时间*/
#define LOCAL_RINGING_TIME_OUT   35000
/*本地呼叫通话超时时间*/
#define LOCAL_CALLING_TIME_OUT   90000
/*心跳包超时时间*/
#define KEEP_ALIVE_TIME_OUT      5000 //5s
/*呼叫门口机本地接受到100 try超时时间*/
#define INVITE_DOORLOCK_RECV_100_TRYING_TIME_OUT 2000

typedef struct _REGPARAM
{
    int regid;
    int expiry;
    int auth;
} REGPARAM;

typedef struct _SIP_EVENT_KEEP_ALIVE_INFO
{
    int timeout_num;
}SIP_EVENT_KEEP_ALIVE_INFO;

typedef struct _SIP_EVENT_OBJECT
{
    SIP_USER_HANDLE user_handle;
    SIP_TIMER_HANDLE timer_handle;
    SIP_CONF_HANDLE config_handle;
    SIP_THREAD_HANDLE  thread_handle;
    
    SIP_EXOSIP_EVENT *event;//save temp session id.
    struct eXosip_t *context_exosip;
    SIP_EVENT_KEEP_ALIVE_INFO keep_alive;
    char send_buf[4096];
    void *user_info;
    MEDIA_CMD_CB ui_cb;         /*返回给UI层*/
    MEDIA_CMD_CB ses_cb;        /*返回给session层*/
}SIP_EVENT_OBJECT;

static int sip_event_register(SIP_EVENT_HANDLE handle);
static void sip_event_center(void *user_info);

static int sip_event_send_message(SIP_EVENT_HANDLE handle, char * msg_buf, char *sip_call_id, char *call_num);
static int sip_event_handle_invite(SIP_EVENT_HANDLE handle, eXosip_event_t *event, char *sip_call_id);
static int sip_event_handle_ring(SIP_EVENT_HANDLE handle, eXosip_event_t *event, char *sip_call_id);
static int sip_event_handle_answered(SIP_EVENT_HANDLE handle, char *sip_call_id);
static int sip_event_handle_ack(SIP_EVENT_HANDLE handle, char *sip_call_id);
static int sip_event_handle_closed(SIP_EVENT_HANDLE handle, char *sip_call_id);
static int sip_event_handle_reject(SIP_EVENT_HANDLE handle, char *sip_call_id);
static int sip_event_handle_message_answered(SIP_EVENT_HANDLE handle, char *sip_call_id, char *keep_alive_call_id);
static int sip_event_user_close(SIP_EVENT_HANDLE handle, int index);
static int sip_event_parse_sdp(SIP_EVENT_HANDLE handle, int index);
static int sip_event_parse_sdp_audio_rate(char *audio_rate, int audio_rate_len, void *sdp);
static int sip_event_parse_addr(SIP_EVENT_HANDLE handle, int index, void *request);
static int sip_event_parse_call_num(SIP_EVENT_HANDLE handle, int index, void *request);
static int sip_event_reply_486_busy(SIP_EVENT_HANDLE handle, int index);
static int sip_event_report_msg_to_server(SIP_EVENT_HANDLE handle, int index);
static int sip_event_set_timer_calling_timeout(SIP_EVENT_HANDLE handle, int time, int index);
static int sip_event_set_timer_request_server_timeout(SIP_EVENT_HANDLE handle, int index);
static int sip_event_time_out_disconnect(int time_id, void *arg);
static int sip_event_kill_all_timer(SIP_EVENT_HANDLE handle, int index);
static int sip_event_recv_100_trying_time_out(int time_id, void *arg);
static int sip_event_recv_100_trying(SIP_EVENT_HANDLE handle, char *sip_call_id);
static int sip_event_check_answer(SIP_EVENT_HANDLE handle, char *sip_call_id);
#ifdef SIP_KEEP_ALIVE
static int sip_event_keep_alive_time_out(int time_id, void *arg);
static int sip_event_send_keep_alive_msg(SIP_EVENT_HANDLE handle, char *sip_call_id);
#endif
SIP_EVENT_HANDLE sip_event_create(void *user_info)
{
    SIP_EVENT_OBJECT * handle = (SIP_EVENT_OBJECT *)calloc(1, sizeof(SIP_EVENT_OBJECT));
    if (NULL == handle)
    {
        debug_err("not enough memory \n");
        return NULL;
    }
    handle->user_handle = sip_user_create();
    if (NULL ==  handle->user_handle)
    {
        debug_err("sip_user_create failed \n");
        goto create_failed;
    }
    handle->config_handle = sip_conf_create((void *)handle);
    if (NULL ==  handle->config_handle)
    {
        debug_err("sip_config_create failed \n");
        goto create_failed;
    }
    handle->timer_handle = sip_timer_create();
    if (NULL == handle->timer_handle)
    {
        debug_err("sip_timer_create failed \n");
        goto create_failed;
    }
    handle->context_exosip = eXosip_malloc();
    if (NULL == handle->context_exosip)
    {
        debug_err("eXosip_malloc failed \n");
        goto create_failed;
    }
    handle->event = (SIP_EXOSIP_EVENT *)calloc(1, sizeof(SIP_EXOSIP_EVENT));
    if (NULL == handle->event)
    {
        debug_err("eXosip_malloc failed \n");
        goto create_failed;
    }
    if (NULL != user_info)
    {
        handle->user_info = user_info;
    }

    return handle;

create_failed:
    sip_event_destroy(handle);

    return NULL;
}

void sip_event_destroy(SIP_EVENT_HANDLE handle)
{
    if (NULL != handle)
    {
        if (NULL != handle->user_handle)
        {
            sip_user_destroy(handle->user_handle);
            handle->user_handle = NULL;
        }
        if (NULL != handle->config_handle)
        {
            sip_conf_destroy(handle->config_handle);
            handle->config_handle = NULL;
        }
        if (NULL != handle->timer_handle)
        {
            sip_timer_destroy(handle->timer_handle);
            handle->timer_handle = NULL;
        }
        if (NULL != handle->context_exosip)
        {
            eXosip_quit(handle->context_exosip);
            osip_free(handle->context_exosip);
        }
        if (NULL != handle->event)
        {
            free(handle->event);
            handle->event = NULL;
        }
        free(handle);
        handle = NULL;
    }
}

int sip_event_start(SIP_EVENT_HANDLE handle, SIP_CONFIG *pConfig)
{
    if (NULL == handle)
    {
        debug_err("invalid param \n");
        return -1;
    }
    int ret = -1;
    SIP_CONFIG sip_config;
    memset(&sip_config, 0x00, sizeof(sip_config));
    sip_conf_start(handle->config_handle, pConfig);
    sip_conf_get_config(handle->config_handle, &sip_config);
    if (0 != eXosip_init(handle->context_exosip))
    {
        debug_err("Couldn't initialize eXosip!\n");
        return - 1;
    }
    debug_info("wan_ip:%s username:%s\n", sip_config.wan_ip, sip_config.username);
    if (sip_config.transport == 0)
    {
        ret = eXosip_listen_addr(handle->context_exosip, IPPROTO_UDP, NULL, LOCAL_PORT, AF_INET, 0);
    }
    else if (sip_config.transport == 1)
    {
        ret = eXosip_listen_addr(handle->context_exosip, IPPROTO_TCP, NULL, LOCAL_PORT, AF_INET, 0);
    }
    else if (sip_config.transport == 2)
    {
        ret = eXosip_listen_addr(handle->context_exosip, IPPROTO_TCP, NULL, LOCAL_PORT, AF_INET, 1);
    }
    else if (sip_config.transport == 3)
    {
        ret = eXosip_listen_addr(handle->context_exosip, IPPROTO_UDP, NULL, LOCAL_PORT, AF_INET, 1);
    }
    else
    {
        debug_err("invalid transport");
    }
    if (0 != ret)
    {
        debug_err("eXosip_listen_addr failed");
        return - 1;
    }
    handle->thread_handle = sip_thread_create(sip_event_center, (void *)handle);
    if (NULL == handle->thread_handle)
    {
        debug_err("sip_thread_create failed \n");
        return -1;
    }
    /*向网关注册用户信息*/
    sip_event_register(handle);

    return 0;
}

int sip_event_set_cb(SIP_EVENT_HANDLE handle, MEDIA_CMD_CB ui_cb, MEDIA_CMD_CB ses_cb)
{
    if ((NULL == handle) || (NULL == ui_cb) || (NULL == ses_cb))
    {
        debug_err("invalid param \n");
        return -1;
    }
    handle->ui_cb = ui_cb;
    handle->ses_cb = ses_cb;
    
    return 0;
}

static int sip_event_register(SIP_EVENT_HANDLE handle)
{
    if (NULL == handle)
    {
        debug_err("invalid param \n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    debug_info("sip_event_register !!\n");
    int ret = -1;
    osip_message_t *reg = NULL;
    char from[128] = {'\0'};
    char proxy[128] = {'\0'};
    REGPARAM regparam = {0, 3600, 0};
    SIP_CONFIG sip_config;
    memset(&sip_config, 0x00, sizeof(sip_config));
    sip_conf_get_config(handle->config_handle, &sip_config);
    eXosip_set_user_agent(handle->context_exosip, UA_STRING);
    if ((strlen(sip_config.username) > 0) &&(strlen(sip_config.password) > 0))
    {
        debug_info("username: %s password: %s \n", sip_config.username, sip_config.password);
        if (eXosip_add_authentication_info(handle->context_exosip, sip_config.username,
                                           sip_config.username, sip_config.password, NULL, NULL))
        {
            debug_err("eXosip_add_authentication_info failed \n");
            return -1;
        }
    }
    else
    {
        debug_err("username or password is null\n");
        return -1;
    }
    snprintf(from, sizeof(from), "\"%s\"<sip:%s@%s:%d>",
                    sip_config.src_call_num,
                    sip_config.username,
                    sip_config.src_ip,
                    sip_config.src_port);
    snprintf(proxy, sizeof(proxy), "sip:%s:%d", sip_config.svr_ip, sip_config.svr_port);
    debug_info("from:%s proxy:%s\n", from, proxy);
    regparam.regid = eXosip_register_build_initial_register(handle->context_exosip,
                                                            from,proxy, NULL,
                                                            regparam.expiry * 2, &reg);
    if ((regparam.regid < 1) || (NULL == reg))
    {
        debug_err("eXosip_register_build_initial_register failed \n");
        return -1;
    }
    ret = eXosip_register_send_register(handle->context_exosip, regparam.regid, reg);
    if (0 != ret)
    {
        debug_err("eXosip_register_send_register failed \n");
        return -1;
    }
    
    return 0;
}


static void sip_event_center(void *user_info)
{
    SIP_EVENT_OBJECT * handle = (SIP_EVENT_OBJECT *)user_info;
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return;
    }
    int counter = 0;
    eXosip_event_t *event;
    struct eXosip_stats stats;
    osip_call_id_t *call_id = NULL;
    char keep_alive_call_id[30] = {'\0'};

    while (1)
    {
        if (NULL == handle->context_exosip)
        {
            debug_err("please eXosip_init first.\n");
            break;
        }
        counter++;
        if (counter % 6000 == 0)/*十分钟打印一次*/
        {
            memset(&stats, 0, sizeof (struct eXosip_stats));
            eXosip_lock(handle->context_exosip);
            eXosip_set_option(handle->context_exosip, EXOSIP_OPT_GET_STATISTICS, &stats);
            eXosip_unlock(handle->context_exosip);
            debug_info("eXosip stats: inmemory=(tr:%i//reg:%i) average=(tr:%f//reg:%f)",
                            stats.allocated_transactions,
                            stats.allocated_registrations,
                            stats.average_transactions,
                            stats.average_registrations);
        }
#ifdef SIP_KEEP_ALIVE
        if(counter % 600 == 0)/*心跳包每600==60s发一次到服务器*/
        {
            sip_event_send_keep_alive_msg(handle, keep_alive_call_id);
            sip_timer_set(handle->timer_handle, SIP_TIMER_KEEP_ALIVE_TIME_OUT, KEEP_ALIVE_TIME_OUT,
                            sip_event_keep_alive_time_out, (void *)handle);
        }
#endif
        if (!(event = eXosip_event_wait (handle->context_exosip, 0, 100)))
        {
#ifdef OSIP_MONOTHREAD
            eXosip_execute (handle->context_exosip);
#endif
            eXosip_automatic_action(handle->context_exosip);
            osip_usleep (10000);
            continue;
        }
        //save tmp event id.
        handle->event->cid = event->cid;
        handle->event->tid = event->tid;
        handle->event->did = event->did;
        debug_info("[sip_event_center]type:%d cid:%d did:%d tid:%d rid:%d sid:%d nid:%d\n",
            event->type, event->cid,event->did,event->tid,event->rid,event->sid,event->nid);
#ifdef OSIP_MONOTHREAD
        eXosip_execute(handle->context_exosip);
#endif
        eXosip_lock(handle->context_exosip);
        eXosip_automatic_action(handle->context_exosip);
        if (NULL == event->request)
        {
            eXosip_unlock (handle->context_exosip);
            eXosip_event_free (event);
            osip_usleep (10000);
            continue;
        }
        switch (event->type)
        {
            case EXOSIP_REGISTRATION_SUCCESS:
            {
                debug_info("registrered successfully !!\n");
                break;
            }
            case EXOSIP_REGISTRATION_FAILURE:
            {
                debug_info("registrered failed !!\n");
                break;
            }
            case EXOSIP_CALL_INVITE:
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv sip user invite,call_id:%s\n", call_id->number);
                sip_event_handle_invite(handle, event, call_id->number);
                break;
            }
            case EXOSIP_CALL_PROCEEDING: //收到100 trying消息，表示请求正在处理中
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv 100 trying,call_id:%s\n", call_id->number);
                sip_event_recv_100_trying(handle, call_id->number);
                break;
            }
            case EXOSIP_CALL_RINGING:   /*收到180 Ringing应答，表示接收到INVITE请求的UAS正在向被叫用户振铃*/
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv 180 ringing,call_id:%s\n", call_id->number);
                sip_event_handle_ring(handle, event, call_id->number);
                break;
            }
            case EXOSIP_CALL_ANSWERED:
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv sip user answer,call_id:%s\n", call_id->number);
                sip_event_handle_answered(handle, call_id->number);
                break;
            }
            case EXOSIP_CALL_ACK: //ACK received for 200ok to INVITE 
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv a ack,call_id:%s\n", call_id->number);
                sip_event_handle_ack(handle, call_id->number);
                break;
            }
            case EXOSIP_CALL_CLOSED: //a BYE was received for this call
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv a bye,call_id:%s\n", call_id->number);
                sip_event_handle_closed(handle, call_id->number);
                break;
            }
            case EXOSIP_CALL_RELEASED:
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("release ,call_id:%s\n", call_id->number);
                //sip_event_handle_closed(handle, call_id->number);
                break;
            } 
            case EXOSIP_MESSAGE_ANSWERED:
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("recv message answer,call_id:%s\n", call_id->number);
                sip_event_handle_message_answered(handle, call_id->number, keep_alive_call_id);
                break;
            }
            case EXOSIP_CALL_GLOBALFAILURE:/*该返回值为呼叫对方，对方拒绝接听，返回603*/
            {
                call_id = osip_message_get_call_id(event->request);
                debug_info("EXOSIP_CALL_GLOBALFAILURE, call_id:%s\n", call_id->number);
                sip_event_handle_reject(handle, call_id->number);
                break;
            }
            case EXOSIP_MESSAGE_NEW:
            {
                //debug_info("recv new message !!\n");
                int i = 0;;
                osip_message_t *answer = NULL;
                i = eXosip_message_build_answer (handle->context_exosip, event->tid, SIP_OK, &answer);
                if (i != 0)
                {
                    debug_err("failed to reject %s\n", event->request->sip_method);
                    break;
                }
                i = eXosip_message_send_answer (handle->context_exosip, event->tid, SIP_OK, answer);
                if (i != 0)
                {
                    debug_err("failed to reject %s\n", event->request->sip_method);
                    break;
                }
                break;
            }
            case EXOSIP_IN_SUBSCRIPTION_NEW:
            {
                debug_info("recv new subscription !!\n");
                osip_message_t *answer = NULL;
                int i;

                i = eXosip_insubscription_build_answer (handle->context_exosip, event->tid, 405, &answer);
                if (i != 0)
                {
                    debug_err("failed to reject %s\n", event->request->sip_method);
                    break;
                }
                i = eXosip_insubscription_send_answer (handle->context_exosip, event->tid, 405, answer);
                if (i != 0)
                {
                    debug_err("failed to reject %s\n", event->request->sip_method);
                    break;
                }
                break;
            } 
            default:
                //debug_info("recieved unknown eXosip event (type, did, cid) = (%d, %d, %d)\n", event->type, event->did, event->cid);
                break;
        }
        eXosip_unlock (handle->context_exosip);
        eXosip_event_free (event);
    }
    debug_err("sip_event_center exit\n");
}

static int sip_event_handle_invite(SIP_EVENT_HANDLE handle, eXosip_event_t *event, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == event) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int index = -1;
    int ret = -1;
    SIP_SESSION_INFO sip_session;
    SIP_USER_EVENT_INFO sip_user_event;
    SIP_EXOSIP_EVENT sip_event;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&sip_event, 0x00, sizeof(sip_event));
    memset(&sip_user_event, 0x00, sizeof(sip_user_event));
    sip_event.tid = event->tid;
    sip_event.did = event->did;
    sip_event.rid = event->rid;
    sip_event.cid = event->cid;
    sip_event.sid = event->sid;
    sip_event.nid = event->nid;
    index = sip_user_register(handle->user_handle, &sip_event, sip_call_id);
    if ((index > -1)  && (index < MAX_USER_CNT))
    {
        sip_event_set_timer_calling_timeout(handle, REMOTE_RINGING_TIME_OUT, index);
        sip_event_parse_addr(handle, index, (void *)event->request);
        sip_event_parse_call_num(handle, index, (void *)event->request);
        sip_event_parse_sdp(handle, index);
        //add something
        sip_user_get_media(handle->user_handle, &sip_session, index);
        if (NULL != handle->ses_cb)
        {
            handle->ses_cb(handle->user_info, SIP_EVENT_INVITE, &sip_session);
        }
        if (NULL != handle->ui_cb)
        {
            memcpy(sip_user_event.call_num, sip_session.call_num, sizeof(sip_user_event.call_num));
            handle->ui_cb(NULL, SIP_EVENT_INVITE, &sip_user_event);
        }
    }
    else
    {
        debug_info("terminate the session !!\n");
        sip_event_reply_486_busy(handle, index);
    }
    
    return 0;
}

static int sip_event_handle_ring(SIP_EVENT_HANDLE handle, eXosip_event_t *event, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == event) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int index = -1;
    SIP_SESSION_INFO sip_session;
    SIP_USER_EVENT_INFO sip_user_event;
    SIP_EXOSIP_EVENT sip_event;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&sip_event, 0x00, sizeof(sip_event));
    memset(&sip_user_event, 0x00, sizeof(sip_user_event));
    sip_event.tid = event->tid;
    sip_event.did = event->did;
    sip_event.rid = event->rid;
    sip_event.cid = event->cid;
    sip_event.sid = event->sid;
    sip_event.nid = event->nid;
    index = sip_user_update_event(handle->user_handle, &sip_event, sip_call_id);
    if ((index > -1) && (index < MAX_USER_CNT))
    {
        sip_event_parse_sdp(handle, index);
        sip_user_get_media(handle->user_handle, &sip_session, index);
        if (NULL != handle->ses_cb)
        {
            handle->ses_cb(handle->user_info, SIP_EVENT_RING, &sip_session);
        }
        if (NULL != handle->ui_cb)
        {
            memcpy(sip_user_event.call_num, sip_session.call_num, sizeof(sip_user_event.call_num));
            handle->ui_cb(NULL, SIP_EVENT_RING, &sip_user_event);
        }
    }
    else
    {
        debug_info("[EXOSIP_CALL_RINGING]reply busy to user!!!!\n");
        sip_event_reply_486_busy(handle, index);
        sip_event_kill_all_timer(handle, index);
        sip_user_unregister(handle->user_handle, index);
    }

    return 0;
}

static int sip_event_handle_answered(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    int index = -1, ret = -1;
    SIP_SESSION_INFO sip_session;
    SIP_USER_EVENT_INFO sip_user_event;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&sip_user_event, 0x00, sizeof(sip_user_event));
    index = sip_event_check_answer(handle, sip_call_id);
    if (index > -1)
    {
        sip_user_get_media(handle->user_handle, &sip_session, index);
        if (NULL != handle->ses_cb)
        {
            handle->ses_cb(handle->user_info, SIP_EVENT_ANSWER, &sip_session);
        }
        if (NULL != handle->ui_cb)
        {
            memcpy(sip_user_event.call_num, sip_session.call_num, sizeof(sip_user_event.call_num));
            handle->ui_cb(NULL, SIP_EVENT_ANSWER, &sip_user_event);
        }
    }
    /*
    else
    {
        debug_info("[EXOSIP_CALL_ANSWERED]terminate the session !!\n");
        eXosip_lock(handle->context_exosip);
        eXosip_call_terminate(handle->context_exosip, dev_user.session.cid, dev_user.session.did);  
        eXosip_unlock(handle->context_exosip);
    }*/

    return 0;
}

static int sip_event_handle_ack(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    int index = -1;
    SIP_SESSION_INFO sip_session;
    SIP_USER_EVENT_INFO sip_user_event;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&sip_user_event, 0x00, sizeof(sip_user_event));
    index = sip_event_check_answer(handle, sip_call_id);
    if (index > -1)
    {
        sip_user_get_media(handle->user_handle, &sip_session, index);
        if (NULL != handle->ses_cb)
        {
            handle->ses_cb(handle->user_info, SIP_EVENT_ACK, &sip_session);
        }
        if (NULL != handle->ui_cb)
        {
            memcpy(sip_user_event.call_num, sip_session.call_num, sizeof(sip_user_event.call_num));
            handle->ui_cb(NULL, SIP_EVENT_ACK, &sip_user_event);
        }
    }

    return 0;
}

static int sip_event_handle_closed(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int index = -1;
    SIP_SESSION_INFO sip_session;
    SIP_USER_EVENT_INFO sip_user_event;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&sip_user_event, 0x00, sizeof(sip_user_event));
    index = sip_user_find_user_from_sip_call_id(handle->user_handle, sip_call_id);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found sip_call_id user\n");
        return -1;
    }
    sip_user_get_media(handle->user_handle, &sip_session, index);
    if (NULL != handle->ses_cb)
    {
        handle->ses_cb(handle->user_info, SIP_EVENT_BYE, &sip_session);
    }
    if (NULL != handle->ui_cb)
    {
        memcpy(sip_user_event.call_num, sip_session.call_num, sizeof(sip_user_event.call_num));
        handle->ui_cb(NULL, SIP_EVENT_BYE, &sip_user_event);
    }
    sip_event_user_close(handle, index);
    sip_event_kill_all_timer(handle, index);
    sip_user_unregister(handle->user_handle, index);

    return 0;
}

static int sip_event_handle_reject(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int index = -1;
    SIP_SESSION_INFO sip_session;
    SIP_USER_EVENT_INFO sip_user_event;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&sip_user_event, 0x00, sizeof(sip_user_event));

    index = sip_user_find_user_from_sip_call_id(handle->user_handle, sip_call_id);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found sip_call_id user\n");
        return -1;
    }
    sip_user_get_media(handle->user_handle, &sip_session, index);
    debug_info("sip_event_handle_reject,call_num:%s\n", sip_session.call_num);
    if (NULL != handle->ses_cb)
    {
        handle->ses_cb(handle->user_info, SIP_EVENT_BYE, &sip_session);
    }
    if (NULL != handle->ui_cb)
    {
        memcpy(sip_user_event.call_num, sip_session.call_num, sizeof(sip_user_event.call_num));
        handle->ui_cb(NULL, SIP_EVENT_BYE, &sip_user_event);
    }

    sip_event_user_close(handle, index);
    sip_event_kill_all_timer(handle, index);
    sip_user_unregister(handle->user_handle, index);

    return 0;
}

static int sip_event_handle_message_answered(SIP_EVENT_HANDLE handle, char *sip_call_id, char *keep_alive_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id) || (NULL == keep_alive_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
#ifdef SIP_KEEP_ALIVE
    int ret = -1;
    ret = sip_event_clear_keep_alive_flag(handle, sip_call_id, keep_alive_call_id);
    if (0 == ret)
    {
        sip_timer_kill(handle->timer_handle, SIP_TIMER_KEEP_ALIVE_TIME_OUT);
        handle->keep_alive.timeout_num = 0;
    }
#endif
    return 0;
}

int sip_event_invite_user(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession)
{ 
    if ((NULL == handle) || (NULL == pSession) || (0 == strlen(pSession->call_num)))
    {
        debug_err("invalid param \n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    debug_info("sip_event_invite_user:%s\n", pSession->call_num);

    osip_message_t *invite  = NULL;
    osip_call_id_t *call_id = NULL;;
    int index = -1;
    int ret = -1;
    char src_call[64] = {'\0'};
    char dst_call[64] = {'\0'};
    SIP_CONFIG sip_config;
    memset(&sip_config, 0x00, sizeof(sip_config));
    sip_conf_get_config(handle->config_handle, &sip_config);
    snprintf(src_call, sizeof(src_call), "sip:%s@%s:%d", sip_config.src_call_num, sip_config.src_ip, sip_config.src_port);
    snprintf(dst_call, sizeof(dst_call), "sip:%s@%s:%d", pSession->call_num, sip_config.svr_ip, sip_config.svr_port);

    debug_info("dst_call:%s src_call:%s\n",dst_call,src_call);
    ret = eXosip_call_build_initial_invite(handle->context_exosip,
                                         &invite,
                                         dst_call,
                                         src_call, NULL,
                                         "This is a call for conversation");
    if ((0 != ret) || (NULL == invite))
    {
        debug_err("eXosip_call_build_initial_invite failed!\n");
        return -1;
    }
    memset(handle->send_buf, 0x00, sizeof(handle->send_buf));
    sip_user_generate_sdp_msg(handle->user_handle, handle->send_buf, sizeof(handle->send_buf),
                                sip_config.wan_ip, sip_config.username, pSession->local_vport, pSession->local_aport);
    osip_message_set_body(invite, handle->send_buf, strlen(handle->send_buf));
    osip_message_set_content_type(invite, "application/sdp");
    call_id = osip_message_get_call_id(invite);
    if (NULL == call_id)
    {
        debug_err("osip_message_get_call_id failed!!\n");
        return -1;
    }
    index = sip_user_register(handle->user_handle, NULL, call_id->number);
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("sip_event_dev_user_register failed!!\n");
        return -1;
    }
    sip_user_save_call_num(handle->user_handle, index, pSession->call_num, sip_config.src_call_num);
    //p_event_set_timer_request_server_timeout(handle, index);
    //p_event_set_timer_calling_timeout(handle, LOCAL_RINGING_TIME_OUT, index);
    sip_user_save_addr(handle->user_handle, index, sip_config.svr_ip, sip_config.svr_port);

    //eXosip_lock(handle->context_exosip);
    //invite SIP INVITE message to send
    ret = eXosip_call_send_initial_invite(handle->context_exosip, invite);
    //eXosip_unlock(handle->context_exosip);

    
    return ret;
}

int sip_event_session_terminate(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession)
{
    if ((NULL == handle) || (NULL == pSession) || (0 == strlen(pSession->call_num)))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    int index = -1, ret = -1;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    index = sip_user_find_user_from_call_num(handle->user_handle, pSession->call_num);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found sip user\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        //eXosip_lock(handle->context_exosip);
        eXosip_call_terminate(handle->context_exosip, dev_user.session.cid, dev_user.session.did);
        //eXosip_unlock(handle->context_exosip);
        debug_info("close user[%d],call_num:%s\n", index, dev_user.call_num);
        sip_event_kill_all_timer(handle, index);
        sip_user_unregister(handle->user_handle, index);
    }

    return 0;
}

int sip_event_reply_180_ring(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession)
{
    if ((NULL == handle) || (NULL == pSession) || (0 == strlen(pSession->call_num)))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    int index = -1;
    int ret = -1;
    osip_message_t *answer = NULL;;
    SIP_CONFIG sip_config;
    SIP_USER_INFO dev_user;
    memset(&sip_config, 0x00, sizeof(sip_config));
    memset(&dev_user, 0x00, sizeof(dev_user));
    index = sip_user_find_user_from_call_num(handle->user_handle, pSession->call_num);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found call num sip user\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        ret = eXosip_call_build_answer(handle->context_exosip, dev_user.session.tid, SIP_RINGING, &answer);
        if ((0 != ret) || (NULL == answer))
        {
            debug_err("eXosip_call_build_answer,ret:%d\n", ret);
            return -1;
        }
        memset(handle->send_buf, 0x00, sizeof(handle->send_buf));
        sip_conf_get_config(handle->config_handle, &sip_config);
        sip_user_generate_sdp_msg(handle->user_handle, handle->send_buf, sizeof(handle->send_buf),
                                    sip_config.wan_ip, sip_config.username,
                                    pSession->local_vport,
                                    pSession->local_aport);
        osip_message_set_body(answer, handle->send_buf, strlen(handle->send_buf));
        osip_message_set_content_type(answer, "application/sdp");
        eXosip_call_send_answer(handle->context_exosip, dev_user.session.tid, SIP_RINGING, answer);
        debug_info("sip_event_reply_180_ring success,call_num:%s\n", dev_user.call_num);
    }
    return 0;
}

int sip_event_reply_200_OK(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession)
{
    if ((NULL == handle) || (NULL == pSession) || (0 == strlen(pSession->call_num)))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }

    int ret = -1;
    osip_message_t *answer = NULL;;
    int index = -1;
    
    SIP_CONFIG sip_config;
    SIP_USER_INFO dev_user;
    memset(&sip_config, 0x00, sizeof(sip_config));
    memset(&dev_user, 0x00, sizeof(dev_user));
    index = sip_user_find_user_from_call_num(handle->user_handle, pSession->call_num);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found call num sip user\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle,&dev_user, index);
    if (0 == ret)
    {
        /*test:后面需要改成主动挂断*/
        sip_timer_set(handle->timer_handle, dev_user.timer_id_calling, REMOTE_CALLING_TIME_OUT,
                        sip_event_time_out_disconnect, (void *)handle);
        dev_user.answer = 1;
        ret = eXosip_call_build_answer(handle->context_exosip, dev_user.session.tid, SIP_OK, &answer);
        if ((0 != ret) || (NULL == answer))
        {
            debug_err("failed to build answer, ret:%d\n", ret);
            return -1;
        }
        memset(handle->send_buf, 0x00, sizeof(handle->send_buf));
        sip_conf_get_config(handle->config_handle, &sip_config);
        sip_user_generate_sdp_msg(handle->user_handle, handle->send_buf, sizeof(handle->send_buf), sip_config.wan_ip,
                                     sip_config.username, pSession->local_vport, pSession->local_aport);
        osip_message_set_body(answer, handle->send_buf, strlen(handle->send_buf));
        osip_message_set_content_type(answer, "application/sdp"); 
        {
            ret = eXosip_call_send_answer (handle->context_exosip,dev_user.session.tid, SIP_OK, answer);
            if (0 != ret)
            {
                debug_err("failed to send answer,ret:%d\n", ret);
                return -1;
            }
            debug_info("sip_event_reply_200_OK success,call_num:%s\n", dev_user.call_num);
        }
    }
    return 0;
}

int sip_event_reply_ack(SIP_EVENT_HANDLE handle, SIP_SESSION_INFO *pSession)
{
    if ((NULL == handle) || (NULL == pSession) || (0 == strlen(pSession->call_num)))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    osip_message_t *ack = NULL;
    int index = -1;
    int ret = -1;
    SIP_CONFIG sip_config;
    SIP_USER_INFO dev_user;
    memset(&sip_config, 0x00, sizeof(sip_config));
    memset(&dev_user, 0x00, sizeof(dev_user));
    index = sip_user_find_user_from_call_num(handle->user_handle, pSession->call_num);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found call num sip user\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle,&dev_user, index);
    if (0 == ret)
    {
        ret = eXosip_call_build_ack(handle->context_exosip, dev_user.session.did, &ack);
        if ((0 != ret) || (NULL == ack))
        {
            debug_err("failed to build ack,ret:%d\n", ret);
            return -1;
        }
        ret = eXosip_call_send_ack(handle->context_exosip, dev_user.session.did, ack);
        if (0 != ret)
        {
            debug_err("failed to send ack,ret:%d\n", ret);
            return -1;
        }
        debug_info("sip_event_reply_ack success,call_num:%s\n", dev_user.call_num);
    }
    return 0;
}

static int sip_event_reply_486_busy(SIP_EVENT_HANDLE handle, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    int ret = -1;
    osip_message_t *answer = NULL;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle,&dev_user, index);
    if (0 == ret)
    {
        /*test:这里的tid需要测试，每个请求的tid 貌似都不一样*/
        ret = eXosip_call_build_answer (handle->context_exosip, dev_user.session.tid, SIP_BUSY_HERE, &answer);
        if ((0 != ret) || (NULL == answer))
        {
            debug_err("failed to reject,ret:%d\n", ret);
            return -1;
        }
        ret = eXosip_call_send_answer(handle->context_exosip, dev_user.session.tid, SIP_BUSY_HERE, answer);
        if (0 != ret)
        {
            debug_err("failed to send answer,ret:%d\n", ret);
            return -1;
        }
        debug_info("sip_event_reply_486_busy,call_num:%s\n", dev_user.call_num);
    }
    return 0;
}

static int sip_event_user_close(SIP_EVENT_HANDLE handle, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    int ret = 0;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        if (dev_user.start_time)
        {
            //dev_user.end_time = get_real_time_msec();
            sip_user_set_all_info(handle->user_handle, &dev_user, index);
        }
    }

    return 0;
}

static int sip_event_parse_sdp(SIP_EVENT_HANDLE handle, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please eXosip_init first.\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    sdp_message_t *sdp = NULL;
    sdp_connection_t *connection = NULL;
    sdp_media_t * video_sdp = NULL;
    sdp_media_t * audio_sdp = NULL;
    int ret = 0;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        sdp = eXosip_get_remote_sdp (handle->context_exosip, dev_user.session.did);
        if(NULL == sdp)
        {
            debug_err("sdp is null\n");
            return -1;
        }
        if (NULL != sdp->o_username)
        {
            strncpy(dev_user.sdp.user_name,sdp->o_username, sizeof(dev_user.sdp.user_name));
        }
        if (NULL != sdp->o_addr)
        {
            strncpy(dev_user.sdp.device_addr,sdp->o_addr, sizeof(dev_user.sdp.device_addr));
        }
        /*because audio.addr == video.addr,so we use video addr!! */
        connection = eXosip_get_video_connection(sdp);
        if ((NULL != connection) && (NULL != connection->c_addr))
        {
            strncpy(dev_user.sdp.media_addr,connection->c_addr, sizeof(dev_user.sdp.media_addr));
        }
        video_sdp = eXosip_get_video_media(sdp);
        if ((NULL != video_sdp) && (NULL != video_sdp->m_port))
        {
            strncpy(dev_user.sdp.video_port, video_sdp->m_port, sizeof(dev_user.sdp.video_port));
        }
        audio_sdp = eXosip_get_audio_media(sdp);
        if ((NULL != audio_sdp) && (NULL != audio_sdp->m_port))
        {
            strncpy(dev_user.sdp.audio_port, audio_sdp->m_port, sizeof(dev_user.sdp.audio_port));
        }
        snprintf(dev_user.sdp.v_payload,sizeof(dev_user.sdp.v_payload), "%d",VI_H264_PAYLOAD);
        strncpy(dev_user.sdp.a_payload,"0", sizeof(dev_user.sdp.a_payload));
        sip_event_parse_sdp_audio_rate(dev_user.sdp.audio_rate, sizeof(dev_user.sdp.audio_rate), (void *)sdp);
        sip_user_set_all_info(handle->user_handle, &dev_user, index);
        debug_info("[sip user %d sdp]user_name:%s device_addr:%s media_addr:%s video_port:%s v_payload:%s audio_port:%s a_payload:%s audio_rate:%s \n",
                            index,
                            dev_user.sdp.user_name,
                            dev_user.sdp.device_addr,
                            dev_user.sdp.media_addr,
                            dev_user.sdp.video_port,
                            dev_user.sdp.v_payload,
                            dev_user.sdp.audio_port,
                            dev_user.sdp.a_payload,
                        dev_user.sdp.audio_rate);
    }
#if 0   
    debug_info("[video]:c_nettype:%s c_addrtype:%s c_addr:%s c_addr_multicast_ttl:%s c_addr_multicast_int:%s\n",
        connection->c_nettype,
        connection->c_addrtype,
        connection->c_addr,
        connection->c_addr_multicast_ttl,
        connection->c_addr_multicast_int
        );

    debug_info("[sdp]version:%s\n",sdp_message_v_version_get(sdp));    
    debug_info("[sdp]o_username:%s\n",sdp_message_o_username_get(sdp));    
    debug_info("[sdp]o_sess_id:%s\n",sdp_message_o_sess_id_get(sdp));
    debug_info("[sdp]o_sess_version:%s\n",sdp_message_o_sess_version_get(sdp));
    debug_info("[sdp]o_nettype:%s\n",sdp_message_o_nettype_get(sdp));
    debug_info("[sdp]o_addrtype:%s\n",sdp_message_o_addrtype_get(sdp));
    debug_info("[sdp]o_addr:%s\n",sdp_message_o_addr_get(sdp));   
    debug_info("[sdp]s_name:%s\n",sdp_message_s_name_get(sdp));
    debug_info("[sdp]i_info:%s\n",sdp_message_i_info_get(sdp,0));    
    debug_info("[sdp]payload 0 0:%s\n",sdp_message_m_payload_get(sdp,0,0));
    debug_info("[sdp]payload 0 1:%s\n",sdp_message_m_payload_get(sdp,0,1));
    debug_info("[sdp]payload 0 2:%s\n",sdp_message_m_payload_get(sdp,0,2));
    debug_info("[sdp]payload 1 0:%s\n",sdp_message_m_payload_get(sdp,1,0));
    debug_info("[sdp]payload 1 1:%s\n",sdp_message_m_payload_get(sdp,1,1));
    debug_info("[sdp]payload 1 2:%s\n",sdp_message_m_payload_get(sdp,1,2));
    debug_info("[sdp]port 0 :%s\n",sdp_message_m_port_get(sdp,0));
    debug_info("[sdp]port 1:%s\n",sdp_message_m_port_get(sdp,1));
    
    debug_info("[video]:c_nettype:%s c_addrtype:%s c_addr:%s c_addr_multicast_ttl:%s c_addr_multicast_int:%s\n",
        connection->c_nettype,
        connection->c_addrtype,
        connection->c_addr,
        connection->c_addr_multicast_ttl,
        connection->c_addr_multicast_int
        );

    connection = eXosip_get_audio_connection(sdp);
    debug_info("[sdp]v_version:%s o_username:%s o_sess_id:%s o_sess_version:%s o_nettype:%s \
              o_addrtype:%s  o_addr:%s s_name:%s i_info:%s u_uri:%s z_adjustments:%s\n",
        sdp->v_version,
        sdp->o_username,
        sdp->o_sess_id,
        sdp->o_sess_version,
        sdp->o_nettype,
        sdp->o_addrtype,
        sdp->o_addr,
        sdp->s_name,
        sdp->i_info,
        sdp->u_uri,
        sdp->z_adjustments);
    debug_info("[audio]:c_nettype:%s c_addrtype:%s c_addr:%s c_addr_multicast_ttl:%s c_addr_multicast_int:%s\n",
        connection->c_nettype,
        connection->c_addrtype,
        connection->c_addr,
        connection->c_addr_multicast_ttl,
        connection->c_addr_multicast_int
        );
    debug_info("[video]m_media:%s  port:%s m_number_of_port:%s m_proto:%s i_info:%s\n",
        video_sdp->m_media,
        video_sdp->m_port,
        video_sdp->m_number_of_port,
        video_sdp->m_proto,
        video_sdp->i_info);
    debug_info("[audio]m_media:%s  port:%s m_number_of_port:%s m_proto:%s i_info:%s\n",
        audio_sdp->m_media,
        audio_sdp->m_port,
        audio_sdp->m_number_of_port,
        audio_sdp->m_proto,
        audio_sdp->i_info);
#endif  

    return 0;
}

static int sip_event_parse_sdp_audio_rate(char *audio_rate, int audio_rate_len, void *sdp)
{
    if ((NULL == sdp) || (NULL == audio_rate))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int pos_media = 0;
    sdp_attribute_t *attr = NULL;
    int i = 0;
    int max_media_elem_num = 10;
    sdp_message_t *sdp_msg = (sdp_message_t *)sdp;
    
    sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp_msg->m_medias, 0);
    while (med != NULL)
    {
      if ((NULL != med->m_media) && (0 == osip_strcasecmp (med->m_media, "audio")))
      {
          break;
      }
      pos_media++;
      med = (sdp_media_t *) osip_list_get (&sdp_msg->m_medias, pos_media);
    }
    for (i = 0; i < max_media_elem_num; i++)
    {
        attr = sdp_message_attribute_get(sdp_msg,pos_media,i);
        if (NULL == attr)
        {
            break;
        }
        else
        {
            char *p = NULL;
            if (NULL != attr->a_att_value)
            {
                p = strstr(attr->a_att_value,"/");
                if (p)
                {
                    strncpy(audio_rate,p+1, audio_rate_len);
                    break;
                }
                //debug_info("[%d]a_att_field:%s a_att_value:%s\n",i,attr->a_att_field,attr->a_att_value);
            }
        }
    }

    return 0;
}

static int sip_event_parse_addr(SIP_EVENT_HANDLE handle, int index, void *request)
{
    if ((NULL == handle) || (NULL == request))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    int ret = -1;
    osip_via_t *via = NULL;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        osip_message_get_via((osip_message_t *)request,0,&via);
        if (NULL != via)
        {
            if (NULL != via->host)
            {
                snprintf(dev_user.ip,sizeof(dev_user.ip), "%s", via->host);
            }
            if (NULL != via->port)
            {
                dev_user.port = atoi(via->port);
            }
            sip_user_set_all_info(handle->user_handle, &dev_user, index);
            debug_info("sip user[%d] ip:%s:%d\n", index, dev_user.ip, dev_user.port);
        }
    }

    return 0;
}

static int sip_event_parse_call_num(SIP_EVENT_HANDLE handle, int index, void *request)
{
    if ((NULL == handle) || (NULL == request))
    {
        debug_err("invalid param\n");
        return -1;
    }
    if ((index < 0)  || (index >= MAX_USER_CNT))
    {
        debug_err("index %d invalid param.\n", index);
        return -1;
    }
    int ret = -1;
    osip_from_t *from = NULL;
    SIP_CONFIG sip_config;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    memset(&sip_config, 0x00, sizeof(sip_config));
    ret = sip_conf_get_config(handle->config_handle, &sip_config);
    if (0 != ret)
    {
        debug_err("sip_conf_get_config get failed\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        from = osip_message_get_from((osip_message_t *)request);
        if (NULL != from)
        {
            if ((NULL != from->url) && (NULL != from->url->username))
            {
                snprintf(dev_user.call_num, sizeof(dev_user.call_num), "%s", from->url->username);
            }
            snprintf(dev_user.from_call_num, sizeof(dev_user.from_call_num), "%s", dev_user.call_num);
            snprintf(dev_user.to_call_num, sizeof(dev_user.to_call_num), "%s", sip_config.src_call_num);
            sip_user_set_all_info(handle->user_handle, &dev_user, index);
            debug_info("sip user call num:%s from_call_num:%s to_call_num:%s\n",
                            dev_user.call_num,
                            dev_user.from_call_num,
                            dev_user.to_call_num);
        }
    }

    return 0;
}

static int sip_event_send_message(SIP_EVENT_HANDLE handle, char * msg_buf, char *sip_call_id, char *call_num)
{
    if ((NULL == handle) || (NULL == msg_buf) || (NULL == call_num))
    {
        debug_err("invalid param \n");
        return -1;
    }
    if (NULL == handle->context_exosip)
    {
        debug_err("please invite at first. \n");
        return -1;
    }
    osip_message_t * message = NULL;
    int i = 0, index = -1, ret = -1;
    char src_call[64] = {'\0'};
    char dst_call[64] = {'\0'};
    SIP_USER_INFO dev_user;
    SIP_CONFIG sip_config;
    memset(&dev_user, 0x00, sizeof(dev_user));
    memset(&sip_config, 0x00, sizeof(sip_config));
    sip_conf_get_config(handle->config_handle, &sip_config);
    
    // "sip:133@127.0.0.1:5061";
    snprintf(src_call, sizeof(src_call), "sip:%s@%s:%d", sip_config.src_call_num, sip_config.src_ip, sip_config.src_port);
    if (NULL == sip_call_id)
    {
        index = sip_user_find_user_from_call_num(handle->user_handle, call_num);
        if ((index < 0) || (index >= MAX_USER_CNT))
        {
            debug_err("not found call num sip user\n");
            return -1;
        }
        ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
        if (0 == ret)
        {
            if (1 == sip_config.is_connect_sever)
            {
                snprintf(dst_call, sizeof(dst_call), "sip:%s@%s:%d", dev_user.call_num, sip_config.svr_ip, sip_config.svr_port);
            }
            else
            {
                snprintf(dst_call, sizeof(dst_call), "sip:%s@%s:%d", dev_user.call_num, dev_user.ip, dev_user.port);
            }
        }
    }
    else/*心跳包消息，只往服务器发送心跳包*/
    {
        snprintf(dst_call, sizeof(dst_call), "sip:%s@%s:%d", call_num, sip_config.svr_ip, sip_config.svr_port);
    }
    // 传输MESSAGE方法,也就是即时消息,和INFO方法相比,我认为主要区别是:
    // MESSAGE不用建立连接,直接传输信息,而INFO消息必须在建立INVITE的基础上传输
    /*如果dst_call src_call地址没有端口或者端口出错，会导致osip_message_set_body出现段错误*/
    ret = eXosip_message_build_request(handle->context_exosip, &message,
                                            "MESSAGE", dst_call, src_call,NULL);
    if ((0 != ret) || (NULL == message))
    {
        debug_err("eXosip_message_build_request failed,ret:%d\n", ret);
        return -1;
    }
    /* 内容, 方法,     to, from, route*/
    osip_message_set_body(message, msg_buf, strlen(msg_buf));
    // 假设格式是xml
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_message_send_request(handle->context_exosip, message);
    if ((NULL != message) && (NULL != message->call_id) && 
        (NULL != message->call_id->number) && (NULL != sip_call_id))
    {
        snprintf(sip_call_id, 32, "%s", message->call_id->number);
        debug_info("call id:%s\n",message->call_id->number);
    }
    debug_info("src_call:%s dst_call:%s send message ok!! \n", src_call, dst_call);
    
    return 0;
}

static int sip_event_send_keep_alive_msg(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if(NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    debug_info("sip_event_send_keep_alive_msg !!\n");
    char msg_buf[512] = {'\0'};
    SIP_CONFIG sip_config;
    memset(&sip_config, 0x00, sizeof(sip_config));
    sip_conf_get_config(handle->config_handle, &sip_config);
    /*心跳包的呼叫号码默认为 hello，但是服务器现在不解析它*/
    sip_event_send_message(handle, msg_buf, sip_call_id, "hello");

    return 0;
}

static int sip_event_set_timer_calling_timeout(SIP_EVENT_HANDLE handle, int time, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    int ret = -1;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 != ret)
    {
        debug_err("not found index\n");
        return -1;
    }
    //dev_user.timer_id_calling = (int)get_real_time_usec();
    debug_info("timer_id_calling:%d\n", dev_user.timer_id_calling);
    sip_timer_set(handle->timer_handle, dev_user.timer_id_calling, time, sip_event_time_out_disconnect, (void *)handle);
    sip_user_set_all_info(handle->user_handle, &dev_user, index);

    return 0;
}

static int sip_event_set_timer_request_server_timeout(SIP_EVENT_HANDLE handle, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    int ret = -1;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found call num sip user\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 != ret)
    {
        debug_err("not found index\n");
        return -1;
    }
    //dev_user.timer_id_server_req = (int)get_real_time_usec();
    sip_timer_set(handle->timer_handle, dev_user.timer_id_server_req,
                     INVITE_DOORLOCK_RECV_100_TRYING_TIME_OUT,
                     sip_event_recv_100_trying_time_out,
                     (void *)handle);
    sip_user_set_all_info(handle->user_handle, &dev_user, index);

    return 0;
}


static int sip_event_time_out_disconnect(int time_id, void *arg)
{
    if (NULL == arg)
    {
        debug_err("invalid param\n");
        return -1;
    }
    debug_info("sip_event_time_out_disconnect\n");
    
    SIP_EVENT_HANDLE handle = (SIP_EVENT_OBJECT *)arg;
    SIP_SESSION_INFO sip_session;
    SIP_USER_INFO dev_user;
    int index = -1;
    memset(&sip_session, 0x00, sizeof(sip_session));
    memset(&dev_user, 0x00, sizeof(dev_user));
    index = sip_user_find_user_from_calling_timer_id(handle->user_handle, &dev_user, time_id);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found calling timer id sip user\n");
        return -1;
    }
    /*通知上层模块，因为异常重置session回话*/
    if (NULL != handle->ses_cb)
    {
        memcpy(sip_session.call_num, dev_user.call_num, sizeof(sip_session.call_num));
        //add something
    }
    dev_user.end_time = dev_user.start_time;
    sip_user_set_all_info(handle->user_handle, &dev_user, index);
    sip_event_session_terminate(handle, &sip_session);

    return 0;
}

static int sip_event_kill_all_timer(SIP_EVENT_HANDLE handle, int index)
{
    if (NULL == handle)
    {
        debug_err("invalid param\n");
        return -1;
    }
    int ret = -1;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 != ret)
    {
        debug_err("not found index\n");
        return -1;
    }
    if (0 != dev_user.timer_id_calling)
    {
        sip_timer_kill(handle->timer_handle, dev_user.timer_id_calling);
        dev_user.timer_id_calling = 0;
    }
    if (0 != dev_user.timer_id_server_req)
    {
        sip_timer_kill(handle->timer_handle, dev_user.timer_id_server_req);
        dev_user.timer_id_server_req = 0;
    }
    sip_user_set_all_info(handle->user_handle, &dev_user, index);
    
    return 0;
}

/*test:后面确认一下门口机和服务器都会回复100try*/
static int sip_event_recv_100_trying_time_out(int time_id, void *arg)
{
    if(NULL == arg)
    {
        return -1;
    }
    int index = -1, ret = -1;
    SIP_EVENT_HANDLE handle = (SIP_EVENT_OBJECT *)arg;
    debug_err("sip_timer_recv_100_trying_time_out\n");
    SIP_USER_INFO dev_user;
    SIP_SESSION_INFO sip_session;
    memset(&dev_user, 0x00, sizeof(dev_user));
    memset(&sip_session, 0x00, sizeof(sip_session));
    index = sip_user_find_user_from_server_timer_id(handle->user_handle, &dev_user, time_id);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found call num sip user\n");
        return -1;
    }
    ret = sip_user_get_media(handle->user_handle, &sip_session, index);
    if (0 == ret)
    {
        sip_event_session_terminate(handle, &sip_session);
        //add something
    }
    
    return 0;
}

static int sip_event_recv_100_trying(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int index = -1, ret = -1;
    SIP_USER_INFO dev_user;
    memset(&dev_user, 0x00, sizeof(dev_user));
    index = sip_user_find_user_from_sip_call_id(handle->user_handle, sip_call_id);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found call num sip user\n");
        return -1;
    }
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        if (0 != dev_user.timer_id_server_req)
        {
            sip_timer_kill(handle->timer_handle, dev_user.timer_id_server_req);
            dev_user.timer_id_server_req = 0;
            sip_user_set_all_info(handle->user_handle, &dev_user, index);
        }
    }
    
    return 0;
}

static int sip_event_check_answer(SIP_EVENT_HANDLE handle, char *sip_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    int index = -1, ret = -1;
    SIP_USER_INFO dev_user;
    index = sip_user_find_user_from_sip_call_id(handle->user_handle, sip_call_id);
    if ((index < 0) || (index >= MAX_USER_CNT))
    {
        debug_err("not found sip_call_id sip user\n");
        return -1;
    }
    memset(&dev_user, 0x00, sizeof(dev_user));
    ret = sip_user_get_all_info(handle->user_handle, &dev_user, index);
    if (0 == ret)
    {
        sip_timer_set(handle->timer_handle, dev_user.timer_id_calling, LOCAL_CALLING_TIME_OUT,
                        sip_event_time_out_disconnect, (void *)handle);
        dev_user.answer = 1;
        //dev_user.start_time = get_real_time_msec();
        sip_user_set_all_info(handle->user_handle, &dev_user, index);
        debug_info("The remoter user answering a call!!\n");
    }
    
    return index;
}
#ifdef SIP_KEEP_ALIVE
static int sip_event_keep_alive_time_out(int time_id, void *arg)
{
    if (NULL == arg)
    {
        return -1;
    }
    SIP_EVENT_HANDLE handle = (SIP_EVENT_OBJECT *)arg;
    SIP_CONFIG sip_config;
    memset(&sip_config, 0x00, sizeof(sip_config));
    if (handle->keep_alive.timeout_num >= 2)
    {
        sip_conf_get_config(handle->config_handle, &sip_config);
        sip_config.is_connect_sever = 0;
        debug_err("sip_timer keep alive timeout,num:%d \n",handle->keep_alive.timeout_num);
        sip_event_register(handle);
    }
    handle->keep_alive.timeout_num++;
    
    return 0;
}

static int sip_event_clear_keep_alive_flag(SIP_EVENT_HANDLE handle, char *sip_call_id, char *keep_alive_call_id)
{
    if ((NULL == handle) || (NULL == sip_call_id) || (NULL == keep_alive_call_id))
    {
        debug_err("invalid param\n");
        return -1;
    }
    SIP_CONFIG sip_config;
    memset(&sip_config, 0x00, sizeof(sip_config));
    if (0 == strcmp(sip_call_id, keep_alive_call_id))
    {
        sip_conf_get_config(handle->config_handle, &sip_config);
        sip_config.is_connect_sever = 1;
        debug_info("The server reply a keepalive!!!\n");
    }
    else
    {
        return -1;
    }

    return 0;
}
#endif
