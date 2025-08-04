/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       undoc.h
 *
 ***************************************************************************/

#ifndef UNDOC_H_INCLUDED
#define UNDOC_H_INCLUDED

#include <windows.h>
#include <ntstatus.h>

/*
    HACK:
    NT_SUCCESS predefinition, since ntdef.h conflicts with windows.h
*/
#define NT_SUCCESS(status) (((NTSTATUS) (status)) >= 0)

/*
    +-------------------------------------------------------------------+
    |   NTDLL.DLL                                                       |
    +-------------------------------------------------------------------+
*/

extern NTSYSAPI NTSTATUS NTAPI
RtlAdjustPrivilege(
    IN  ULONG       uPrivilege,
    IN  BOOLEAN     bEnable,
    IN  BOOLEAN     bCurrentThread,
    OUT PBOOLEAN    pbEnabled
    );

typedef enum _HARDERROR_RESPONSE_OPTION {
    OptionAbortRetryIgnore,
    OptionOk,
    OptionOkCancel,
    OptionRetryCancel,
    OptionYesNo,
    OptionYesNoCancel,
    OptionShutdownSystem
} HARDERROR_RESPONSE_OPTION, *PHARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
    ResponseReturnToCaller,
    ResponseNotHandled,
    ResponseAbort,
    ResponseCancel,
    ResponseIgnore,
    ResponseNo,
    ResponseOk,
    ResponseRetry,
    ResponseYes
} HARDERROR_RESPONSE, *PHARDERROR_RESPONSE;

extern NTSYSAPI NTSTATUS NTAPI
NtRaiseHardError(
    IN  NTSTATUS                    errorStatus,
    IN  ULONG                       uNumberOfParameters,
    IN  ULONG                       uUnicodeStringParameterMask,
    IN  PVOID*                      ppParameters,
    IN  HARDERROR_RESPONSE_OPTION   options,
    OUT PHARDERROR_RESPONSE         pResponse
    );

extern NTSYSAPI NTSTATUS NTAPI
NtInitiatePowerAction(
    IN POWER_ACTION         systemAction,
    IN SYSTEM_POWER_STATE   lightestSystemState,
    IN ULONG                uFlags,
    IN BOOL                 bAsynchronous
    );

extern NTSYSAPI NTSTATUS NTAPI
NtPowerInformation(
    IN  POWER_INFORMATION_LEVEL infoLevel,
    IN  PVOID                   pInputBuffer,
    IN  ULONG                   uInputBufferLength,
    OUT PVOID                   pOutputBuffer,
    IN  ULONG                   uOutputBufferLength
    );

extern NTSYSAPI NTSTATUS NTAPI
NtSuspendProcess(
    IN HANDLE   hProcessHandle
    );

extern NTSYSAPI NTSTATUS NTAPI
NtResumeProcess(
    IN HANDLE   hProcessHandle
    );

/*
    +-------------------------------------------------------------------+
    |   WINBRAND.DLL                                                    |
    +-------------------------------------------------------------------+
*/

LPWSTR
BrandingFormatString(
    IN LPCWSTR  szFormat
    );

/*
    +-------------------------------------------------------------------+
    |   SHLWAPI.DLL                                                     |
    +-------------------------------------------------------------------+
*/

HWND
SHCreateWorkerWindowW(
    IN WNDPROC  fnWndProc,
    IN HWND     hWndParent,
    IN DWORD    dwExStyle,
    IN DWORD    dwStyle,
    IN HMENU    hMenu,
    IN LONG_PTR wnd_extra);

#endif /* UNDOC_H_INCLUDED */
