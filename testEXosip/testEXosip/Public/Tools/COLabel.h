//
//  COLabel.h
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface COLabel : UILabel

- (instancetype) initWithText:(NSString *)text;

- (instancetype) initWithText:(NSString *)text color:(UIColor *)color;

- (instancetype) initWithText:(NSString *)text font:(UIFont *)font;

- (instancetype) initWithText:(NSString *)text font:(UIFont *)font color:(UIColor *)color;

@end
