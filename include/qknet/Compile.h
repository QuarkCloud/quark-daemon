
#ifndef QKNET_COMPILE_H
#define QKNET_COMPILE_H 1

#include <stdint.h>

#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS) ||defined(_WINDOWS)
#include <windows.h>

#ifdef QKNET_EXPORTS
#define QKNETAPI __declspec(dllexport)
#else
#define QKNETAPI __declspec(dllimport)
#endif

#else
#define QKNETAPI
#endif


#endif /**QKNET_COMPILE_H*/
