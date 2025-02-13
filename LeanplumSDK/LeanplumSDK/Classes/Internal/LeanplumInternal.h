//
//  LeanplumInternal.h
//  Leanplum
//
//  Created by Andrew First on 4/30/15.
//  Copyright (c) 2015 Leanplum, Inc. All rights reserved.
//
//  Licensed to the Apache Software Foundation (ASF) under one
//  or more contributor license agreements.  See the NOTICE file
//  distributed with this work for additional information
//  regarding copyright ownership.  The ASF licenses this file
//  to you under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing,
//  software distributed under the License is distributed on an
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//  KIND, either express or implied.  See the License for the
//  specific language governing permissions and limitations
//  under the License.

#import "Leanplum.h"
#import "LPActionContext-Internal.h"
#import "LPVar-Internal.h"
#import "LPConstants.h"
#import "LPActionManager.h"
#import "LPJSON.h"
#import "LPInternalState.h"
#import "LPCountAggregator.h"

NS_ASSUME_NONNULL_BEGIN

@class LeanplumSocket;
@class LPRegisterDevice;
@class NotificationsManager;

/**
 * Keys for the plist file name
 */
extern NSString *const kAppKeysFileName;
extern NSString *const kAppKeysFileType;

/**
 * Keys for the strings in the plist file.
 */
extern NSString *const kAppIdKey;
extern NSString *const kDevKey;
extern NSString *const kProdKey;
extern NSString *const kEnvKey;

@interface Leanplum ()

typedef void (^LeanplumStartIssuedBlock)(void);
typedef void (^LeanplumEventsChangedBlock)(void);
typedef void (^LeanplumHandledBlock)(BOOL success);

+ (NotificationsManager*)notificationsManager;

+ (void)throwError:(NSString *)reason;

+ (void)onHasStartedAndRegisteredAsDeveloper;

+ (void)pause;
+ (void)resume;

+ (BOOL)setAppUsingPlist:(NSDictionary *)appKeysDictionary forEnvironment:(NSString *)env;
+ (NSDictionary *) getDefaultAppKeysPlist;

+ (void)track:(nullable NSString *)event
    withValue:(double)value
      andArgs:(nullable NSDictionary<NSString *, id> *)args
andParameters:(nullable NSDictionary<NSString *, id> *)params
NS_SWIFT_NAME(track(event:value:args:params:));

+ (void)track:(nullable NSString *)event
    withValue:(double)value
      andInfo:(nullable NSString *)info
      andArgs:(nullable NSDictionary<NSString *, id> *)args
andParameters:(nullable NSDictionary<NSString *, id> *)params
NS_SWIFT_NAME(track(event:value:info:args:params:));

+ (void)setUserLocationAttributeWithLatitude:(double)latitude
                                   longitude:(double)longitude
                                        city:(NSString *)city
                                      region:(NSString *)region
                                     country:(NSString *)country
                                        type:(LPLocationAccuracyType)type
                             responseHandler:(nullable LeanplumSetLocationBlock)response;

+ (LPActionContext *)createActionContextForMessageId:(NSString *)messageId;
+ (void)triggerAction:(LPActionContext *)context;
+ (void)triggerAction:(LPActionContext *)context
         handledBlock:(nullable LeanplumHandledBlock)handledBlock;
+ (void)maybePerformActions:(NSArray<NSString *> *)whenConditions
              withEventName:(nullable NSString *)eventName
                 withFilter:(LeanplumActionFilter)filter
              fromMessageId:(nullable NSString *)sourceMessage
       withContextualValues:(nullable LPContextualValues *)contextualValues;

+ (NSInvocation *)createInvocationWithResponder:(id)responder selector:(SEL)selector;
+ (void)addInvocation:(NSInvocation *)invocation toSet:(NSMutableSet *)responders;
+ (void)removeResponder:(id)responder withSelector:(SEL)selector fromSet:(NSMutableSet *)responders;

+ (void)onStartIssued:(LeanplumStartIssuedBlock)block;
+ (void)synchronizeDefaults;

@end

#pragma mark - LPInbox class

@interface LPInbox () {
@private
    BOOL _didLoad;
}

typedef void (^LeanplumInboxCacheUpdateBlock)(void);

#pragma mark - LPInbox properties

@property(assign, nonatomic) NSUInteger unreadCount;
@property(strong, nonatomic) NSMutableDictionary *messages;
@property(strong, nonatomic) NSMutableArray *inboxChangedBlocks;
@property(strong, nonatomic) NSMutableSet *inboxChangedResponders;
@property(strong, nonatomic) NSMutableArray *inboxSyncedBlocks;
@property(strong, nonatomic) NSMutableSet *downloadedImageUrls;
@property(strong, nonatomic) LPCountAggregator *countAggregator;

#pragma mark - LPInbox method declaration

+ (LPInbox *)sharedState;

- (void)downloadMessages;
- (void)load;
- (void)save;
- (void)updateUnreadCount:(NSUInteger)unreadCount;
- (void)updateMessages:(NSMutableDictionary *)messages unreadCount:(NSUInteger)unreadCount;
- (void)removeMessageForId:(NSString *)messageId;
- (void)reset;
- (void)triggerInboxChanged;
- (void)triggerInboxSyncedWithStatus:(BOOL)success withCompletionHandler:(nullable LeanplumInboxSyncedBlock)completionHandler;

@end

#pragma mark - LPInboxMessage class

@interface LPInboxMessage ()

#pragma mark - LPInboxMessage properties

@property(strong, nonatomic) NSString *messageId;
@property(strong, nonatomic) NSDate *deliveryTimestamp;
@property(strong, nonatomic) NSDate *expirationTimestamp;
@property(assign, nonatomic) BOOL isRead;
@property(strong, nonatomic) LPActionContext *context;

@end

NS_ASSUME_NONNULL_END
