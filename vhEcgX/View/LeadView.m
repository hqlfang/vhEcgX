//
//  LeadView.m
//  GJLightBlueTooth
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 Jun Gao. All rights reserved.
//

#import "LeadView.h"

@implementation LeadView

- (id)initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor whiteColor];
        self.clearsContextBeforeDrawing = YES;
        NSInteger _pointsPerSecond = 500; //采样率 每秒多个点
        _pixelsPerMm = 6.16;//每毫米 6.16 像素
        _mmPerMV = 10;  //每毫伏 10 毫米
        _pixelsPerUV = _mmPerMV * _pixelsPerMm / 1000.0;//5 * 10.0 / 1000; //每微伏 多少 个像素
        _mmPerSecond = 25; //每秒 25 毫米
        pixelsPerPoint = (_pixelsPerMm * _mmPerSecond) / _pointsPerSecond;  //每个点 多少  像素
        lineWidth = 1.2f;
        uVpb = 1.0;//1.272;
    }
    return self;
}

- (void)drawRect:(CGRect)rect
{
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    [self drawGrid:context];
    [self drawCurve:context];
}

- (void)transitionToPoint{
    CGFloat pos_x = 0;
    CGFloat pos_y = 0;
    CGFloat pos_x_offset = 0.0;
    CGFloat full_height = self.frame.size.height;
    
    for (int i=0; i<self.pointsArray.count; i++) {
        
        pos_x = pos_x + pos_x_offset + pixelsPerPoint;
        
        pos_y = full_height/2 - [[self.pointsArray objectAtIndex:i] intValue] * uVpb * _pixelsPerUV;
        
        drawingPoints[i] = CGPointMake(pos_x, pos_y);
    }
    
}

- (void)drawGrid:(CGContextRef)ctx {
    CGFloat full_height = self.frame.size.height;
    CGFloat full_width = self.frame.size.width;
    CGFloat cell_square_width = 6.16*5;
    
    CGContextSetLineWidth(ctx, 0.2);
    CGContextSetStrokeColorWithColor(ctx, [UIColor lightGrayColor].CGColor);
    
    int pos_x = 1;
    while (pos_x < full_width) {
        CGContextMoveToPoint(ctx, pos_x, 1);
        CGContextAddLineToPoint(ctx, pos_x, full_height);
        pos_x += cell_square_width;
        
        CGContextStrokePath(ctx);
    }
    
    CGFloat pos_y = 1;
    while (pos_y <= full_height) {
        
        CGContextSetLineWidth(ctx, 0.2);
        
        CGContextMoveToPoint(ctx, 1, pos_y);
        CGContextAddLineToPoint(ctx, full_width, pos_y);
        pos_y += cell_square_width;
        
        CGContextStrokePath(ctx);
    }
    
    
    CGContextSetLineWidth(ctx, 0.1);
    
    cell_square_width = cell_square_width / 5;
    pos_x = 1 + cell_square_width;
    while (pos_x < full_width) {
        CGContextMoveToPoint(ctx, pos_x, 1);
        CGContextAddLineToPoint(ctx, pos_x, full_height);
        pos_x += cell_square_width;
        
        CGContextStrokePath(ctx);
    }
    
    pos_y = 1 + cell_square_width;
    while (pos_y <= full_height) {
        CGContextMoveToPoint(ctx, 1, pos_y);
        CGContextAddLineToPoint(ctx, full_width, pos_y);
        pos_y += cell_square_width;
        
        CGContextStrokePath(ctx);
    }
}

- (void)drawCurve:(CGContextRef)ctx
{
    if (self.pointsArray.count<=0) {
        return;
    }
    CGContextSetLineWidth(ctx, lineWidth);
    CGContextSetStrokeColorWithColor(ctx, [UIColor blackColor].CGColor);
    
    CGContextAddLines(ctx, drawingPoints, self.pointsArray.count);
    CGContextStrokePath(ctx);
    
}


@end
