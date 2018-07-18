//
//  CollectDataViewController.m
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/17.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import "CollectDataViewController.h"
#import "BLEManager.h"

@interface CollectDataViewController ()

@end

@implementation CollectDataViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor whiteColor];
    self.title = @"Collect data";
    [self setupUI];
}

- (void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:10.0 repeats:NO block:^(NSTimer * _Nonnull timer) {
        [[BLEManager sharedInstance] stopReadDataWithCharacteristic];
        [self drawLine];
    }];
}

- (void)setupUI{
    scrollV = [UIScrollView new];
    scrollV.backgroundColor = [UIColor colorWithRed:240/255.0 green:240/255.0 blue:240/255.0 alpha:1.0];
    [self.view addSubview:scrollV];
    [scrollV mas_makeConstraints:^(MASConstraintMaker *make) {
        make.edges.mas_equalTo(self.view);
    }];
    scrollV.delegate = self;
    [scrollV setShowsVerticalScrollIndicator:NO];
    [scrollV setShowsHorizontalScrollIndicator:NO];
    holder = [UIView new];
    holder.backgroundColor = [UIColor colorWithRed:240/255.0 green:240/255.0 blue:240/255.0 alpha:1.0];
    [scrollV addSubview:holder];
    [holder mas_makeConstraints:^(MASConstraintMaker *make) {
        make.edges.equalTo(self->scrollV);
    }];
    scrollV.maximumZoomScale = 2.0;
    scrollV.minimumZoomScale = 0.5;
    
    NSInteger _pointsPerSecond = 500; //采样率 每秒多个点
    _pixelsPerMm = 6.16;//每毫米 6.16 像素
    _mmPerMV = 10;  //每毫伏 10 毫米
    _pixelsPerUV = 5 * 10.0 / 1000; //每微伏 多少 个像素
    _mmPerSecond = 25; //每秒 25 毫米
    pixelsPerPoint = (_pixelsPerMm * _mmPerSecond) / _pointsPerSecond;  //每个点 多少  像素
    

}

- (void)drawLine{
    
    [scrollV setContentSize:CGSizeMake(pixelsPerPoint*[BLEManager sharedInstance].reciveDataArray.count, self.view.frame.size.height)];
    
    lead = [[LeadView alloc] initWithFrame:CGRectMake(0, 100, pixelsPerPoint*[BLEManager sharedInstance].reciveDataArray.count, 400)];
    [holder addSubview:lead];
    
    lead.pointsArray = [BLEManager sharedInstance].reciveDataArray;
    [lead transitionToPoint];
    [lead setNeedsDisplay];
}

@end
