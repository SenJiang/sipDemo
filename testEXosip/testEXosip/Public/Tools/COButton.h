//
//  COButton.h
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef void(^COButtonBlock)(NSString * reMark);

@interface COButton : UIButton

/**
 标签
 默认设置为title
 */
@property (nonatomic,strong) NSString * reMark;

@property (nonatomic,copy) COButtonBlock block;


- (instancetype)initWithTitle:(NSString *)title font:(UIFont *)font;

- (instancetype)initWithTitle:(NSString *)title font:(UIFont *)font color:(UIColor *)color selectColor:(UIColor *) sColor;

- (instancetype)initWithTitle:(NSString *)title font:(UIFont *)font withImage:(NSString *)imageName;

- (instancetype)initWithTitle:(NSString *)title font:(UIFont *)font withImage:(NSString *)imageName selectImage:(NSString *)selectImageName;

/**
 设置layer
 */
- (void) layerCornerRadius:(CGFloat)radius borderWidth:(CGFloat)border borderColor:(UIColor *)color;


@end
