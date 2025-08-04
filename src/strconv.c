/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       strconv.c
 *
 ***************************************************************************/

#include "strconv.h"

BOOL
AnsiToWide(
    IN  LPCSTR  szAnsi,
    OUT LPWSTR  szWideBuffer,
    IN  ULONG   cchWideBufferMaxSize
    )
{
    DWORD dwWritten = 0;

    if (szAnsi == NULL || szWideBuffer == NULL)
    {
        return FALSE;
    }

    dwWritten = (DWORD) MultiByteToWideChar(
        CP_ACP,
        0,
        szAnsi,
        -1,
        szWideBuffer,
        (INT) cchWideBufferMaxSize);

    return dwWritten > 0;
}

LPWSTR
AnsiToWideHeapAlloc(
    IN LPCSTR   szAnsi
    )
{
    LPWSTR szwUnicode = NULL;
    ULONG  cchUnicode = 0;      /* szUnicode length including '\0' */
    BOOL   bResult    = FALSE;

    if (szAnsi == NULL)
    {
        return NULL;
    }

    cchUnicode = MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, NULL, 0);
    
    if (cchUnicode <= 0)
    {
        return NULL;
    }

    szwUnicode = (LPWSTR) HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        cchUnicode * sizeof(WCHAR));
    
    if (szwUnicode == NULL)
    {
        return NULL;
    }

    bResult = AnsiToWide(szAnsi, szwUnicode, cchUnicode);

    if (bResult == FALSE)
    {
        HeapFree(GetProcessHeap(), 0, szwUnicode);
        return NULL;
    }
    
    return szwUnicode;
}

BOOL
WideToAnsi(
    IN  LPCWSTR szWide,
    OUT LPSTR   szAnsiBuffer,
    IN  ULONG   cchAnsiBufferMaxSize
    )
{
    DWORD dwWritten = 0;

    if (szWide == NULL || szAnsiBuffer == NULL)
    {
        return FALSE;
    }

    dwWritten = (DWORD) WideCharToMultiByte(
        CP_ACP,
        0,
        szWide,
        -1,
        szAnsiBuffer,
        cchAnsiBufferMaxSize,
        NULL,
        NULL);

    return dwWritten > 0;
}

LPSTR
WideToAnsiHeapAlloc(
    IN LPCWSTR  szWide
    )
{
    LPSTR szAnsi  = NULL;
    ULONG cchAnsi = 0;          /* szAnsi length including '\0' */
    BOOL  bResult = FALSE;

    if (szWide == NULL)
    {
        return NULL;
    }

    cchAnsi = WideCharToMultiByte(CP_ACP, 0, szWide, -1, NULL, 0, NULL, NULL);
    
    if (cchAnsi <= 0)
    {
        return NULL;
    }

    szAnsi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cchAnsi);
    
    if (szAnsi == NULL)
    {
        return NULL;
    }

    bResult = WideToAnsi(szWide, szAnsi, cchAnsi);

    if (bResult == FALSE)
    {
        HeapFree(GetProcessHeap(), 0, szAnsi);
        return NULL;
    }
    
    return szAnsi;
}

LPWSTR
AnsiResParamToWideHeapAlloc(
    IN LPCSTR   szAnsi
    )
{
    if (IS_INTRESOURCE(szAnsi) || szAnsi == NULL)
    {
        return (LPWSTR) szAnsi;
    }
    else
    {
        return AnsiToWideHeapAlloc(szAnsi);
    }
}

VOID
SafeResParamHeapFree(
    IN LPVOID   lpParam
    )
{
    if (lpParam != NULL && IS_INTRESOURCE(lpParam) == TRUE)
    {
        HeapFree(GetProcessHeap(), 0, lpParam);
    }
}

