#ifndef __SIP_COMMON_H
#define __SIP_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#define debug_err(fmt,...)       do { if (1) fprintf(stderr, "\033[0;31m[sip][error][%s:%s():%d]:" fmt, __FILE__, __func__,__LINE__, ##__VA_ARGS__);} while (0)
#define debug_info(fmt,...)      do { if (1) fprintf(stderr, "\033[0;32m[sip][info][%s:%s():%d]:" fmt, __FILE__,__func__, __LINE__,##__VA_ARGS__); } while (0)
#define debug_warning(fmt,...)   do { if (1) fprintf(stderr, "\033[0;33m[sip][warning][%s:%s():%d]:" fmt, __FILE__,__func__, __LINE__,##__VA_ARGS__); } while (0)

#define UNUSED_VALUE(x) ((void)(x))

#define TINY_CHAT_SDK_VERSION "tc_sdk_v.1.0.0"
//#define SIP_KEEP_ALIVE 1

#define MAX_USER_CNT   2


#define DEFAULT_SIP_PORT             5060
#define VI_H264_PAYLOAD              96
#define VI_G711_PAYLOAD              0
#define VI_G711_PAYLOAD_STRING       "PCMA"

enum SIP_EVENT_TYPE
{
    SIP_EVENT_INVITE = 1000,
    SIP_EVENT_RING,
    SIP_EVENT_ANSWER,
    SIP_EVENT_ACK,
    SIP_EVENT_BUSY,
    SIP_EVENT_BYE,
};

typedef struct _SIP_USER_EVENT_INFO
{
    char call_num[32];   /*远程呼叫号码*/
}SIP_USER_EVENT_INFO;

typedef struct _SIP_CONFIG
{
    char username[32];/*用户名为当前设备的device id*/
    char password[32];
    char wan_ip[16];
    int transport;      // 0 UDP 1 TCP 2 TLS 3 DTLS.
    char svr_ip[16];    /* 代理服务器地址*/
    unsigned int svr_port;      /* 代理服务器端口*/    
    char src_ip[16];    // local ip
    unsigned int src_port;      // local port
    char src_call_num[16];
    char src_device_id[32];
    int is_connect_sever;
}SIP_CONFIG;

typedef struct _SIP_EXOSIP_EVENT
{
    char sip_call_id[64];  /*sip 的唯一会话ID*/

    int tid; /**< unique id for transactions (to be used for answers) */
    int did; /**< unique id for SIP dialogs */

    int rid; /**< unique id for registration */
    int cid; /**< unique id for SIP calls (but multiple dialogs!) */
    int sid; /**< unique id for outgoing subscriptions */
    int nid; /**< unique id for incoming subscriptions */
}SIP_EXOSIP_EVENT;


typedef struct _SIP_USER_SDP_INFO
{
    char user_name[32];
    char device_addr[32];
    char media_addr[32];
    char video_port[8];
    char v_payload[8];
    char audio_port[8];
    char a_payload[8];
    char audio_rate[8];
}SIP_USER_SDP_INFO;

typedef struct _SIP_USER_INFO
{
    int is_use;
    char call_num[32];   /*远程呼叫号码*/
    char from_call_num[32];
    char to_call_num[32];
    char ip[32];        /*远程用户IP*/
    unsigned int port;  /*远程用户端口*/
    char invite_time[32];
    long long start_time;
    long long end_time;
    int answer;
    int lock;
    char url[32];
    int timer_id_server_req;/*请求timer*/
    int timer_id_calling;/*通话timer*/
    SIP_EXOSIP_EVENT session;
    SIP_USER_SDP_INFO sdp;
}SIP_USER_INFO;

typedef struct _SIP_SESSION_INFO
{
    /*唯一呼叫号码*/
    char call_num[16];
    /*本地端口地址信息*/
    char local_ip[16];
    unsigned short local_vport;
    unsigned short local_aport;
    /*设备端口地址信息*/
    char dev_ip[16];
    unsigned short dev_vport;
    unsigned short dev_aport;
    /*扩展数据*/
    int len;
    char data[0];
}SIP_SESSION_INFO;

typedef void (*MEDIA_DATA_CB)(void *arg, void *param, char *data, int len);
typedef int (* MEDIA_CMD_CB)(void * arg, int cmd, void *param);

typedef void (*THREAD_FUNC)(void *arg);

#ifdef __cplusplus
}
#endif

#endif
