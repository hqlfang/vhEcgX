//
//  LeadView.h
//  GJLightBlueTooth
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 Jun Gao. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LeadView : UIView{
    CGFloat lineWidth;
    CGPoint drawingPoints[100000];
    CGFloat _mmPerMV, _pixelsPerMm, _voltageScale, _mmPerSecond, _pixelsPerUV, pixelsPerPoint, uVpb;

}
@property (nonatomic, strong) NSMutableArray *pointsArray;
- (void)transitionToPoint;


@end
