
#ifndef QKHTTP_COMPILE_H
#define QKHTTP_COMPILE_H 1

#include <stdint.h>
#include <windows.h>

#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS) ||defined(_WINDOWS)

#ifdef QKHTTP_EXPORTS
#define QKHTTPAPI __declspec(dllexport)
#else
#define QKHTTPAPI __declspec(dllimport)
#endif

#else
#define QKHTTPAPI
#endif


#endif /**QKHTTP_COMPILE_H*/
