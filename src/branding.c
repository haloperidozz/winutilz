/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       branding.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include "undoc.h"

WUAPI BOOL
WuBrandingFormatStringW(
    IN  LPCWSTR szFormat,
    OUT LPWSTR  szBuffer,
    IN  ULONG   cchBufferSize
    )
{
    LPWSTR szFormattedString = NULL;

    if ((NULL == szFormat) || (NULL == szBuffer) || (0 == cchBufferSize))
    {
        return FALSE;
    }
    
    szFormattedString = BrandingFormatString(szFormat);

    if (szFormattedString == NULL)
    {
        return FALSE;
    }

    if (FAILED(StringCchCopyW(szBuffer, cchBufferSize, szFormattedString)))
    {
        GlobalFree((HGLOBAL) szFormattedString);
        return FALSE;
    }

    return (GlobalFree((HGLOBAL) szFormattedString) == NULL);
}

WUAPI BOOL
WuBrandingFormatStringA(
    IN  LPCSTR  szFormat,
    OUT LPSTR   szBuffer,
    IN  ULONG   cchBufferSize
    )
{
    LPWSTR szwFormat = NULL;
    LPWSTR szwBuffer = NULL;
    BOOL   bResult   = FALSE;

    szwFormat = WuAnsiToWideHeapAlloc(szFormat);

    if (NULL == szwFormat)
    {
        return FALSE;
    }

    szwBuffer = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        cchBufferSize * sizeof(WCHAR));

    if (NULL == szwBuffer)
    {
        goto cleanup;
    }

    bResult = WuBrandingFormatStringW(szwFormat, szwBuffer, cchBufferSize);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    bResult = WuWideToAnsi(szwBuffer, szBuffer, cchBufferSize);

cleanup:
    if (szwBuffer != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwBuffer);
    }

    if (szwFormat != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwFormat);
    }

    return bResult;
}
