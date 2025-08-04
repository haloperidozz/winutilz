/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       process.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include "undoc.h"
#include "strconv.h"

#define PROCESS_PRIVILEGE_MAX   37

WUAPI BOOL
WuCurrentProcessSetPrivilege(
    IN WU_PROCESS_PRIVILEGE privilege,
    IN BOOL                 bEnable
    )
{
    BOOLEAN  bEnabled = FALSE;
    NTSTATUS status   = STATUS_SUCCESS;

    if (privilege >= PROCESS_PRIVILEGE_MAX)
    {
        return FALSE;
    }

    status = RtlAdjustPrivilege(
        privilege,
        bEnable,
        FALSE,
        &bEnabled);

    return NT_SUCCESS(status);
}

typedef BOOL (*PROCACTION)(HANDLE hProcessHandle);

static BOOL
ActOnProcessById(
    IN DWORD        dwProcessId,
    IN DWORD        dwDesiredAccess,
    IN PROCACTION   pfnAction
    )
{
    HANDLE hProcess = NULL;
    BOOL   bResult  = FALSE;
    
    hProcess = OpenProcess(dwDesiredAccess, FALSE, dwProcessId);

    if (hProcess == NULL)
    {
        return FALSE;
    }

    bResult = pfnAction(hProcess);

    CloseHandle(hProcess);

    return bResult;
}

WUAPI BOOL
WuSuspendProcess(
    IN HANDLE   hProcessHandle
    )
{
    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_DEBUG, TRUE);

    return NT_SUCCESS(NtSuspendProcess(hProcessHandle));
}

WUAPI BOOL
WuSuspendProcessByPid(
    IN DWORD    dwProcessId
    )
{
    return ActOnProcessById(
        dwProcessId,
        PROCESS_SUSPEND_RESUME,
        WuSuspendProcess);
}

WUAPI BOOL
WuResumeProcess(
    IN HANDLE   hProcessHandle
    )
{
    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_DEBUG, TRUE);

    return NT_SUCCESS(NtResumeProcess(hProcessHandle));
}

WUAPI BOOL
WuResumeProcessByPid(
    IN DWORD    dwProcessId
    )
{
    return ActOnProcessById(
        dwProcessId,
        PROCESS_SUSPEND_RESUME,
        WuResumeProcess);
}

WUAPI BOOL
WuKillProcess(
    IN HANDLE   hProcessHandle
    )
{
    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_DEBUG, TRUE);

    return TerminateProcess(hProcessHandle, ERROR_PROCESS_ABORTED); 
}

WUAPI BOOL
WuKillProcessByPid(
    IN DWORD    dwProcessId
    )
{
    return ActOnProcessById(
        dwProcessId,
        PROCESS_TERMINATE,
        WuKillProcess);
}

WUAPI BOOL
WuProcessInjectFunction(
    IN HANDLE       hProcess,
    IN INJFUNCPROC  fnFunction,
    IN SIZE_T       cbFunction,
    IN CONST LPVOID lpUserData,
    IN SIZE_T       cbUserData
    )
{
    LPVOID lpRemoteFunction = NULL;
    LPVOID lpRemoteParam    = NULL;
    SIZE_T nWritten         = 0;
    DWORD  dwExitCode       = 0;
    HANDLE hRemoteThread    = INVALID_HANDLE_VALUE;
    BOOL   bResult          = FALSE;

    if (hProcess == NULL || fnFunction == NULL || cbFunction == 0)
    {
        return FALSE;
    }

    lpRemoteFunction = VirtualAllocEx(
        hProcess,
        NULL,
        cbFunction,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);

    if (lpRemoteFunction == NULL)
    {
        return FALSE;
    }

    bResult = WriteProcessMemory(
        hProcess,
        lpRemoteFunction,
        fnFunction,
        cbFunction,
        &nWritten);
    
    if (bResult == FALSE || nWritten < cbFunction)
    {
        bResult = FALSE;
        goto cleanup;
    }

    if (lpUserData == NULL || cbUserData == 0)
    {
        goto no_data;
    }

    lpRemoteParam = VirtualAllocEx(
        hProcess,
        NULL,
        cbUserData,
        MEM_COMMIT,
        PAGE_READWRITE);
    
    if (lpRemoteParam == NULL)
    {
        goto cleanup;
    }

    bResult = WriteProcessMemory(
        hProcess,
        lpRemoteParam,
        lpUserData,
        cbUserData,
        &nWritten);
    
    if (bResult == FALSE || nWritten < cbFunction)
    {
        bResult = FALSE;
        goto cleanup;
    }

no_data:
    hRemoteThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        lpRemoteFunction,
        lpRemoteParam,
        0,
        NULL);
    
    if (hRemoteThread == NULL || hRemoteThread == INVALID_HANDLE_VALUE)
    {
        bResult = FALSE;
        goto cleanup;
    }

    WaitForSingleObject(hRemoteThread, INFINITE);

    if (GetExitCodeProcess(hRemoteThread, &dwExitCode) == FALSE)
    {
        bResult = FALSE;
    }
    else
    {
        bResult = (dwExitCode == 0);
    }

