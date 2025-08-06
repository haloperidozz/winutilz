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

WUAPI BOOL
WuResourceExists(
    IN HINSTANCE    hInstance,
    IN LPCTSTR      szResourceName,
    IN LPCTSTR      szResourceType
    )
{
    return FindResource(hInstance, szResourceName, szResourceType) != NULL;
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

    if (szResourceName == NULL || szResourceType == NULL)
    {
        return NULL;
    }

    if (pcbSize == NULL)
    {
        return NULL;
    }

    hResource = FindResourceW(hInstance, szResourceName, szResourceType);

    if (hResource == NULL)
    {
        return NULL;
    }

    *pcbSize = SizeofResource(hInstance, hResource);

    if ((*pcbSize) == 0)
    {
        return NULL;
    }

    hResourceData = LoadResource(hInstance, hResource);

    if (hResourceData == NULL)
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
    DWORD  dwWritten      = 0;

    if (szResourceName == NULL || szResourceType == NULL)
    {
        return FALSE;
    }

    if (szFilePath == NULL)
    {
        return FALSE;
    }

    pResourceData = WuLoadResourceToMemoryW(
        hInstance,
        szResourceName,
        szResourceType,
        &cbResourceData);
    
    if (pResourceData == NULL)
    {
        goto cleanup;
    }

    if (ExpandEnvironmentStringsW(
        szFilePath,
        szTempPath,
        MAX_PATH) == 0)
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
    
    if (hResourceFile == INVALID_HANDLE_VALUE)
    {
        goto cleanup;
    }

    WriteFile(
        hResourceFile,
        pResourceData,
        cbResourceData,
        &dwWritten,
        NULL);

cleanup:
    if (hResourceFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hResourceFile);
    }

    return dwWritten == cbResourceData;
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
