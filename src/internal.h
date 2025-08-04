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
GetWinUtilzCacheFileName(
    IN  LPCWSTR szFileName,
    OUT LPWSTR  szFilePath,
    IN  ULONG   cchFilePath
    );

BOOL
SetSomethingFromResource(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSet,
    IN HINSTANCE        hInstance,
    IN LPVOID           szResourceName,
    IN LPVOID           szResourceType,
    IN DWORD            dwExtraValue
    );

BOOL
SetSomethingFromUrl(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSet,
    IN LPVOID           szUrl,
    IN DWORD            dwExtraValue
    );

#endif /* INTERNAL_H_INCLUDED */