cleanup:
    if (lpRemoteParam != NULL)
    {
        VirtualFreeEx(hProcess, lpRemoteParam, 0, MEM_RELEASE);
    }

    if (lpRemoteFunction != NULL)
    {
        VirtualFreeEx(hProcess, lpRemoteFunction, 0, MEM_RELEASE);
    }

    if (hRemoteThread != NULL && hRemoteThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hRemoteThread);
    }

    return bResult;
}

WUAPI BOOL
WuProcessInjectFunctionByPid(
    IN DWORD        dwProcessId,
    IN INJFUNCPROC  fnFunction,
    IN SIZE_T       cbFunction,
    IN CONST LPVOID lpUserData,
    IN SIZE_T       cbUserData
    )
{
    HANDLE hProcess = NULL;
    BOOL   bResult  = FALSE;
    
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

    if (hProcess == NULL)
    {
        return FALSE;
    }

    bResult = WuProcessInjectFunction(
        hProcess,
        fnFunction,
        cbFunction,
        lpUserData,
        cbUserData);

    CloseHandle(hProcess);

    return bResult;
}

WUAPI BOOL
WuRunCommandW(
    IN  LPCWSTR szCommand,
    IN  BOOL    bSilent,
    OUT LPDWORD lpdwExitCode
    )
{
    STARTUPINFOW        si;
    PROCESS_INFORMATION pi;
    SIZE_T              cchCommand    = 0;
    LPWSTR              szCommandCopy = NULL;
    BOOL                bResult       = FALSE;

    if (szCommand == NULL)
    {
        return FALSE;
    }

    cchCommand = lstrlenW(szCommand) + 1;

    szCommandCopy = (LPWSTR) HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        cchCommand * sizeof(WCHAR));

    if (szCommandCopy == NULL)
    {
        goto cleanup;
    }

    wcsncpy(szCommandCopy, szCommand, cchCommand - 1);
    
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(STARTUPINFOW);
    
    if (bSilent == TRUE)
    {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    bResult = CreateProcessW(
        NULL,                               /* lpApplicationName    */
        szCommandCopy,                      /* lpCommandLine        */
        NULL,                               /* lpProcessAttributes  */
        NULL,                               /* lpThreadAttributes   */
        FALSE,                              /* bInheritHandles      */
        bSilent ? CREATE_NO_WINDOW : 0,     /* dwCreationFlags      */
        NULL,                               /* lpEnvironment        */
        NULL,                               /* lpCurrentDirectory   */
        &si,                                /* lpStartupInfo        */
        &pi);                               /* lpProcessInformation */

    if (bResult == FALSE)
    {
        goto cleanup;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    if (lpdwExitCode == NULL)
    {
        goto cleanup;
    }

    if (GetExitCodeProcess(pi.hProcess, lpdwExitCode) == FALSE)
    {
        bResult = FALSE;
    }

cleanup:
    if (pi.hProcess != NULL && pi.hProcess != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pi.hProcess);
    }

    if (pi.hThread != NULL && pi.hThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pi.hThread);
    }

    if (szCommandCopy != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szCommandCopy);
    }

    return bResult;
}

WUAPI BOOL
WuRunCommandA(
    IN  LPCSTR  szCommand,
    IN  BOOL    bSilent,
    OUT LPDWORD lpdwExitCode
    )
{
    LPWSTR szwCommand = NULL;
    BOOL   bResult    = FALSE;

    szwCommand = AnsiToWideHeapAlloc(szCommand);

    if (szwCommand == NULL)
    {
        return FALSE;
    }

    bResult = WuRunCommandW(szwCommand, bSilent, lpdwExitCode);

    HeapFree(GetProcessHeap(), 0, szwCommand);

    return bResult;
}

static BOOL CALLBACK
CurrentProcessGetWindow_EnumWindowsProc(
    IN HWND     hWnd,
    IN LPARAM   lParam
    )
{
    DWORD dwCurrentProcessId = GetCurrentProcessId();
    DWORD  dwWindowProcessId = 0;

    if (GetWindowThreadProcessId(hWnd, &dwWindowProcessId) == 0)
    {
        return TRUE;
    }

    if (dwWindowProcessId != dwCurrentProcessId)
    {
        return TRUE;
    }

    if (GetWindow(hWnd, GW_OWNER) != NULL)
    {
        return TRUE;
    }

    if (GetParent(hWnd) != NULL)
    {
        return TRUE;
    }

    *((HWND*) lParam) = hWnd;

    return FALSE;
}

WUAPI HWND
WuCurrentProcessGetWindow(
    VOID
    )
{
    HWND hWnd = NULL;

    EnumWindows(CurrentProcessGetWindow_EnumWindowsProc, (LPARAM) &hWnd);
    
    return hWnd;
}
