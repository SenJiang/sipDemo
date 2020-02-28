//
//  COTabBarViewController.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "COTabBarViewController.h"
#import "CONavViewController.h"
#import "COCallViewController.h"
#import "COContactViewController.h"
#import "COMessageViewController.h"
#import "COSettingViewController.h"



@interface COTabBarViewController ()

@end

@implementation COTabBarViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self initUI];
}

- (void)initUI{
    CONavViewController *callNav = [[CONavViewController alloc]initWithRootViewController:[[COCallViewController alloc] init]];
    callNav.tabBarItem.image = [[UIImage imageNamed:@"tab_call"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    callNav.tabBarItem.selectedImage = [[UIImage imageNamed:@"tab_call_s"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    callNav.tabBarItem.title = @"拨号";
    
    CONavViewController *messageNav = [[CONavViewController alloc]initWithRootViewController:[[COMessageViewController alloc] init]];
    messageNav.tabBarItem.image = [[UIImage imageNamed:@"tab_message"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    messageNav.tabBarItem.selectedImage = [[UIImage imageNamed:@"tab_message_s"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    messageNav.tabBarItem.title = @"信息";
    
    CONavViewController *contactNav = [[CONavViewController alloc]initWithRootViewController:[[COContactViewController alloc] init]];
    contactNav.tabBarItem.image = [[UIImage imageNamed:@"tab_contact"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    contactNav.tabBarItem.selectedImage = [[UIImage imageNamed:@"tab_contact_s"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    contactNav.tabBarItem.title = @"联系人";
    
    CONavViewController *settingNav = [[CONavViewController alloc]initWithRootViewController:[[COSettingViewController alloc] init]];
    settingNav.tabBarItem.image = [[UIImage imageNamed:@"tab_Settings"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    settingNav.tabBarItem.selectedImage = [[UIImage imageNamed:@"tab_Settings_s"] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
    settingNav.tabBarItem.title = @"设置";
    
    self.tabBar.tintColor = [UIColor blackColor];
    self.viewControllers = @[callNav,messageNav,contactNav,settingNav];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
