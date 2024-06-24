
#ifndef QKRTL_COMPILE_H
#define QKRTL_COMPILE_H 1

#include <stdint.h>
#include <windows.h>

#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS) ||defined(_WINDOWS)

#ifdef QKRTL_EXPORTS
#define QKRTLAPI __declspec(dllexport)
#else
#define QKRTLAPI __declspec(dllimport)
#endif

#else
#define QKRTLAPI
#endif


#endif /**QKRTL_COMPILE_H*/
