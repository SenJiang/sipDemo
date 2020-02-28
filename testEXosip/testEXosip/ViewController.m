//
//  ViewController.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/2.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "ViewController.h"
#import "osip.h"
#import "eXosip.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "sip_session.h"

#import "NSString+addres.h"

#define USER1_NAME "test20"
#define USER1_PWD  "test20"
#define USER1_PORT 7800

@interface ViewController ()

@end

@implementation ViewController


static int  media_cmd_cb(void * arg, int cmd, void *param)
{
    if (NULL == param)
    {
        debug_err("invald param\n");
        return -1;
    }
    SIP_USER_EVENT_INFO *pEvent_user = (SIP_USER_EVENT_INFO *)param;
    debug_info("cmd:%d call_num:%s\n", cmd, pEvent_user->call_num);
    
    return 0;
}

static int media_data_cb(void *arg, char *param, char *data, int len)
{
    UNUSED_VALUE(arg);
    UNUSED_VALUE(param);
    UNUSED_VALUE(data);
    UNUSED_VALUE(len);
    
    debug_info("audio_data_cb,len:%d", len);
    
    return 0;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    
//    [self request];
    // 当前手机地址
    NSString *iphone_ip = [NSString getIPAddress];
    printf("current iphone ip addres is:%s",[iphone_ip UTF8String]);
    
//    NSString *iphone_port = [NSString getRouterPort];
//    printf("current iphone ip pory is:%s",[iphone_port UTF8String]);
    int i = 0;
    SIP_SESSION_HANDLE sip_handle = NULL;
    SIP_CONFIG config;
    SIP_SESSION_INFO session;
    memset(&config, 0x00, sizeof(config));
    memset(&session, 0x00, sizeof(session));
    config.transport = 0;//udp
    snprintf(config.username, sizeof(config.username), "%s", USER1_NAME);
    snprintf(config.password, sizeof(config.username), "%s", USER1_PWD);
    snprintf(config.wan_ip, sizeof(config.wan_ip), "%s", [iphone_ip UTF8String]);
    snprintf(config.svr_ip, sizeof(config.svr_ip), "%s", "182.61.51.229");
    config.svr_port = DEFAULT_SIP_PORT;
    snprintf(config.src_ip, sizeof(config.src_ip), "%s", [iphone_ip UTF8String]);
    config.src_port = USER1_PORT;
    memcpy(config.src_call_num, "test20", sizeof(config.src_call_num));
    memcpy(session.call_num, "test11", sizeof(session.call_num));
    session.dev_vport = 60000;
    session.dev_aport = 60002;
    sip_handle= sip_session_create(NULL);
    if (NULL == sip_handle)
    {
        debug_err("no memery");
        return ;
    }
    sip_session_start(sip_handle, &config);
    sip_session_set_cb(sip_handle, media_cmd_cb, media_data_cb);
    while(1)
    {
        if (i == 50)
        {
            sip_session_call(sip_handle, &session);
        }
        i++;
        usleep(100*1000);
    }
    sip_session_destroy(sip_handle);
    return ;
//
//    eXosip_event_t *je = NULL;
//    osip_message_t *ack = NULL;
//    osip_message_t *invite = NULL;
//    osip_message_t *answer = NULL;
//    sdp_message_t *remote_sdp = NULL;
//    int call_id, dialog_id;
//    int i,j;
//    int id;
//
//    char *sour_call = "sip:test@182.61.51.229";
//    char *dest_call = "sip:test@182.61.51.229:5061"; //client ip/port
//
//    char command;
//    char tmp[4096];
//    char localip[128];
//    int pos = 0;
//
//    struct eXosip_t *excontext = NULL;
//    // 初始化 sip
//
//    excontext = (struct eXosip_t *)malloc(sizeof(excontext));
//    i = eXosip_init(excontext);
//    if (i != 0){
//        printf("\n\t--> Can't initialize eXosip!\n");
//        return ;
//    }else{
//         printf("\n\t--> eXosip_init successfully!\n");
//    }
//
//    i = eXosip_listen_addr(excontext, IPPROTO_UDP, "182.61.51.229", 5060, AF_INET, 0); //(IPPROTO_UDP, NULL, 5060, AF_INET, 0);
//    if (i != 0){
//        eXosip_quit(excontext);
//        printf("\n\t--> eXosip_listen_addr error! Couldn't initialize transport layer!\n");
//    }
//    for(;;){
//        // 侦听是否有消息到来
//        je = eXosip_event_wait(excontext, 0, 50);
//        // 协议栈带有此语句,具体作用未知
//        eXosip_lock(excontext);
//        eXosip_default_action(excontext, je);
//        eXosip_automatic_action(excontext);
//        eXosip_unlock(excontext);
//        if (je == NULL) // 没有接收到消息，继续
//        {
//            continue;
//        }
//
//        switch (je->type)   {
//
//        case EXOSIP_MESSAGE_NEW: // 新的消息到来
//            printf("\n\t*** EXOSIP_MESSAGE_NEW!\n");
//            if (MSG_IS_MESSAGE (je->request)) // 如果接收到的消息类型是 MESSAGE
//            {
//                {
//                    osip_body_t *body;
//                    osip_message_get_body (je->request, 0, &body);
//                    printf("I get the msg is: ");
//                }
//                // 按照规则，需要回复 OK 信息
//                eXosip_message_build_answer(excontext, je->tid, 200, &answer);
//                eXosip_message_send_answer (excontext,je->tid, 200, answer);
//            }
//            break;
//
//        case EXOSIP_CALL_INVITE: // INVITE 请求消息
//            // 得到接收到消息的具体信息
//
//            // 得到消息体,认为该消息就是 SDP 格式.
//            remote_sdp = eXosip_get_remote_sdp(excontext ,je->did);
//            call_id = je->cid;
//            dialog_id = je->did;
//
//            eXosip_lock (excontext);
//
//            eXosip_call_send_answer (excontext,je->tid, 180, NULL);
//            i = eXosip_call_build_answer (excontext,je->tid, 200, &answer);
//            if (i != 0)
//            {
//                printf("\n\t--> This request msg is invalid! Cann't response!\n");
//                eXosip_call_send_answer (excontext,je->tid, 400, NULL);
//            }
//            else
//            {
//                snprintf (tmp, 4096,
//                          "v=0\r\n"
//                          "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
//                          "t=1 10\r\n"
//                          "a=username:rainfish\r\n"
//                          "a=password:123\r\n");
//                // 设置回复的SDP消息体,下一步计划分析消息体
//                // 没有分析消息体，直接回复原来的消息，这一块做的不好。
//                osip_message_set_body (answer, tmp, strlen(tmp));
//                osip_message_set_content_type (answer, "application/sdp");
//
//                eXosip_call_send_answer (excontext,je->tid, 200, answer);
//                printf("\n\t--> send 200 over!");
//            }
//            eXosip_unlock (excontext);
//
//            // 显示出在 sdp 消息体中的 attribute 的内容,里面计划存放我们的信息
//            printf( "\n\t--> The INFO is :\n" );
//            while (!osip_list_eol ( &(remote_sdp->a_attributes), pos))
//            {
//                sdp_attribute_t *at;
//                //这里解释了为什么在SDP消息体中属性a里面存放必须是两列
//                at = (sdp_attribute_t *) osip_list_get ( &remote_sdp->a_attributes, pos);
//                pos ++;
//            }
//            break;
//        case EXOSIP_CALL_ACK:
//            printf("\n\t--> ACK recieved!\n" );
//            // printf ("the cid is %s, did is %s\n", je->did, je->cid);
//            break;
//        case EXOSIP_CALL_CLOSED:
//            printf("\n\t--> the remote hold the session!\n");
//            // eXosip_call_build_ack(dialog_id, &ack);
//            // eXosip_call_send_ack(dialog_id, ack);
//            i = eXosip_call_build_answer (excontext,je->tid, 200, &answer);
//            if (i != 0)
//            {
//                printf ("This request msg is invalid!Cann't response!\n");
//                eXosip_call_send_answer (excontext,je->tid, 400, NULL);
//            }
//            else
//            {
//                eXosip_call_send_answer (excontext,je->tid, 200, answer);
//                printf("\n\t--> bye send 200 over!\n");
//            }
//            break;
//
//        case EXOSIP_CALL_MESSAGE_NEW:
//
//            printf("\n\t*** EXOSIP_CALL_MESSAGE_NEW\n");
//            if (MSG_IS_INFO(je->request) ) // 如果传输的是 INFO 方法
//            {
//                eXosip_lock (excontext);
//                i = eXosip_call_build_answer (excontext,je->tid, 200, &answer);
//                if (i == 0)
//                {
//                    eXosip_call_send_answer (excontext,je->tid, 200, answer);
//                }
//                eXosip_unlock (excontext);
//                {
//                    osip_body_t *body;
//                    osip_message_get_body (je->request, 0, &body);
//                    printf("the body is ");
//                }
//
//            }
//
//            break;
//
//        default:
//
//            printf("\n\t--> Could not parse the msg!\n");
//
//        }
//
    //    }

}

- (void)request{
    NSString* wifiUrl = @"https://blog.csdn.net/yxys01/article/details/77186561?locationNum=10&fps=1";
     wifiUrl = [wifiUrl stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet characterSetWithCharactersInString:@"`#%^{}\"[]|\\<> "].invertedSet];
    __block NSError *__error = nil;
    __block NSData *__result = nil;
    
    NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
    
    NSURLSession *session = [NSURLSession sessionWithConfiguration:config];
    NSMutableURLRequest  * request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:wifiUrl]];
    [request setHTTPMethod:@"GET"];
    
    request.timeoutInterval = 10;
    
    NSURLSessionDataTask *task = [session dataTaskWithRequest:request completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        
        if (error) {
            __error = error;
        }else __result = data;
        
    }];
    [task resume];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
