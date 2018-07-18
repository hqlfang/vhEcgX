//
//  HomeViewController.m
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import "HomeViewController.h"
#import "CollectDataViewController.h"

@interface HomeViewController ()

@end

@implementation HomeViewController

- (void)scanBleDevice{
    [[BLEManager sharedInstance] startScanPeriperals];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor redColor];
    self.title = @"BLE Device";
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"扫描" style:UIBarButtonItemStylePlain target:self action:@selector(scanBleDevice)];
    _bleDeviceList = [NSMutableArray arrayWithCapacity:0];
    [self setupUI];
    [BLEManager sharedInstance].delegate = self;
}

- (void)setupUI{
    bleTableView = [UITableView new];
    [self.view addSubview:bleTableView];
    bleTableView.backgroundColor = [UIColor whiteColor];
    bleTableView.delegate = self;
    bleTableView.dataSource = self;
    [bleTableView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.edges.mas_equalTo(self.view);
    }];
    
    
}

- (void)bleManagerdidDiscoverPeripheral:(NSArray *)peripheralArray{
    self.bleDeviceList = [BLEManager sharedInstance].peripheralArray;
    [bleTableView reloadData];
}

- (void)bleManagerdidConnectSuccessPeripheral{
    [[BLEManager sharedInstance] stopScan];
    CollectDataViewController * ctr = [[CollectDataViewController alloc] init];
    [self.navigationController pushViewController:ctr animated:YES];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath{
    return 80;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section{
    return self.bleDeviceList.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath{
    static NSString *cellID = @"cellID";

    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellID];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellID];
        cell.selectionStyle = UITableViewCellSelectionStyleGray;
    }
    CBPeripheral *peripheral = self.bleDeviceList[indexPath.row];
    cell.textLabel.text = peripheral.name;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath{
    [tableView deselectRowAtIndexPath:indexPath animated:NO];
    CBPeripheral *per = [self.bleDeviceList objectAtIndex:indexPath.row];
    [BLEManager sharedInstance].connectPeripheral = per;
    [[BLEManager sharedInstance] connectToPeripheral];
    [[BLEManager sharedInstance].reciveDataArray removeAllObjects];
}

@end
