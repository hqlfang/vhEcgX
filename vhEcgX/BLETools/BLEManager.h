//
//  BLEManager.h
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol BLEManagerDelegate <NSObject>
@optional
- (void)bleManagerdidDiscoverPeripheral:(NSArray *)peripheralArray;
- (void)bleManagerdidConnectSuccessPeripheral;


@end


@interface BLEManager : NSObject<CBCentralManagerDelegate, CBPeripheralDelegate>{

}

//系统蓝牙设备管理对象，可以把他理解为主设备，通过他，可以去扫描和链接外设
@property (nonatomic, strong)CBCentralManager *bleCentralManager;
//当前链接的设备
@property (nonatomic, strong)CBPeripheral *connectPeripheral;
//保存扫描到的设备
@property (nonatomic, strong) NSMutableArray *peripheralArray;
//连接的特征值
@property(strong, nonatomic) CBCharacteristic *uartTxCharacteristic;
//读取数据的数组
@property(nonatomic,strong)NSMutableArray *reciveDataArray;
//当前外设是否已经连接
@property(nonatomic,assign)BOOL isConnectPer;


+ (BLEManager *)sharedInstance;
@property (weak, nonatomic) NSObject <BLEManagerDelegate> *delegate;

- (void)startScanPeriperals;
- (void)stopScan;
- (void)connectToPeripheral;
- (void)cancelConnectWithPeripheral;
- (void)stopReadDataWithCharacteristic;

@end
