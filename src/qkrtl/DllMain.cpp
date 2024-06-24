
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS) ||defined(_WINDOWS)

#include <windows.h>

#if defined(_DLL) || defined(_USRDLL)


BOOL APIENTRY DllMain(HMODULE hm, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:break;
    case DLL_THREAD_ATTACH:break;
    case DLL_THREAD_DETACH:break;
    case DLL_PROCESS_DETACH:break;
    default:break;
    }
    return TRUE;
}
#endif

#endif