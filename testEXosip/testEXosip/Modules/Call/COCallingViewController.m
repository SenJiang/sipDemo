//
//  COCallingViewController.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "COCallingViewController.h"
#import "COLabel.h"
#import "COButton.h"
#import "Masonry.h"
#import "COHelpHeader.h"

@interface COCallingViewController ()
/**
背景图片
*/
@property (nonatomic,strong) UIImageView * bgImageView;

/**
 对方昵称显示
 */
@property (nonatomic,strong) COLabel * nameLabel;

/**
 对方账号显示
 */
@property (nonatomic,strong) COLabel * sipnumLabel;

/**
 通话状态显示
 */
@property (nonatomic,strong) COLabel * labelStatus;

/**
 免提按钮
 */
@property (nonatomic,strong) COButton * handsFreeBtn;

/**
 静音按钮
 */
@property (nonatomic,strong) COButton * muteBtn;

/**
 拒绝/挂断按钮
 */
@property (nonatomic,strong) COButton *buttonHangup;

/**
 接受按钮
 */
@property (nonatomic,strong) COButton *buttonAccept;
@end

@implementation COCallingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = COUIColorBlack;
    [self initUI];
}
-(void)initUI{
    
    _bgImageView = [[UIImageView alloc]initWithFrame:self.view.bounds];
    _bgImageView.userInteractionEnabled = YES;
    _bgImageView.backgroundColor = [UIColor blackColor];
    [self.view addSubview:_bgImageView];
    
    
    _nameLabel = [[COLabel alloc]initWithText:nil font:MiddleFont color:COUIColorWhite];
    _nameLabel.textAlignment = NSTextAlignmentCenter;
    [_bgImageView addSubview:_nameLabel];
    
    
    _labelStatus = [[COLabel alloc]initWithText:@"语音请求中..." font:LargeFont color:COUIColorWhite];
    _labelStatus.frame = CGRectMake(0, CGRectGetMaxY(_nameLabel.frame) + 30, kScreenWidth, 30);
    _labelStatus.textAlignment = NSTextAlignmentCenter;
    [_bgImageView addSubview:_labelStatus];
    
    
    //免提
    /*
     非ARC 下
     build setting -> Apple LLVM7.1 - Language - Objective C -> Weak References in Manual Retain Release YES。
     */
    WeakSelf
    _handsFreeBtn = [[COButton alloc]initWithTitle:@"免提" font:MiddleFont color:COUIColorWhite selectColor:COUIColorRed];
    _handsFreeBtn.block = ^(NSString * reMark){
        StrongSelf
        [self btnClick:@"免提"];
    };
    [_bgImageView addSubview:_handsFreeBtn];
    
    
    //静音
    _muteBtn = [[COButton alloc]initWithTitle:@"静音" font:MiddleFont color:COUIColorWhite selectColor:COUIColorRed];
    _muteBtn.block = ^(NSString * reMark){
        StrongSelf
        [self btnClick:@"静音"];
    };
    [_bgImageView addSubview:_muteBtn];
    
    
    //挂断
    _buttonHangup = [[COButton alloc]initWithTitle:@"挂断" font:MiddleFont color:COUIColorRed selectColor:nil];
    _buttonHangup.block = ^(NSString * reMark){
        StrongSelf
        [self btnClick:@"挂断"];
    };
    [_buttonHangup layerCornerRadius:15.0f borderWidth:1.0f borderColor:COUIColorRed];
    [_bgImageView addSubview:_buttonHangup];
    
    //接受
    _buttonAccept = [[COButton alloc]initWithTitle:@"接受" font:MiddleFont color:COUIColorGreen selectColor:nil];
    _buttonAccept.hidden = YES;
    _buttonAccept.block = ^(NSString * reMark){
        StrongSelf
        [self btnClick:@"接受"];
    };
    [_buttonAccept layerCornerRadius:15.0f borderWidth:1.0f borderColor:COUIColorGreen];
    [_bgImageView addSubview:_buttonAccept];
    
    [_nameLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        StrongSelf
        make.left.equalTo(self.bgImageView).with.offset(20);
        make.right.equalTo(self.bgImageView).with.offset(-20);
        make.top.equalTo(self.bgImageView).with.offset(100);
        make.height.equalTo(@30);
    }];
    
    [_labelStatus mas_makeConstraints:^(MASConstraintMaker *make) {
        StrongSelf
        make.left.equalTo(self.nameLabel);
        make.right.equalTo(self.nameLabel);
        make.top.equalTo(self.nameLabel.mas_bottom).with.offset(10);
        make.height.equalTo(@30);
    }];
    
    [_handsFreeBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        StrongSelf
        make.left.equalTo(self.bgImageView).with.offset(15);
        make.width.equalTo(@45);
        make.top.equalTo(self.bgImageView.mas_bottom).with.offset(-60);
        make.height.equalTo(@30);
        
    }];
    
    [_muteBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        StrongSelf
        make.right.equalTo(self.bgImageView).with.offset(-15);
        make.width.equalTo(@45);
        make.top.equalTo(self.handsFreeBtn);
        make.height.equalTo(@30);
    }];
    
    [_buttonAccept mas_makeConstraints:^(MASConstraintMaker *make) {
        StrongSelf
        make.left.equalTo(self.buttonHangup);
        make.right.equalTo(self.buttonHangup);
        make.bottom.equalTo(self.buttonHangup.mas_top).with.offset(-10);
        make.height.equalTo(self.buttonHangup);
    }];
    
    [_buttonHangup mas_makeConstraints:^(MASConstraintMaker *make) {
        StrongSelf
        make.left.equalTo(self.handsFreeBtn.mas_right).with.offset(20);
        make.right.equalTo(self.muteBtn.mas_left).with.offset(-20);
        make.top.equalTo(self.handsFreeBtn);
        make.height.equalTo(@30);
    }];
}

- (void) btnClick:(NSString *)remark{
    if ([remark isEqualToString:@"免提"]) {
        [self handsFreeBtnClick];
    }
    else if ([remark isEqualToString:@"静音"]) {
        [self muteBtnClick];
    }
    else if ([remark isEqualToString:@"挂断"]) {
        [self buttonHangupClick];
    }
    else if ([remark isEqualToString:@"接受"]) {
        [self buttonAcceptClick];
    }
    
}

//挂断
- (void) buttonHangupClick{
    //挂断
    [self.navigationController dismissViewControllerAnimated:YES completion:nil];
}

//接受
- (void) buttonAcceptClick{
    
}

//免提
- (void)handsFreeBtnClick{
    self.handsFreeBtn.selected =  !self.handsFreeBtn.selected;
}

//静音
- (void)muteBtnClick{
    
//    if([audioSession setMute:![audioSession isMuted]]){
        self.muteBtn.selected = !self.muteBtn.selected;//[audioSession isMuted];
//    }
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
