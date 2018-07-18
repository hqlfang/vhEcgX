//
//  HomeViewController.h
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "BLEManager.h"

@interface HomeViewController : UIViewController<UITableViewDelegate, UITableViewDataSource, BLEManagerDelegate>{
    UITableView *bleTableView;
}

@property (nonatomic, strong)NSMutableArray *bleDeviceList;

@end
