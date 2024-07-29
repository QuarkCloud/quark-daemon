
#ifndef QKRTL_OS_HANDLE_H
#define QKRTL_OS_HANDLE_H 1

#include "qkrtl/Compile.h"

#if defined(PLATFORM_WINDOWS)
typedef HANDLE OsHandle;
const OsHandle kDefaultHandleValue = INVALID_HANDLE_VALUE;
#else
typedef int OsHandle;
const OsHandle kDefaultHandleValue = -1;
#endif


#endif /**QKRTL_OS_HANDLE_H*/
