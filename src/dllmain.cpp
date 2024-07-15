#define UNICODE 1
#include <windows.h>
#include "version.h"
#include "detours.h"
#include <string>

BOOL(WINAPI* Real_GetVolumePathName) (
    LPCWSTR lpszFileName,
    LPWSTR  lpszVolumePathName,
    DWORD   cchBufferLength) = GetVolumePathNameW;

BOOL WINAPI Routed_GetVolumePathName(LPCWSTR lpszFileName,
    LPWSTR  lpszVolumePathName,
    DWORD   cchBufferLength)
{
    if (std::wstring(lpszFileName).starts_with(L"Z:\\")) {
        //lpszVolumePathName = (LPWSTR)L"Z:\\";
        //return TRUE;
        return Real_GetVolumePathName(L"C:\\", lpszVolumePathName, cchBufferLength);
    }
    else {
        return Real_GetVolumePathName(lpszFileName, lpszVolumePathName, cchBufferLength);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Load, hModule, NULL, NULL);
        LONG Error;
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)Real_GetVolumePathName, Routed_GetVolumePathName);
        Error = DetourTransactionCommit();

        if (Error == NO_ERROR)
            OutputDebugString(TEXT("Hooked Success"));
        else
            OutputDebugString(TEXT("Hook Error"));

        break;
    case DLL_PROCESS_DETACH:
        FreeLibrary(version_dll); 
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)Real_GetVolumePathName, Routed_GetVolumePathName);
        Error = DetourTransactionCommit();

        if (Error == NO_ERROR)
            OutputDebugString(TEXT("Un-Hooked Success"));
        else
            OutputDebugString(TEXT("Un-Hook Error"));
        break;
    }
    return TRUE;
}
