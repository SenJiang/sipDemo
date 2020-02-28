//
//  COLabel.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "COLabel.h"

@implementation COLabel

- (instancetype) initWithText:(NSString *)text{
    if (self = [super init]) {
        [self text:text color:nil font:nil];
    }
    return self;
}

- (instancetype) initWithText:(NSString *)text color:(UIColor *)color{
    if (self = [super init]) {
        [self text:text color:color font:nil];
    }
    return self;
}

- (instancetype) initWithText:(NSString *)text font:(UIFont *)font{
    if (self = [super init]) {
        [self text:text color:nil font:font];
    }
    return self;
}

- (instancetype) initWithText:(NSString *)text font:(UIFont *)font color:(UIColor *)color{
    if (self = [super init]) {
        [self text:text color:color font:font];
    }
    return self;
}

- (void) text:(NSString *)text color:(UIColor *)color font:(UIFont *)font{
    
    self.textAlignment = NSTextAlignmentCenter;
    
    if (text) {
        self.text = text;
    }
    
    if (color) {
        self.textColor = color;
    }
    
    if (font) {
        self.font = font;
    }
    
}
/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
