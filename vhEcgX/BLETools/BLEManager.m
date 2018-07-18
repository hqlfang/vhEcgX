//
//  BLEManager.m
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import "BLEManager.h"
#import "CFilters.h"

NSString *const BLE_SERVICE_UUID = @"6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
NSString *const BLE_UART_RX_UUID = @"6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
NSString *const BLE_UART_TX_UUID = @"6E400003-B5A3-F393-E0A9-E50E24DCCA9E";


static CLowpassFilter2 *lowPass=NULL;

@implementation BLEManager


- (CBUUID *)SERVICE_UUID
{
    static __strong CBUUID *_serviceUUID = nil;
    if (!_serviceUUID)
        _serviceUUID = [CBUUID UUIDWithString:BLE_SERVICE_UUID];
    return _serviceUUID;
}


- (CBUUID *)UART_RX_UUID
{
    static __strong CBUUID *_uartRxUUID = nil;
    if (!_uartRxUUID)
        _uartRxUUID = [CBUUID UUIDWithString:BLE_UART_RX_UUID];
    return _uartRxUUID;
}


- (CBUUID *)UART_TX_UUID
{
    static __strong CBUUID *_uartTxUUID = nil;
    if (!_uartTxUUID)
        _uartTxUUID = [CBUUID UUIDWithString:BLE_UART_TX_UUID];
    return _uartTxUUID;
}


+ (BLEManager *)sharedInstance{
    static BLEManager *singleton = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (singleton == nil) {
            singleton = [[BLEManager alloc] init];
        }
    });
    return singleton;
}

- (instancetype)init{
    self = [super init];
    if (self) {
        self.bleCentralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        self.peripheralArray = [NSMutableArray arrayWithCapacity:0];
        self.reciveDataArray = [NSMutableArray arrayWithCapacity:0];
        lowPass = new CLowpassFilter2(40,500);

    }
    return self;
}

//开始扫描外设
- (void)startScanPeriperals{
    [self.bleCentralManager scanForPeripheralsWithServices:nil options:nil];
}

//停止扫描
- (void)stopScan{
    [self.bleCentralManager stopScan];
}

//连接外设
- (void)connectToPeripheral{
    [self.bleCentralManager connectPeripheral:self.connectPeripheral options:nil];
}

//断开连接
- (void)cancelConnectWithPeripheral{
    [self stopScan];
    [self.bleCentralManager cancelPeripheralConnection:self.connectPeripheral];
    self.isConnectPer = NO;
}

- (void)stopReadDataWithCharacteristic{
    [self.connectPeripheral setNotifyValue:NO forCharacteristic:self.uartTxCharacteristic];
    [self cancelConnectWithPeripheral];
}
/*
 * 读取信号量
 * read RSSI
 */
- (void)readRSSIWithPeriperal{
    [self.connectPeripheral readRSSI];
}

