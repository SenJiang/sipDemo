//
//  COButton.m
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import "COButton.h"

@implementation COButton

-(instancetype)initWithTitle:(NSString *)title font:(UIFont *)font{
    
    if (self = [super init]) {
        
        [self title:title image:nil selectImage:nil font:font];
        
    }
    return self;
}
-(instancetype)initWithTitle:(NSString *)title font:(UIFont *)font color:(UIColor *)color selectColor:(UIColor *) sColor{
    
    if (self = [super init]) {
        
        
        [self title:title image:nil selectImage:nil font:font];
        
        
        if (color) {
            [self setTitleColor:color forState:UIControlStateNormal];
        }
        
        if (sColor) {
            [self setTitleColor:sColor forState:UIControlStateSelected];
        }
        
    }
    return self;
}

-(instancetype)initWithTitle:(NSString *)title font:(UIFont *)font withImage:(NSString *)imageName{
    
    if (self = [super init]) {
        
        [self title:title image:imageName selectImage:nil font:font];
    }
    return self;
}


-(instancetype)initWithTitle:(NSString *)title font:(UIFont *)font withImage:(NSString *)imageName selectImage:(NSString *)selectImageName{
    
    if (self = [super init]) {
        [self title:title image:imageName selectImage:selectImageName font:font];
    }
    return self;
}


- (void)title:(NSString *)title image:(NSString *)image selectImage:(NSString *)selectImage font:(UIFont *)font {
    
    self.reMark = @"";
    
    if (title) {
        self.reMark = title;
        [self setTitle:title forState:UIControlStateNormal];
        [self setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    }
    
    if (image) {
        [self setImage:[UIImage imageNamed:image] forState:UIControlStateNormal];
    }
    
    if (selectImage) {
        [self setImage:[UIImage imageNamed:selectImage] forState:UIControlStateSelected];
    }
    
    if (font) {
        self.titleLabel.font = font;
    }
    
    [self addTarget:self action:@selector(btnClick) forControlEvents:UIControlEventTouchUpInside];
}

-(void)btnClick{
    
    if (self.block) {
        self.block(self.reMark);
    }
}

/**
 设置layer
 */
- (void) layerCornerRadius:(CGFloat)radius borderWidth:(CGFloat)border borderColor:(UIColor *)color{
    self.layer.cornerRadius = radius;
    self.layer.borderWidth = border;
    self.layer.borderColor = color.CGColor;
}
/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
