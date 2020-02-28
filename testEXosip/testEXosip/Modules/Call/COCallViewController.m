//
//  COViewController.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "COCallViewController.h"
#import "COButton.h"
#import "COLabel.h"
#import "masonry.h"
#import "COHelpHeader.h"
#import "COCallingViewController.h"
#import "CONavViewController.h"

@interface COCallViewController ()

@property (nonatomic,strong) COLabel * dailLabel;

@end

@implementation COCallViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.navigationItem.title = @"拨号";
    self.view.backgroundColor = COUIColorf1efef;
    [self initUI];
}
- (void)initUI{
    CGRect stateBarFrame = [[UIApplication sharedApplication] statusBarFrame];
    self.dailLabel = [[COLabel alloc]initWithText:@"" font:[UIFont boldSystemFontOfSize:35]];
    self.dailLabel.userInteractionEnabled = YES;
    self.dailLabel.frame = CGRectMake(0, stateBarFrame.size.height+44, kScreenWidth, 100);
    self.dailLabel.backgroundColor = [UIColor whiteColor];
    [self.view addSubview:self.dailLabel];
    
    COButton * plusBtn = [[COButton alloc] initWithTitle:@"" font:nil withImage:@"Plus2" selectImage:nil];
    plusBtn.reMark = @"增加";
    plusBtn.block = ^(NSString * reMark){
        [self btnClickBtn:reMark];
    };
    [self.dailLabel addSubview:plusBtn];
    
    COButton * deleteBtn = [[COButton alloc] initWithTitle:@"" font:nil withImage:@"Delete" selectImage:nil];
    deleteBtn.reMark = @"<-";
    deleteBtn.block = ^(NSString * reMark){
        [self btnClickBtn:reMark];
    };
    [self.dailLabel addSubview:deleteBtn];
    
    [plusBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(self.dailLabel).with.offset(20);
        make.width.equalTo(@30);
        make.top.equalTo(self.dailLabel).with.offset(25);
        make.height.equalTo(@40);
    }];
    
    [deleteBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.right.equalTo(self.dailLabel).with.offset(-20);
        make.width.equalTo(@50);
        make.top.equalTo(plusBtn);
        make.height.equalTo(plusBtn);
    }];
    
    NSArray * numArr = @[@"1",@"2",@"3",
                         @"4",@"5",@"6",
                         @"7",@"8",@"9",
                         @"*",@"0",@"#"
                         ];
    
    CGFloat btnWidth = 80;
    CGFloat btnLeft = (kScreenWidth - 3 * btnWidth) / 4.0;
    
    for (int i = 0; i < numArr.count; i ++) {
        COButton * btn = [[COButton alloc]initWithTitle:numArr[i] font:[UIFont systemFontOfSize:35]];
        [btn setBackgroundImage:[UIImage imageNamed:@"btn_num"] forState:UIControlStateNormal];
        btn.block = ^(NSString * reMark){
            
            [self btnClickBtn:reMark];
        };
        [self.view addSubview:btn];
        
        
        if (i != 9) {
            [btn setTitleEdgeInsets:UIEdgeInsetsMake(-8, 0, 0, 0)];
        }
        else{
            [btn setTitleEdgeInsets:UIEdgeInsetsMake(8, 0, 0, 0)];
        }
        
        [btn mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(self.view).with.offset(i%3*(btnWidth + btnLeft) + btnLeft);
            make.top.equalTo(self.dailLabel.mas_bottom).with.offset(i/3*(btnWidth + 10) + 20);
            make.width.equalTo(@(btnWidth));
            make.height.equalTo(@(btnWidth));
        }];
    }
    
    NSArray * dailArr = @[@"语音通话",@"视频通话"];
    NSArray * imageArr = @[@"phonecall",@"vedio"];
    NSArray * colorArr = @[COUIColorFromColorValue(0x448aca),COUIColorFromColorValue(0x8fd06c)];
    
    CGFloat dailWidth = 130;
    CGFloat dailLeft = (kScreenWidth - 2 * dailWidth) / 3.0;
    CGFloat btnBottomY = numArr.count / 3 * (btnWidth + 10) + 20 + btnWidth;
    for (int i = 0; i < dailArr.count; i ++) {
        
        COButton * btn = [[COButton alloc]initWithTitle:dailArr[i] font:[UIFont systemFontOfSize:16] withImage:imageArr[i] selectImage:nil];
        [btn setTitleColor:COUIColorWhite forState:UIControlStateNormal];
        UIColor * color = colorArr[i];
        btn.backgroundColor = color;
        [btn layerCornerRadius:8.0f borderWidth:1.0f borderColor:color];
        btn.block = ^(NSString * reMark){
            
            [self btnClickBtn:reMark];
        };
        [self.view addSubview:btn];
        [btn mas_makeConstraints:^(MASConstraintMaker *make) {
            make.left.equalTo(self.view).with.offset(i*(dailWidth + dailLeft) + dailLeft);
            make.top.equalTo(self.dailLabel).with.offset(btnBottomY + 30);
            make.width.equalTo(@(dailWidth));
            make.height.equalTo(@50);
        }];
        
    }
}

- (void)btnClickBtn:(NSString *)remark{
    
    NSString * text = self.dailLabel.text;
    if ([remark isEqualToString:@"语音通话"]) {
        COCallingViewController *callingVC = [[COCallingViewController alloc]init];
        callingVC.view.backgroundColor = COUIColorBlack;
        CONavViewController *nav = [[CONavViewController alloc]initWithRootViewController:callingVC];
        nav.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
        [self presentViewController:nav animated:YES completion:nil];
        
    }else if ([remark isEqualToString:@"视频通话"]){

    }else if ([remark isEqualToString:@"<-"]){
        if (text.length != 0) {
            self.dailLabel.text = [text substringToIndex:self.dailLabel.text.length - 1];
        }else{
            self.dailLabel.text = @"";
        }
    }else if ([remark isEqualToString:@"增加"]){
    }else{
        self.dailLabel.text = [text stringByAppendingString:remark];
    }
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