#pragma mark - CBCentralManagerDelegate
-(void)centralManagerDidUpdateState:(CBCentralManager *)central{
    if (central.state != CBManagerStatePoweredOn) {
        NSLog(@"CBCentralManagerStatePoweredOff");
        return;
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *, id> *)advertisementData RSSI:(NSNumber *)RSSI{

    NSLog(@"didDiscoverPeripheral %@ advertisementData==%@", peripheral, advertisementData);
    NSLog(@"serviceUUID==%@", [advertisementData objectForKey:@"kCBAdvDataServiceUUIDs"]);
    if (![self.peripheralArray containsObject:peripheral]) {
        [self.peripheralArray addObject:peripheral];
    }
    if (self.delegate && [self.delegate respondsToSelector:@selector(bleManagerdidDiscoverPeripheral:)]) {
        [self.delegate bleManagerdidDiscoverPeripheral:self.peripheralArray];
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral{
    peripheral.delegate = self;
    [peripheral discoverServices:@[[self SERVICE_UUID]]];
//    self.connectPeripheral = peripheral;
    NSLog(@"连接成功");
    if ([self.connectPeripheral isEqual:peripheral] ) {
        if (self.isConnectPer) {
            
        } else {
            self.isConnectPer = YES;
            if (self.delegate && [self.delegate respondsToSelector:@selector(bleManagerdidConnectSuccessPeripheral)]) {
                [self.delegate bleManagerdidConnectSuccessPeripheral];
            }
            
        }
     
    }
}
 

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(nullable NSError *)error{
    NSLog(@"连接失败， %@", [error description]);
    
    if ([self.connectPeripheral isEqual:peripheral]){
        [self.bleCentralManager connectPeripheral:peripheral options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(nullable NSError *)error{
    //断开连接
    self.isConnectPer = NO;
}



#pragma mark - CBPeripheralDelegate
-(void)peripheral:(CBPeripheral *)peripheral didReadRSSI:(NSNumber *)RSSI error:(NSError *)error{
    DLOG(@"RSSI ==%@", RSSI);
}

-(void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error{
    
    for (CBService *service in peripheral.services){
        if ([service.UUID isEqual:[self SERVICE_UUID]]) {
            [peripheral discoverCharacteristics:@[[self UART_TX_UUID], [self UART_RX_UUID]] forService:service];
            
        }
    }
    DLOG(@"didDiscoverServices ==%@", peripheral.services);
}

-(void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error{

    
    for (CBCharacteristic *characteristic in service.characteristics) {
        [peripheral readValueForCharacteristic:characteristic];
        if ([characteristic.UUID isEqual:[self UART_TX_UUID]]) {
            self.uartTxCharacteristic = characteristic;
//            [self.reciveCharacteristicArray addObject:characteristic];
            [peripheral setNotifyValue:YES forCharacteristic:characteristic];
        }
    }
    NSLog(@"didDiscoverCharacteristicsForService ==%@", service.characteristics);
    
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    NSLog(@"didUpdateValueForCharacteristic ===%@", characteristic.value);
    [self didReceiveData:characteristic.value];
}

/*
 /// ADC增益 6倍
 const float ADC_GAIN = 4f;
 /// 1个ADC数据单位对应的uV值
 /// </summary>
 const float EMG_uV = (0x07fffff + 1) / 0x3A / VREF;
 /// 参考电压正输入 uV
 /// </summary>
 const float VREFP = 2.4f * 1000000;//uV
 /// <summary>
 /// 参考电压负输入 uV
 /// </summary>
 const float VREFN = 0f * 1000000;//uV
 /// <summary>
 /// 参考电压
 /// </summary>
 const float VREF = VREFP - VREFN;
 AdcData_uV = (int)(AdcData / ADC_GAIN / EMG_uV);//uv
 */



- (void)didReceiveData:(NSData *)data{
    NSInteger length = data.length;
    Byte bytes[length];
    [data getBytes:bytes length:length];
    float VREFN = 0.0f * 1000000;//uV
    float VREFP = 2.4f * 1000000;//uV
    float VREF = VREFP - VREFN;

    float EMG_uV = (0x07fffff + 1) / VREF;
    float ADC_GAIN = 6.0f;

    static int j=0;
    for (int i=1; i<length-1; i=i+3)
    {
        Byte b1 = bytes[i];
        Byte b2 = bytes[i+1];
        Byte b3 = bytes[i+2];
        int numberInt = b1<<24 | b2<<16 | b3;
        int numberInt1 = numberInt>>8;
//        numberInt1=2000*sin(2*3.1415926*10/500*j++);
        int AdcData_uV = (int)(numberInt1 / ADC_GAIN / EMG_uV);
        NSLog(@"AdcData_uV===%ld", (long)AdcData_uV);

        if (lowPass != nil) {
            lowPass->Filter(AdcData_uV);//低通
        }
        
        NSLog(@"AdcData_uV===%ld", (long)AdcData_uV);

        NSNumber *number = [NSNumber numberWithInt:AdcData_uV];
        [self.reciveDataArray addObject:number];

    }
    NSLog(@"didReceiveUARTData reciveDataArray length===%lu", (unsigned long)self.reciveDataArray.count);
    //    NSLog(@"didReceiveUARTData reciveDataArray===%@", self.reciveDataArray);
    
}


- (void)didReceiveUARTData:(NSData *)data{
    NSInteger length = data.length;
    Byte bytes[length];
    [data getBytes:bytes length:length];
    [self.reciveDataArray removeAllObjects];
    for (int i=1; i<length-1; i=i+3)
    {
        Byte b1 = bytes[i];
        Byte b2 = bytes[i+1];
        Byte b3 = bytes[i+2];
        int numberInt = b1*65535 + b2*256 + b3;
        NSNumber *number = [NSNumber numberWithInt:(numberInt-16777216)];
        [self.reciveDataArray addObject:number];
    }
    NSLog(@"didReceiveUARTData reciveDataArray length===%lu", (unsigned long)self.reciveDataArray.count);
    
    NSLog(@"didReceiveUARTData reciveDataArray===%@", self.reciveDataArray);
    
}





@end
