
#ifndef QKITC_COMPILE_H
#define QKITC_COMPILE_H 1

#include <stdint.h>
#include <windows.h>

#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS) ||defined(_WINDOWS)

#ifdef QKITC_EXPORTS
#define QKITCAPI __declspec(dllexport)
#else
#define QKITCAPI __declspec(dllimport)
#endif

#else
#define QKITCAPI
#endif


#endif /**QKITC_COMPILE_H*/
