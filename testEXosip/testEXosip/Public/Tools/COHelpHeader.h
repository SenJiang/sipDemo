//
//  COHelpHeader.h
//  testEXosip
//
//  Created by jiangdesheng on 2018/8/7.
//  Copyright © 2018年 JXW. All rights reserved.
//

#ifndef COHelpHeader_h
#define COHelpHeader_h


#define kScreenWidth [UIScreen mainScreen].bounds.size.width

/**
 弱引用
 */
#define WeakSelf                                 __weak typeof(self) _weak = self;
#define StrongSelf                               __strong typeof(_weak) self = _weak;


/*
 * 颜色转换
 */
#define COUIColorFromColorValue(ColorValue)      [UIColor \
colorWithRed:((float)((ColorValue & 0xFF0000) >> 16))/255.0 \
green:((float)((ColorValue & 0xFF00) >> 8))/255.0 \
blue:((float)(ColorValue & 0xFF))/255.0 alpha:1.0]

#define COUIColorFromColorValueAlpha(ColorValue,Alpha)      [UIColor \
colorWithRed:((float)((ColorValue & 0xFF0000) >> 16))/255.0 \
green:((float)((ColorValue & 0xFF00) >> 8))/255.0 \
blue:((float)(ColorValue & 0xFF))/255.0 alpha:Alpha]

//根据RGB自定义颜色
#define COUIColorFromRGB(r, g, b) [UIColor colorWithRed:(r)/255.0 green:(g)/255.0 blue:(b)/255.0 alpha:1.0];
#define COUIColorFromRGB_ALPHA(r, g, b, a) [UIColor colorWithRed:(r)/255.0 green:(g)/255.0 blue:(b)/255.0 alpha:(a)]

/**
 * 颜色
 */
#define COUIColorGreen                           [UIColor greenColor]
#define COUIColorRed                             [UIColor redColor]
#define COUIColorWhite                           [UIColor whiteColor]
#define COUIColorGray                            [UIColor grayColor]
#define COUIColorLightGray                       [UIColor lightGrayColor]
#define COUIColorBlack                           [UIColor blackColor]
#define COUIColorBlue                            [UIColor blueColor]
#define COUIColorClearColor                      [UIColor clearColor]
#define COUIColor222222                          COUIColorFromColorValue(0x222222)
#define COUIColor353535                          COUIColorFromColorValue(0x353535)
#define COUIColor19a4fe                          COUIColorFromColorValue(0x19a4fe)
#define COUIColorf1efef                          COUIColorFromColorValue(0xf1efef)
#define COUI_SET_SEPARATOR_COLOR                 COUIColorFromColorValue(0x444444)
#define COUIColorCustomBlue                      [UIColor colorWithRed:40/255.0 green:166/255.0 blue:250/255.0 alpha:1]
#define COUIColor_TableView_Backgroud_Color      COUIColorFromRGB(22, 22, 22)
#define COUIColor_TabBarColor

// ************************ 字号 *****************************
/**
 *  小号字体
 */
#define SmallFont [UIFont systemFontOfSize:11]
/**
 *  中号字体
 */
#define MiddleFont [UIFont systemFontOfSize:14]
/**
 *  正常字体
 */
#define LargeFont [UIFont systemFontOfSize:16]

#endif /* COHelpHeader_h */
