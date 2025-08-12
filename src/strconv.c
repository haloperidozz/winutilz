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

#include "winutilz.h"

WUAPI BOOL
WuAnsiToWide(
    IN  LPCSTR  szAnsi,
    OUT LPWSTR  szWideBuffer,
    IN  ULONG   cchWideBufferMaxSize
    )
{
    DWORD dwWritten = 0;

    if ((NULL == szAnsi) || (NULL == szWideBuffer))
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

    return (dwWritten > 0);
}

WUAPI LPWSTR
WuAnsiToWideHeapAlloc(
    IN LPCSTR   szAnsi
    )
{
    LPWSTR szwUnicode = NULL;
    ULONG  cchUnicode = 0;      /* szUnicode length including '\0' */
    BOOL   bResult    = FALSE;

    if (NULL == szAnsi)
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
    
    if (NULL == szwUnicode)
    {
        return NULL;
    }

    bResult = WuAnsiToWide(szAnsi, szwUnicode, cchUnicode);

    if (FALSE == bResult)
    {
        HeapFree(GetProcessHeap(), 0, szwUnicode);
        return NULL;
    }
    
    return szwUnicode;
}

WUAPI BOOL
WuWideToAnsi(
    IN  LPCWSTR szWide,
    OUT LPSTR   szAnsiBuffer,
    IN  ULONG   cchAnsiBufferMaxSize
    )
{
    DWORD dwWritten = 0;

    if ((NULL == szWide) || (NULL == szAnsiBuffer))
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

    return (dwWritten > 0);
}

WUAPI LPSTR
WuWideToAnsiHeapAlloc(
    IN LPCWSTR  szWide
    )
{
    LPSTR szAnsi  = NULL;
    ULONG cchAnsi = 0;          /* szAnsi length including '\0' */
    BOOL  bResult = FALSE;

    if (NULL == szWide)
    {
        return NULL;
    }

    cchAnsi = WideCharToMultiByte(CP_ACP, 0, szWide, -1, NULL, 0, NULL, NULL);
    
    if (cchAnsi <= 0)
    {
        return NULL;
    }

    szAnsi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cchAnsi);
    
    if (NULL == szAnsi)
    {
        return NULL;
    }

    bResult = WuWideToAnsi(szWide, szAnsi, cchAnsi);

    if (FALSE == bResult)
    {
        HeapFree(GetProcessHeap(), 0, szAnsi);
        return NULL;
    }
    
    return szAnsi;
}
