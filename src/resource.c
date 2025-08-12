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

/* TODO: */
WUAPI BOOL
WuResourceExists(
    IN HINSTANCE    hInstance,
    IN LPCTSTR      szResourceName,
    IN LPCTSTR      szResourceType
    )
{
    return (NULL != FindResource(hInstance, szResourceName, szResourceType));
}

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

    szwResourceName = AnsiResParamToWideHeapAlloc(szResourceName);
    szwResourceType = AnsiResParamToWideHeapAlloc(szResourceType);

    lpResourceData = WuLoadResourceToMemoryW(
        hInstance,
        szwResourceName,
        szwResourceType,
        pcbSize);
    
    SafeResParamHeapFree(szwResourceName);
    SafeResParamHeapFree(szwResourceType);
    
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
    DWORD  cchTempPath    = 0;
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

    SetLastError(ERROR_SUCCESS);

    cchTempPath = ExpandEnvironmentStringsW(
        szFilePath,
        szTempPath,
        MAX_PATH);

    if (0 == cchTempPath)
    {
        if (GetLastError() != ERROR_SUCCESS)
        {
            goto cleanup;
        }
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

    szwResourceName = AnsiResParamToWideHeapAlloc(szResourceName);
    szwResourceType = AnsiResParamToWideHeapAlloc(szResourceType);
    szwFilePath     = WuAnsiToWideHeapAlloc(szFilePath);

    bResult = WuExtractResourceToFileW(
        hInstance,
        szwResourceName,
        szwResourceType,
        szwFilePath);

    if (szwFilePath != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwFilePath);
    }
    
    SafeResParamHeapFree(szwResourceName);
    SafeResParamHeapFree(szwResourceType);
    
    return bResult;
}
