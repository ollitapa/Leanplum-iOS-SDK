//
//  LPWebInterstitialViewControllerTest.m
//  Leanplum-SDK_Tests
//
//  Created by Nikola Zagorchev on 14.07.20.
//  Copyright © 2020 Leanplum. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <OCMock/OCMock.h>
#import "Leanplum+Extensions.h"
#import <Leanplum/LPWebInterstitialViewController.h>
#import <Leanplum/LPMessageTemplateConstants.h>
#import "LeanplumHelper.h"

/*
 * Tests WebView delegate methods of LPWebInterstitialViewController
 */
@interface LPWebInterstitialViewControllerTest : XCTestCase

@end

@implementation LPWebInterstitialViewControllerTest

- (void)setUp
{
    [super setUp];
}

- (void)tearDown
{
    [super tearDown];
}

/*
 * Use this for Xcode13 instead of [LPWebInterstitialViewController instantiateFromStoryboard]
 * Xcode13 cannot find the bundle using the current logic in LPUtils:leanplumBundle
 * NSBundle for Leanplum class returns the xctest from the app bundle:
 * ../LeanplumSDKApp.app/PlugIns/LeanplumSDKTests.xctest
 */
- (LPWebInterstitialViewController*)webInterstitialController
{
    NSBundle *bundle = [NSBundle mainBundle];
    NSURL *bundleUrl = [bundle URLForResource:@"Leanplum-iOS-SDK" withExtension:@".bundle" subdirectory:@"Frameworks/Leanplum.framework"];
    if (bundleUrl != nil)
    {
        NSBundle *lpBundle = [NSBundle bundleWithURL:bundleUrl];
        bundle = lpBundle;
    }
    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:@"WebInterstitial" bundle:bundle];
    
    return [storyboard instantiateInitialViewController];
}

// Blocks for decidePolicyForNavigationAction
typedef void (^AfterBlock)(UIApplication *mockApplication, WKNavigationActionPolicy policy);
typedef void (^BeforeBlock)(UIApplication *mockApplication);

/*
 * Executes the WKWebView decidePolicyForNavigationAction
 * @param url The URL passed to the WKWebView for the WKNavigationAction
 * @param initialPolicy The action policy
 * @param beforeBlock Test block to execute before calling decidePolicyForNavigationAction
 * @param afterBlock Test block to execute after calling decidePolicyForNavigationAction
 */
- (void) execute_decidePolicyForNavigationAction:(NSURL *)url withInitialPolicy:(WKNavigationActionPolicy)initialPolicy withBeforeBlock:(BeforeBlock)beforeBlock withAfterBlock:(AfterBlock)afterBlock
{
    LPWebInterstitialViewController *viewController = [self webInterstitialController];
    
    WKWebView *currentWebView;
    for (id view in viewController.view.subviews) {
        if ([view isMemberOfClass:WKWebView.class]) {
            currentWebView = view;
        }
    }
    
    LPActionContext *context = [LPActionContext actionContextWithName:LPMT_WEB_INTERSTITIAL_NAME args:@{
        LPMT_ARG_URL_CLOSE: LPMT_DEFAULT_CLOSE_URL
    } messageId:0];
    viewController.context = context;
    
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    id action = OCMPartialMock([[WKNavigationAction alloc] init]);
    id reqMock = OCMStub([action request]);
    [reqMock andReturn:request];
    __block WKNavigationActionPolicy policy = WKNavigationActionPolicyAllow;
    void (^decisionBlock)(WKNavigationActionPolicy) = ^(WKNavigationActionPolicy dPolicy){
        policy = dPolicy;
    };
    
    UIApplication *application = [UIApplication sharedApplication];
    id mockApplication = OCMClassMock([UIApplication class]);
    OCMStub([mockApplication sharedApplication]).andReturn(mockApplication);
    
    if (beforeBlock != nil) {
        beforeBlock(mockApplication);
    }
    
    [viewController webView:currentWebView decidePolicyForNavigationAction:action decisionHandler:decisionBlock];
    
    if (afterBlock != nil) {
        afterBlock(mockApplication, policy);
    }
    
    // Restore UIApplication shared instance
    id mockApp = OCMClassMock([UIApplication class]);
    OCMStub([mockApp sharedApplication]).andReturn(application);
}

- (void) test_app_store_url
{
    NSString *urlAppStore = [NSString stringWithFormat:@"%@://itunes.apple.com/us/app/id", LPMT_APP_STORE_SCHEMA];
    NSURL *url = [NSURL URLWithString:urlAppStore];
    WKNavigationActionPolicy expectedPolicy = WKNavigationActionPolicyCancel;
    [self execute_decidePolicyForNavigationAction:url withInitialPolicy: !expectedPolicy withBeforeBlock:nil withAfterBlock:^(UIApplication *mockApplication, WKNavigationActionPolicy policy){
#if TARGET_IPHONE_SIMULATOR
        OCMVerify([mockApplication canOpenURL:[OCMArg any]]);
#elif TARGET_OS_IPHONE
        OCMVerify([mockApplication canOpenURL:[OCMArg any]]);
        OCMVerify([mockApplication openURL:[OCMArg any]]);
#endif
        
        XCTAssertEqual(policy, expectedPolicy);
    }];
}

- (void) test_default_url
{
    NSURL *url = [NSURL URLWithString:LPMT_DEFAULT_URL];
    WKNavigationActionPolicy expectedPolicy = WKNavigationActionPolicyAllow;
    [self execute_decidePolicyForNavigationAction:url withInitialPolicy: !expectedPolicy withBeforeBlock:^(UIApplication *mockApplication){
        OCMReject([mockApplication openURL:[OCMArg any]]);
    } withAfterBlock:^(UIApplication *mockApplication, WKNavigationActionPolicy policy){
        XCTAssertEqual(policy, WKNavigationActionPolicyAllow);
    }];
}
@end
