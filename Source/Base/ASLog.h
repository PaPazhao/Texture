//
//  ASLog.h
//  Texture
//
//  Copyright (c) 2014-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the /ASDK-Licenses directory of this source tree. An additional
//  grant of patent rights can be found in the PATENTS file in the same directory.
//
//  Modifications to this file made after 4/13/2017 are: Copyright (c) 2017-present,
//  Pinterest, Inc.  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//

#import <AsyncDisplayKit/ASAvailability.h>
#import <os/log.h>
#import <os/activity.h>

#pragma once

/**
 * The activity tracing system changed a lot between iOS 9 and 10.
 * In iOS 10, the system was merged with logging and became much more powerful
 * and adopted a new API.
 *
 * The legacy API is visible, but its functionality is limited.
 * For example, activities described by os_activity_start/end are not 
 * reflected in the log whereas activities described by the newer
 * os_activity_scope are. So unfortunately we must use these iOS 10
 * APIs to get meaningful logging data.
 */
#if OS_ACTIVITY_OBJECT_API
#define as_activity_scope(description, flags) os_activity_scope(os_activity_create(description, OS_ACTIVITY_CURRENT, flags))

// Log the current backtrace. Note: the backtrace will be leaked. Only call this when debugging or in case of failure.
#define as_log_backtrace(type, log) os_log_with_type(log, type, "backtrace: %p", CFBridgingRetain(NSThread.callStackSymbols));

#else
static void as_activity_end(os_activity_t *ptr) {
  if (AS_AT_LEAST_IOS9) { os_activity_end(*ptr); }
}
#define __AS_CONCAT(x, y) x ## y
#define AS_CONCAT(x, y) __AS_CONCAT(x, y)

#define as_activity_scope(description, flags) \
  __unused os_activity_t AS_CONCAT(scope, __COUNTER__) __attribute__((cleanup(as_activity_end))) = AS_AT_LEAST_IOS9 ? os_activity_start(description, flags) : (os_activity_t)0
#define as_log_backtrace(type, log)

#endif

#define as_log_create(subsystem, category) AS_AT_LEAST_IOS9 ? os_log_create(subsystem, category) : (os_log_t)0
#define as_log_debug(log, format, ...) AS_AT_LEAST_IOS9 ? os_log_debug(log, format, ##__VA_ARGS__) : (void)0
#define as_log_info(log, format, ...) AS_AT_LEAST_IOS9 ? os_log_info(log, format, ##__VA_ARGS__) : (void)0
#define as_log_error(log, format, ...) AS_AT_LEAST_IOS9 ? os_log_error(log, format, ##__VA_ARGS__) : (void)0
#define as_log_fault(log, format, ...) AS_AT_LEAST_IOS9 ? os_log_fault(log, format, ##__VA_ARGS__) : (void)0

static os_log_t ASLayoutLog;
static os_log_t ASRenderLog;
static os_log_t ASCollectionLog() {
  static os_log_t log;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    log = os_log_create("org.AsyncDisplayKit.AsyncDisplayKit", "Collections");
  });
  return log;
}

#define ASMultiplexImageNodeLogDebug(...)
#define ASMultiplexImageNodeCLogDebug(...)

#define ASMultiplexImageNodeLogError(...)
#define ASMultiplexImageNodeCLogError(...)

// Note: `<sys/kdebug_signpost.h>` only exists in Xcode 8 and later.
#if defined(PROFILE) && __has_include(<sys/kdebug_signpost.h>)

#import <sys/kdebug_signpost.h>

// These definitions are required to build the backward-compatible kdebug trace
// on the iOS 10 SDK.  The kdebug_trace function crashes if run on iOS 9 and earlier.
// It's valuable to support trace signposts on iOS 9, because A5 devices don't support iOS 10.
#ifndef DBG_MACH_CHUD
#define DBG_MACH_CHUD 0x0A
#define DBG_FUNC_NONE 0
#define DBG_FUNC_START 1
#define DBG_FUNC_END 2
#define DBG_APPS 33
#define SYS_kdebug_trace 180
#define KDBG_CODE(Class, SubClass, code) (((Class & 0xff) << 24) | ((SubClass & 0xff) << 16) | ((code & 0x3fff)  << 2))
#define APPSDBG_CODE(SubClass,code) KDBG_CODE(DBG_APPS, SubClass, code)
#endif

#define ASProfilingSignpost(x) \
  AS_AT_LEAST_IOS10 ? kdebug_signpost(x, 0, 0, 0, (uint32_t)(x % 4)) \
                    : syscall(SYS_kdebug_trace, APPSDBG_CODE(DBG_MACH_CHUD, x) | DBG_FUNC_NONE, 0, 0, 0, (uint32_t)(x % 4));

#define ASProfilingSignpostStart(x, y) \
  AS_AT_LEAST_IOS10 ? kdebug_signpost_start((uint32_t)x, (uintptr_t)y, 0, 0, (uint32_t)(x % 4)) \
                    : syscall(SYS_kdebug_trace, APPSDBG_CODE(DBG_MACH_CHUD, x) | DBG_FUNC_START, (uintptr_t)y, 0, 0, (uint32_t)(x % 4));

#define ASProfilingSignpostEnd(x, y) \
  AS_AT_LEAST_IOS10 ? kdebug_signpost_end((uint32_t)x, (uintptr_t)y, 0, 0, (uint32_t)(x % 4)) \
                    : syscall(SYS_kdebug_trace, APPSDBG_CODE(DBG_MACH_CHUD, x) | DBG_FUNC_END, (uintptr_t)y, 0, 0, (uint32_t)(x % 4));
#else

#define ASProfilingSignpost(x)
#define ASProfilingSignpostStart(x, y)
#define ASProfilingSignpostEnd(x, y)

#endif
