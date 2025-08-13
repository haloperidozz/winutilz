/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       resource.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include "internal.h"

WUAPI LPVOID
WuLoadResourceToMemoryW(
    IN  HINSTANCE   hInstance,
    IN  LPCWSTR     szResourceName,
    IN  LPCWSTR     szResourceType,
    OUT PULONG      pcbSize
    )
{
    HRSRC   hResource     = NULL;
    HGLOBAL hResourceData = NULL;

    if ((NULL == szResourceName) || (NULL == szResourceType))
    {
        return NULL;
    }

    if (NULL == pcbSize)
    {
        return NULL;
    }

    hResource = FindResourceW(hInstance, szResourceName, szResourceType);

    if (NULL == hResource)
    {
        return NULL;
    }

    *pcbSize = SizeofResource(hInstance, hResource);

    if (0 == (*pcbSize))
    {
        return NULL;
    }

    hResourceData = LoadResource(hInstance, hResource);

    if (NULL == hResourceData)
    {
        return NULL;
    }

    return LockResource(hResourceData);
}

WUAPI LPVOID
WuLoadResourceToMemoryA(
    IN  HINSTANCE   hInstance,
    IN  LPCSTR      szResourceName,
    IN  LPCSTR      szResourceType,
    OUT PULONG      pcbSize
    )
{
    LPWSTR szwResourceName = NULL;
    LPWSTR szwResourceType = NULL;
    LPVOID lpResourceData  = NULL;

    if (IS_INTRESOURCE(szResourceName))
    {
        szwResourceName = (LPWSTR) szResourceName;
    }
    else
    {
        szwResourceName = WuAnsiToWideHeapAlloc(szResourceName);
    }

    if (IS_INTRESOURCE(szResourceType))
    {
        szwResourceType = (LPWSTR) szResourceType;
    }
    else
    {
        szwResourceType = WuAnsiToWideHeapAlloc(szResourceType);
    }

    lpResourceData = WuLoadResourceToMemoryW(
        hInstance,
        szwResourceName,
        szwResourceType,
        pcbSize);
    
    if (IS_INTRESOURCE(szwResourceName) == FALSE && szwResourceName != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwResourceName);
    }

    if (IS_INTRESOURCE(szwResourceType) == FALSE && szwResourceType != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwResourceType);
    }
    
    return lpResourceData;
}

WUAPI BOOL
WuExtractResourceToFileW(
    IN HINSTANCE    hInstance,
    IN LPCWSTR      szResourceName,
    IN LPCWSTR      szResourceType,
    IN LPCWSTR      szFilePath
    )
{
    WCHAR  szTempPath[MAX_PATH];
    LPVOID pResourceData  = NULL;
    ULONG  cbResourceData = 0;
    HANDLE hResourceFile  = INVALID_HANDLE_VALUE;
    DWORD  dwWritten      = 0;
    BOOL   bResult        = FALSE;

    if ((NULL == szResourceName) || (NULL == szResourceType))
    {
        return FALSE;
    }

    if (NULL == szFilePath)
    {
        return FALSE;
    }

    pResourceData = WuLoadResourceToMemoryW(
        hInstance,
        szResourceName,
        szResourceType,
        &cbResourceData);
    
    if (NULL == pResourceData)
    {
        goto cleanup;
    }
    
    bResult = _WuSafeExpandEnvironmentStrings(
        szFilePath,
        szTempPath,
        MAX_PATH);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    hResourceFile = CreateFileW(
        szTempPath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    if (INVALID_HANDLE_VALUE == hResourceFile)
    {
        goto cleanup;
    }

    bResult = WriteFile(
        hResourceFile,
        pResourceData,
        cbResourceData,
        &dwWritten,
        NULL);
    
    if (bResult == FALSE)
    {
        goto cleanup;
    }

    bResult = (dwWritten == cbResourceData);

cleanup:
    if (hResourceFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hResourceFile);
    }

    return bResult;
}

WUAPI BOOL
WuExtractResourceToFileA(
    IN HINSTANCE    hInstance,
    IN LPCSTR       szResourceName,
    IN LPCSTR       szResourceType,
    IN LPCSTR       szFilePath
    )
{
    LPWSTR szwResourceName = NULL;
    LPWSTR szwResourceType = NULL;
    LPWSTR szwFilePath     = NULL;
    BOOL   bResult         = FALSE;

    if (IS_INTRESOURCE(szResourceName))
    {
        szwResourceName = (LPWSTR) szResourceName;
    }
    else
    {
        szwResourceName = WuAnsiToWideHeapAlloc(szResourceName);
    }

    if (IS_INTRESOURCE(szResourceType))
    {
        szwResourceType = (LPWSTR) szResourceType;
    }
    else
    {
        szwResourceType = WuAnsiToWideHeapAlloc(szResourceType);
    }

    szwFilePath = WuAnsiToWideHeapAlloc(szFilePath);

    bResult = WuExtractResourceToFileW(
        hInstance,
        szwResourceName,
        szwResourceType,
        szwFilePath);

    if (szwFilePath != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwFilePath);
    }
    
    if (IS_INTRESOURCE(szwResourceName) == FALSE && szwResourceName != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwResourceName);
    }

    if (IS_INTRESOURCE(szwResourceType) == FALSE && szwResourceType != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwResourceType);
    }
    
    return bResult;
}

WUAPI BOOL
WuGetStringResourceLength(
    IN  HINSTANCE   hInstance,
    IN  UINT        uId,
    OUT LPDWORD     lpdwSize
    )
{
    LPWSTR lpBlockId     = MAKEINTRESOURCEW((uId >> 4) + 1);
    LPWSTR lpResType     = MAKEINTRESOURCEW(6); /* RT_STRING */
    ULONG  ulBlockSize   = 0;
    UINT   uIndexInBlock = (uId & 0xF);
    LPWORD lpBlock       = NULL;
    UINT   i             = 0;

    if (NULL == lpdwSize)
    {
        return FALSE;
    }

    lpBlock = (LPWORD) WuLoadResourceToMemoryW(
        hInstance,
        lpBlockId,
        lpResType,
        &ulBlockSize);

    if (NULL == lpBlock)
    {
        return FALSE;
    }

/*
    https://pefile.readthedocs.io/en/latest/usage/ReadingResourceStrings.html
*/

    for (i = 0; i < uIndexInBlock; ++i)
    {
        lpBlock += (1 + *lpBlock); /* |LL|DDDDDDDD...|LL|DDDDDDDDD...| */
    }

    *lpdwSize = ((DWORD) *lpBlock);

    return TRUE;
}
