//
//  NSString+addres.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/4.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "NSString+addres.h"
#import <ifaddrs.h>
#import <arpa/inet.h>

@implementation NSString (addres)
+(NSString *)getIPAddress {
    NSString *address = @"error";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // 检索当前接口,在成功时,返回0
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // 循环链表的接口
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                // 检查接口是否en0 wifi连接在iPhone上
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"]) {
                    // 得到NSString从C字符串
                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // 释放内存
    freeifaddrs(interfaces);
    return address;
}
+ (NSString *)getRouterPort{
    
    NSString *address = @"error";
    
    struct ifaddrs *interfaces = NULL;
    
    struct ifaddrs *temp_addr = NULL;
    
    int success = 0;
    
    // retrieve the current interfaces - returns 0 on success
    
    success = getifaddrs(&interfaces);
    
    if (success == 0)   {
        
        // Loop through linked list of interfaces
        
        temp_addr = interfaces;
        
        //*/
        
        while(temp_addr != NULL)
            
        /*/
         
         int i=255;
         
         while((i--)>0)
         
         //*/
            
        {
            
            if(temp_addr->ifa_addr->sa_family == AF_INET)
                
            {
                
                // Check if interface is en0 which is the wifi connection on the iPhone
                
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"])
                    
                {
                    
                    // Get NSString from C String //ifa_addr
                    
                    //ifa->ifa_dstaddr is the broadcast address, which explains the "255's"
                    
                    //                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_dstaddr)->sin_addr)];
                    
                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                    
                    //routerIP----192.168.1.255 广播地址
                    
                    NSLog(@"broadcast address--%@",[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_dstaddr)->sin_addr)]);
                    
                    //--192.168.1.106 本机地址
                    
                    NSLog(@"local device ip--%@",[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)]);
                    
                    //--255.255.255.0 子网掩码地址
                    
                    NSLog(@"netmask--%@",[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_netmask)->sin_addr)]);
                    
                    //--en0 端口地址
                    
                    NSLog(@"interface--%@",[NSString stringWithUTF8String:temp_addr->ifa_name]);
                    
                }
                
            }
            
            temp_addr = temp_addr->ifa_next;
            
        }
        
    }
    
    // Free memory
    
    freeifaddrs(interfaces);
    
    
    return [NSString stringWithUTF8String:temp_addr->ifa_name];
    
}



@end
