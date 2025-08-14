/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       internal.h
 *
 ***************************************************************************/

#ifndef INTERNAL_H_INCLUDED
#define INTERNAL_H_INCLUDED

#include <windows.h>

typedef BOOL (*SETFROMFILEPROC)(LPCWSTR, DWORD);

BOOL
_WuGetWinUtilzCacheFileName(
    IN  LPCWSTR szFileName,
    OUT LPWSTR  szFilePath,
    IN  ULONG   cchFilePath
    );

BOOL
_WuSetSomethingFromResource(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSetFromFile,
    IN HINSTANCE        hInstance,
    IN LPVOID           szResourceName,
    IN LPVOID           szResourceType,
    IN DWORD            dwExtraValue
    );

BOOL
_WuSetSomethingFromUrl(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSetFromFile,
    IN LPVOID           szUrl,
    IN DWORD            dwExtraValue
    );

BOOL
_WuSafeExpandEnvironmentStrings(
    IN  LPCWSTR szSource,
    OUT LPWSTR  szBuffer,
    IN  DWORD   cchSize
    );

BOOL
_WuCurrentThemeGetStringProperty(
    IN  LPCWSTR szSection,
    IN  LPCWSTR szKey,
    OUT LPWSTR  szValue,
    IN  ULONG   cchValueSize
    );

#endif /* INTERNAL_H_INCLUDED */
