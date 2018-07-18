//
//  AppDelegate.h
//  vhEcgX
//
//  Created by 谷山丰 on 2018/7/16.
//  Copyright © 2018年 谷山丰. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreData/CoreData.h>
#import "HomeViewController.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) HomeViewController *home;
@property (strong, nonatomic) UINavigationController *navigationController;

@property (readonly, strong) NSPersistentContainer *persistentContainer;

- (void)saveContext;


@end

