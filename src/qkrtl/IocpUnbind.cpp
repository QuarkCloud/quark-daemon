
#include "qkrtl/Logger.h"
#include "IocpPoller.h"

namespace qkrtl {


typedef enum class _FILE_INFORMATION_CLASS {
    FileCompletionInformation = 30,
    FileReplaceCompletionInformation = 61
} FILE_INFORMATION_CLASS;

typedef LONG NTSTATUS;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK;

typedef struct _FILE_COMPLETION_INFORMATION {
    HANDLE Port;
    PVOID  Key;
} FILE_COMPLETION_INFORMATION;


typedef NTSTATUS(WINAPI* LPFN_NtSetInformationFile)(
    HANDLE FileHandle,
    IO_STATUS_BLOCK* IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
    );
static std::mutex __NtSetInformationFileInitedGuard__;
static LPFN_NtSetInformationFile lpfn_NtSetInformationFile = NULL;

bool NtSetInformationFileInited()
{
    if (lpfn_NtSetInformationFile == NULL)
    {
        std::unique_lock<std::mutex> locker(__NtSetInformationFileInitedGuard__);
        HMODULE handle = ::GetModuleHandle("ntdll.dll");
        if (handle != NULL && handle != INVALID_HANDLE_VALUE)
        {
            lpfn_NtSetInformationFile = (LPFN_NtSetInformationFile)::GetProcAddress
            (handle, "NtSetInformationFile");
        }
        else
        {
            LOGERR("failed to init NetSetInformationFile , errCode[%d]", (int)GetLastError());
        }
    }

    return (lpfn_NtSetInformationFile != NULL);
}

bool UnbindIoCompletionPort(HANDLE fileHandle)
{
    if (NtSetInformationFileInited() == false)
    {
        LOGERR("failed to init NtSetInformationFileInited");
        return false;
    }

    IO_STATUS_BLOCK IoStatusBlock = { 0 , NULL };
    FILE_COMPLETION_INFORMATION FileInformation = { NULL , NULL };
    FILE_INFORMATION_CLASS FileInformationClass =
        FILE_INFORMATION_CLASS::FileReplaceCompletionInformation;
    ULONG Length = sizeof(FileInformation);

    NTSTATUS status = (lpfn_NtSetInformationFile(fileHandle,
        &IoStatusBlock, &FileInformation, Length, FileInformationClass));

    if (status >= 0)
        return true;
    LOGERR("NtSetInformationFile failed , Handle[%p] errCode[%x]",
        fileHandle, status);
    return false;
}
}

