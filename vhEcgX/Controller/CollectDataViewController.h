//
//  CollectDataViewController.h
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/17.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LeadView.h"

@interface CollectDataViewController : UIViewController<UIScrollViewDelegate>{
    UIScrollView *scrollV;
    UIView *holder;
    LeadView *lead;
    CGFloat _mmPerMV, _pixelsPerMm, _voltageScale, _mmPerSecond, _pixelsPerUV, pixelsPerPoint, uVpb;

}

@end
