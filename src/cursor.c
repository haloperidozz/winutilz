/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       cursor.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <ntstatus.h>

#include "strconv.h"
#include "internal.h"

#define REGISTRY_PATH_CURSORS   L"Control Panel\\Cursors"

#define CURSOR_MAX  17

static LPCWSTR g_aszCursorRegistryNames[CURSOR_MAX] = {
    L"AppStarting",     /* OCR_APPSTARTING  */
    L"Arrow",           /* OCR_NORMAL       */
    L"Crosshair",       /* OCR_CROSS        */
    L"Hand",            /* OCR_HAND         */
    L"Help",
    L"IBeam",           /* OCR_IBEAM        */
    L"No",              /* OCR_NO           */
    L"NWPen",
    L"Person",
    L"Pin",
    L"SizeAll",         /* OCR_SIZEALL       */
    L"SizeNESW",        /* OCR_SIZENESW      */
    L"SizeNS",          /* OCR_SIZENS        */
    L"SizeNWSE",        /* OCR_SIZENWSE      */
    L"SizeWE",          /* OCR_SIZEWE        */
    L"UpArrow",         /* OCR_UP            */
    L"Wait"             /* OCR_WAIT          */
};

static BOOL
IsValidCursorFile(
    IN LPCWSTR  szFilePath
    )
{
    BYTE   abBuffer[12];
    DWORD  dwReaded    = 0;
    HANDLE hFile       = INVALID_HANDLE_VALUE;
    BOOL   bResult     = FALSE;
    WORD   wIdReserved, wIdType, wIdCount;
    DWORD  dwRiff, dwType;

    if (szFilePath == NULL)
    {
        return FALSE;
    }

    hFile = CreateFileW(
        szFilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    bResult = ReadFile(
        hFile,
        abBuffer,
        sizeof(abBuffer),
        &dwReaded,
        NULL);

    CloseHandle(hFile);

    if (bResult == FALSE || dwReaded < 6)
    {
        return FALSE;
    }

    wIdReserved = *((WORD*) &abBuffer[0]);
    wIdType     = *((WORD*) &abBuffer[2]);
    wIdCount    = *((WORD*) &abBuffer[4]);

    if (wIdReserved == 0 && wIdType == 2 && wIdCount >= 1)
    {
        return TRUE;            /* .cur */
    }
    else if (dwReaded >= 12)
    {
        dwRiff = *((DWORD*) &abBuffer[0]);  /* RIFF */
        dwType = *((DWORD*) &abBuffer[8]);  /* ACON */

        /* 0x46 = 'F', 0x46 = 'F', 0x49 = 'I', 0x52 = 'R' */
        /* 0x4E = 'N', 0x4F = 'O', 0x43 = 'C', 0x41 = 'A' */

        if (dwRiff == 0x46464952 || dwType == 0x4E4F4341)
        {
            return TRUE;        /* .ani */
        }
    }
    
    return FALSE;
}

WUAPI BOOL
WuSetCursorW(
    IN LPCWSTR          szCursorPath,
    IN WU_CURSOR_ICON   icon
    )
{
    HKEY    hCursorsKey = NULL;
    LSTATUS lStatus     = STATUS_SUCCESS;

    if (szCursorPath == NULL || icon >= CURSOR_MAX)
    {
        return FALSE;
    }

    if (IsValidCursorFile(szCursorPath) == FALSE)
    {
        return FALSE;
    }

    lStatus = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_CURSORS,
        0,
        KEY_SET_VALUE,
        &hCursorsKey);

    if (lStatus != ERROR_SUCCESS || hCursorsKey == NULL)
    {
        return FALSE;
    }

    lStatus = RegSetValueExW(
        hCursorsKey,
        g_aszCursorRegistryNames[icon],
        0,
        REG_EXPAND_SZ,
        (CONST BYTE*) szCursorPath,
        (lstrlenW(szCursorPath) + 1) * sizeof(WCHAR));
    
    if (lStatus != ERROR_SUCCESS)
    {
        RegCloseKey(hCursorsKey);
        return FALSE;
    }

    if (RegCloseKey(hCursorsKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return SystemParametersInfoW(
        SPI_SETCURSORS,
        0,
        0,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

WUAPI BOOL
WuSetCursorA(
    IN LPCSTR           szCursorPath,
    IN WU_CURSOR_ICON   icon
    )
{
    WCHAR szwPath[MAX_PATH];
    
    if (szCursorPath == NULL)
    {
        return FALSE;
    }

    if (AnsiToWide(szCursorPath, szwPath, MAX_PATH) == FALSE)
    {
        return FALSE;
    }

    return WuSetCursorW(szwPath, icon);
}

WUAPI BOOL
WuGetCursorW(
    OUT LPWSTR          szCursorPath,
    IN  ULONG           cchCursorPath,
    IN  WU_CURSOR_ICON  icon
    )
{
    WCHAR   szTempPath[MAX_PATH];
    HKEY    hCursorsKey = NULL;
    DWORD   cchWritten  = 0;
    DWORD   cbData      = sizeof(szTempPath);
    LSTATUS lStatus     = STATUS_SUCCESS;

    if (szCursorPath == NULL || cchCursorPath <= 0 || icon >= CURSOR_MAX)
    {
        return FALSE;
    }

    lStatus = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_CURSORS,
        0,
        KEY_QUERY_VALUE,
        &hCursorsKey);

    if (lStatus != ERROR_SUCCESS || hCursorsKey == NULL)
    {
        return FALSE;
    }

    ZeroMemory(szTempPath, cbData);

    lStatus = RegQueryValueExW(
        hCursorsKey,
        g_aszCursorRegistryNames[icon],
        NULL,
        NULL,
        (LPBYTE) szTempPath,
        &cbData);
    
    if (lStatus != ERROR_SUCCESS)
    {
        RegCloseKey(hCursorsKey);
        return FALSE;
    }

    if (RegCloseKey(hCursorsKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    cchWritten = ExpandEnvironmentStringsW(
        szTempPath,
        szCursorPath,
        cchCursorPath);

    if (cchWritten == 0)
    {
        if (GetLastError() != ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    return cchWritten <= cchCursorPath;
}

WUAPI BOOL
WuGetCursorA(
    OUT LPSTR           szCursorPath,
    IN  ULONG           cchCursorPath,
    IN  WU_CURSOR_ICON  icon
    )
{
    WCHAR szwTempPath[MAX_PATH];
    BOOL  bResult = FALSE;

    if (szCursorPath == NULL || cchCursorPath <= 0 || icon >= CURSOR_MAX)
    {
        return FALSE;
    }

    bResult = WuGetCursorW(szwTempPath, MAX_PATH, icon);

    if (bResult == FALSE)
    {
        return FALSE;
    }

    return WideToAnsi(szwTempPath, szCursorPath, cchCursorPath);
}

WUAPI BOOL
WuSetCursorFromResourceW(
    IN HINSTANCE        hInstance,
    IN LPCWSTR          szResourceName,
    IN LPCWSTR          szResourceType,
    IN WU_CURSOR_ICON   icon
    )
{
    return SetSomethingFromResource(
        TRUE,
        g_aszCursorRegistryNames[icon],
        (SETFROMFILEPROC) WuSetCursorW,
        hInstance,
        (LPVOID) szResourceName,
        (LPVOID) szResourceType,
        icon);
}

WUAPI BOOL
WuSetCursorFromResourceA(
    IN HINSTANCE        hInstance,
    IN LPCSTR           szResourceName,
    IN LPCSTR           szResourceType,
    IN WU_CURSOR_ICON   icon
    )
{
    return SetSomethingFromResource(
        FALSE,
        g_aszCursorRegistryNames[icon],
        (SETFROMFILEPROC) WuSetCursorW,
        hInstance,
        (LPVOID) szResourceName,
        (LPVOID) szResourceType,
        icon);
}

WUAPI BOOL
WuSetCursorFromUrlW(
    IN LPCWSTR          szWallpaperUrl,
    IN WU_CURSOR_ICON   icon
    )
{
    return SetSomethingFromUrl(
        TRUE,
        g_aszCursorRegistryNames[icon],
        (SETFROMFILEPROC) WuSetCursorW,
        (LPVOID) szWallpaperUrl,
        icon);
}

WUAPI BOOL
WuSetCursorFromUrlA(
    IN LPCSTR           szWallpaperUrl,
    IN WU_CURSOR_ICON   icon
    )
{
    return SetSomethingFromUrl(
        FALSE,
        g_aszCursorRegistryNames[icon],
        (SETFROMFILEPROC) WuSetCursorW,
        (LPVOID) szWallpaperUrl,
        icon);
}
