
#ifndef QKINTH_COMPILE_H
#define QKINTH_COMPILE_H 1

#include <stdint.h>
#include <windows.h>

#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS) ||defined(_WINDOWS)

#ifdef QKINTH_EXPORTS
#define QKINTHAPI __declspec(dllexport)
#else
#define QKINTHAPI __declspec(dllimport)
#endif

#else
#define QKINTHAPI
#endif


#endif /**QKINTH_COMPILE_H*/
