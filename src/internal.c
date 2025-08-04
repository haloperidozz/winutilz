/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       internal.c
 *
 ***************************************************************************/

#include "internal.h"

#include "winutilz.h"
#include "strconv.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

#define CACHE_DIRECTORY_NAME    L"WinUtilzCache"

BOOL
GetWinUtilzCacheFileName(
    IN  LPCWSTR szFileName,
    OUT LPWSTR  szFilePath,
    IN  ULONG   cchFilePath
    )
{
    WCHAR   szTempFilePath[MAX_PATH];
    HRESULT hResult = S_OK;

    if (szFileName == NULL || szFilePath == NULL || cchFilePath == 0)
    {
        return FALSE;
    }

    /* C:\Users\%USERNAME%\AppData\Local */
    hResult = SHGetFolderPathW(
        NULL,
        CSIDL_LOCAL_APPDATA,
        NULL,
        0,
        szTempFilePath);

    if (FAILED(hResult))
    {
        return FALSE;
    }

    /* C:\Users\%USERNAME%\AppData\Local\WinUtilzCache */
    if (PathAppendW(szTempFilePath, CACHE_DIRECTORY_NAME) == FALSE)
    {
        return FALSE;
    }

    if (PathFileExistsW(szTempFilePath) == FALSE)
    {
        if (CreateDirectoryW(szTempFilePath, NULL) == FALSE)
        {
            return FALSE;
        }
    }

    /* C:\Users\%USERNAME%\AppData\Local\WinUtilzCache\$szFileName */
    if (PathAppendW(szTempFilePath, szFileName) == FALSE)
    {
        return FALSE;
    }

    if (FAILED(StringCchCopyW(szFilePath, cchFilePath, szTempFilePath)))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
SetSomethingFromResource(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSet,
    IN HINSTANCE        hInstance,
    IN LPVOID           szResourceName,
    IN LPVOID           szResourceType,
    IN DWORD            dwExtraValue
    )
{
    WCHAR  szwTempFile[MAX_PATH];
    LPWSTR szwName = NULL;
    LPWSTR szwType = NULL;
    BOOL   bResult = FALSE;

    if (szCacheFileName == NULL || pfnSet == NULL)
    {
        return FALSE;
    }

    if (szResourceName == NULL || szResourceType == NULL)
    {
        return FALSE;
    }

    if (bUnicode == TRUE)
    {
        szwName = (LPWSTR) szResourceName;
        szwType = (LPWSTR) szResourceType;
    }
    else
    {
        szwName = AnsiResParamToWideHeapAlloc(szResourceName);
        szwType = AnsiResParamToWideHeapAlloc(szResourceType);
    }

    bResult = GetWinUtilzCacheFileName(
        szCacheFileName,
        szwTempFile,
        MAX_PATH);

    if (bResult == FALSE)
    {
        goto cleanup;
    }

    bResult = WuExtractResourceToFileW(
        hInstance,
        szwName,
        szwType,
        szwTempFile);
    
    if (bResult == FALSE)
    {
        goto cleanup;
    }

    bResult = pfnSet(szwTempFile, dwExtraValue);

cleanup:
    if (bUnicode == FALSE)
    {
        SafeResParamHeapFree(szwName);
        SafeResParamHeapFree(szwType);
    }

    return bResult;
}

BOOL
SetSomethingFromUrl(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSet,
    IN LPVOID           szUrl,
    IN DWORD            dwExtraValue
    )
{
    WCHAR  szwTempFile[MAX_PATH];
    LPWSTR szwUrl  = NULL;
    BOOL   bResult = FALSE;

    if (szCacheFileName == NULL || pfnSet == NULL)
    {
        return FALSE;
    }

    if (szUrl == NULL)
    {
        return FALSE;
    }

    if (bUnicode == TRUE)
    {
        szwUrl = (LPWSTR) szUrl;
    }
    else
    {
        szwUrl = AnsiToWideHeapAlloc((LPCSTR) szUrl);
    }

    if (szwUrl == NULL)
    {
        goto cleanup;
    }

    bResult = GetWinUtilzCacheFileName(
        szCacheFileName,
        szwTempFile,
        MAX_PATH);

    if (bResult == FALSE)
    {
        goto cleanup;
    }

    bResult = WuDownloadFileW(szwUrl, szwTempFile);
    
    if (bResult == FALSE)
    {
        goto cleanup;
    }

    bResult = pfnSet(szwTempFile, dwExtraValue);

cleanup:
    if (szwUrl != NULL && bUnicode == TRUE)
    {
        HeapFree(GetProcessHeap(), 0, szwUrl);
    }

    return bResult;
}
