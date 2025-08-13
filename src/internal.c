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

#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <versionhelpers.h>

#include "winutilz.h"

#define CACHE_DIRECTORY_NAME    L"WinUtilzCache"

#define REGISTRY_PATH_THEMES    L"Software\\Microsoft\\Windows"     \
                                L"\\CurrentVersion\\Themes"

#define REGISTRY_KEY_CURRENTTHEME  L"CurrentTheme"      /* Vista+ */
#define REGISTRY_KEY_INSTALLTHEME  L"InstallTheme"

BOOL
_WuGetWinUtilzCacheFileName(
    IN  LPCWSTR szFileName,
    OUT LPWSTR  szFilePath,
    IN  ULONG   cchFilePath
    )
{
    WCHAR   szBuffer[MAX_PATH];
    HRESULT hResult = S_OK;

    if ((NULL == szFileName) || (NULL == szFilePath) || (0 == cchFilePath))
    {
        return FALSE;
    }

    /* C:\Users\%USERNAME%\AppData\Local */
    hResult = SHGetFolderPathW(
        NULL,
        CSIDL_LOCAL_APPDATA,
        NULL,
        0,
        szBuffer);

    if (FAILED(hResult))
    {
        return FALSE;
    }

    /* C:\Users\%USERNAME%\AppData\Local\WinUtilzCache */
    if (PathAppendW(szBuffer, CACHE_DIRECTORY_NAME) == FALSE)
    {
        return FALSE;
    }

    if (PathFileExistsW(szBuffer) == FALSE)
    {
        if (CreateDirectoryW(szBuffer, NULL) == FALSE)
        {
            return FALSE;
        }
    }

    /* C:\Users\%USERNAME%\AppData\Local\WinUtilzCache\$szFileName */
    if (PathAppendW(szBuffer, szFileName) == FALSE)
    {
        return FALSE;
    }

    return SUCCEEDED(StringCchCopyW(szFilePath, cchFilePath, szBuffer));
}

BOOL
_WuSetSomethingFromResource(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSetFromFile,
    IN HINSTANCE        hInstance,
    IN LPVOID           szResourceName,
    IN LPVOID           szResourceType,
    IN DWORD            dwExtraValue
    )
{
    WCHAR  szwTemp[MAX_PATH];
    LPWSTR szwName = NULL;
    LPWSTR szwType = NULL;
    BOOL   bResult = FALSE;

    if ((NULL == szCacheFileName) || (NULL == pfnSetFromFile))
    {
        return FALSE;
    }

    if (TRUE == bUnicode || IS_INTRESOURCE(szResourceName))
    {
        szwName = (LPWSTR) szResourceName;
    }
    else
    {
        szwName = WuAnsiToWideHeapAlloc(szResourceName);
    }

    if (TRUE == bUnicode || IS_INTRESOURCE(szResourceType))
    {
        szwType = (LPWSTR) szResourceType;
    }
    else
    {
        szwType = WuAnsiToWideHeapAlloc(szResourceType);
    }

    bResult = _WuGetWinUtilzCacheFileName(szCacheFileName, szwTemp, MAX_PATH);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    bResult = WuExtractResourceToFileW(hInstance, szwName, szwType, szwTemp);
    
    if (FALSE == bResult)
    {
        goto cleanup;
    }

    bResult = pfnSetFromFile(szwTemp, dwExtraValue);

cleanup:
    if (FALSE == bUnicode && IS_INTRESOURCE(szwName) == FALSE)
    {
        HeapFree(GetProcessHeap(), 0, szwName);
    }

    if (FALSE == bUnicode && IS_INTRESOURCE(szwType) == FALSE)
    {
        HeapFree(GetProcessHeap(), 0, szwType);
    }

    return bResult;
}

BOOL
_WuSetSomethingFromUrl(
    IN BOOL             bUnicode,
    IN LPCWSTR          szCacheFileName,
    IN SETFROMFILEPROC  pfnSetFromFile,
    IN LPVOID           szUrl,
    IN DWORD            dwExtraValue
    )
{
    WCHAR  szwTemp[MAX_PATH];
    LPWSTR szwUrl  = NULL;
    BOOL   bResult = FALSE;

    if ((NULL == szCacheFileName) || (NULL == pfnSetFromFile))
    {
        return FALSE;
    }

    if (NULL == szUrl)
    {
        return FALSE;
    }

    if (TRUE == bUnicode)
    {
        szwUrl = (LPWSTR) szUrl;
    }
    else
    {
        szwUrl = WuAnsiToWideHeapAlloc((LPCSTR) szUrl);

        if (NULL == szwUrl)
        {
            goto cleanup;
        }
    }

    bResult = _WuGetWinUtilzCachePath(szCacheFileName, szwTemp, MAX_PATH);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    bResult = WuDownloadFileW(szwUrl, szwTemp);
    
    if (FALSE == bResult)
    {
        goto cleanup;
    }

    bResult = pfnSetFromFile(szwTemp, dwExtraValue);

cleanup:
    if ((szwUrl != NULL) && (TRUE == bUnicode))
    {
        HeapFree(GetProcessHeap(), 0, szwUrl);
    }

    return bResult;
}

BOOL
_WuSafeExpandEnvironmentStrings(
    IN  LPCWSTR szSource,
    OUT LPWSTR  szBuffer,
    IN  DWORD   cchSize
    )
{
    DWORD cchWritten = 0;

    if ((NULL == szSource) || (NULL == szBuffer) || (cchSize <= 0))
    {
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    cchWritten = ExpandEnvironmentStringsW(szSource, szBuffer, cchSize);

    return !((0 == cchWritten) && (GetLastError() != ERROR_SUCCESS));
}

BOOL
_WuCurrentThemeGetStringProperty(
    IN  LPCWSTR szSection,
    IN  LPCWSTR szKey,
    OUT LPWSTR  szBuffer,
    IN  ULONG   cchBufferSize
    )
{
    static WCHAR s_szThemePath[MAX_PATH];
    static BOOL  s_bThemePathInitialized = FALSE;

    HKEY    hThemesKey  = NULL;
    DWORD   cbData      = sizeof(s_szThemePath);
    LPCWSTR szKeyName   = NULL;
    DWORD   cchWritten  = 0;
    LONG    lResult     = ERROR_SUCCESS;

    if (NULL == szSection || NULL == szKey || NULL == szBuffer)
    {
        return FALSE;
    }

    if (0 == cchBufferSize)
    {
        return FALSE;
    }

    if (s_bThemePathInitialized == TRUE && s_szThemePath[0] != L'\0')
    {
        goto get_property;
    }

    lResult = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_THEMES,
        0,
        KEY_QUERY_VALUE,
        &hThemesKey);

    if ((lResult != ERROR_SUCCESS) || (NULL == hThemesKey))
    {
        return FALSE;
    }

    szKeyName = IsWindowsVistaOrGreater()
        ? REGISTRY_KEY_CURRENTTHEME
        : REGISTRY_KEY_INSTALLTHEME;

    ZeroMemory(s_szThemePath, sizeof(s_szThemePath));

    lResult = RegQueryValueExW(
        hThemesKey,
        szKeyName,
        NULL,
        NULL,
        (LPBYTE) s_szThemePath,
        &cbData);
    
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hThemesKey);
        return FALSE;
    }
    else
    {
        s_bThemePathInitialized = TRUE;
    }

    RegCloseKey(hThemesKey);

get_property:
    SetLastError(ERROR_SUCCESS);

    cchWritten = GetPrivateProfileStringW(
        szSection,
        szKey,
        L"",
        szBuffer,
        cchBufferSize,
        s_szThemePath);

    return !((0 == cchWritten) && (GetLastError() != ERROR_SUCCESS));
}
