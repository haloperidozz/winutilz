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

#include "internal.h"

#define REGISTRY_PATH_CURSORS   L"Control Panel\\Cursors"

#define CURSOR_MAX  17

#define WU_CURSOR_ICON_MAX  CURSOR_MAX

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

#define MAKE_FOURCC(ch1, ch2, ch3, ch4)                                 \
    ((DWORD) ((ch1) | ((ch2) << 8) | ((ch3) << 16) | ((ch4) << 24)))

#define FOURCC_RIFF_ID      MAKE_FOURCC('R', 'I', 'F', 'F')
#define FOURCC_ACON_ID      MAKE_FOURCC('A', 'C', 'O', 'N')

#define DCFT_BUFFER_SIZE    12  /* 12 bytes is enough */

typedef enum {
    CURSOR_FILE_TYPE_CUR,
    CURSOR_FILE_TYPE_ANI,
    CURSOR_FILE_TYPE_UNKNOWN
} CURSOR_FILE_TYPE;

static CURSOR_FILE_TYPE
DetermineCursorFileType(
    IN LPCWSTR  szFilePath
    )
{
    BYTE   abBuffer[DCFT_BUFFER_SIZE];
    DWORD  dwReaded = 0;
    HANDLE hFile    = INVALID_HANDLE_VALUE;
    BOOL   bResult  = FALSE;
    WORD   wIdReserved, wIdType, wIdCount;  /* .cur (ICO )  */
    DWORD  dwChunkId, dwChunkType;          /* .ani (RIFF)  */

    if (NULL == szFilePath)
    {
        return CURSOR_FILE_TYPE_UNKNOWN;
    }

    hFile = CreateFileW(
        szFilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return CURSOR_FILE_TYPE_UNKNOWN;
    }

    bResult = ReadFile(
        hFile,
        abBuffer,
        DCFT_BUFFER_SIZE,
        &dwReaded,
        NULL);

    CloseHandle(hFile);

    if ((FALSE == bResult) || (dwReaded != DCFT_BUFFER_SIZE))
    {
        return CURSOR_FILE_TYPE_UNKNOWN;
    }

    wIdReserved = *((WORD*) &abBuffer[0]);
    wIdType     = *((WORD*) &abBuffer[2]);
    wIdCount    = *((WORD*) &abBuffer[4]);

    if ((0 == wIdReserved) && (2 == wIdType) && (wIdCount >= 1))
    {
        return CURSOR_FILE_TYPE_CUR;
    }

    dwChunkId   = *((DWORD*) &abBuffer[0]);
    dwChunkType = *((DWORD*) &abBuffer[8]);

    if ((FOURCC_RIFF_ID == dwChunkId) && (FOURCC_ACON_ID == dwChunkType))
    {
        return CURSOR_FILE_TYPE_ANI;
    }
    
    return CURSOR_FILE_TYPE_UNKNOWN;
}

static WU_INLINE BOOL
IsValidCursorFile(
    IN LPCWSTR  szFilePath
    )
{
    return (DetermineCursorFileType(szFilePath) != CURSOR_FILE_TYPE_UNKNOWN);
}

WUAPI BOOL
WuSetCursorW(
    IN LPCWSTR          szCursorPath,
    IN WU_CURSOR_ICON   icon
    )
{
    WCHAR   szTempPath[MAX_PATH];
    DWORD   cchWritten  = 0;
    HKEY    hCursorsKey = NULL;
    LSTATUS lStatus     = STATUS_SUCCESS;

    if ((NULL == szCursorPath) || (icon >= WU_CURSOR_ICON_MAX))
    {
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    cchWritten = ExpandEnvironmentStringsW(
        szCursorPath,
        szTempPath,
        MAX_PATH);

    if (0 == cchWritten)
    {
        if (GetLastError() != ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    if (IsValidCursorFile(szTempPath) == FALSE)
    {
        return FALSE;
    }

    lStatus = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_CURSORS,
        0,
        KEY_SET_VALUE,
        &hCursorsKey);

    if ((lStatus != ERROR_SUCCESS) || (NULL == hCursorsKey))
    {
        return FALSE;
    }

    lStatus = RegSetValueExW(
        hCursorsKey,
        g_aszCursorRegistryNames[icon],
        0,
        REG_EXPAND_SZ,
        (CONST BYTE*) szTempPath,
        MAX_PATH * sizeof(WCHAR));
    
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
    
    if (NULL == szCursorPath)
    {
        return FALSE;
    }

    if (WuAnsiToWide(szCursorPath, szwPath, MAX_PATH) == FALSE)
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

    if ((NULL == szCursorPath) || (cchCursorPath <= 0))
    {
        return FALSE;
    }

    if (icon >= WU_CURSOR_ICON_MAX)
    {
        return FALSE;
    }

    lStatus = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_CURSORS,
        0,
        KEY_QUERY_VALUE,
        &hCursorsKey);

    if ((lStatus != ERROR_SUCCESS) || (NULL == hCursorsKey))
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

    if (0 == cchWritten)
    {
        if (GetLastError() != ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    return (cchWritten <= cchCursorPath);
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

    if ((NULL == szCursorPath) || (cchCursorPath <= 0))
    {
        return FALSE;
    }

    if (icon >= WU_CURSOR_ICON_MAX)
    {
        return FALSE;
    }

    bResult = WuGetCursorW(szwTempPath, MAX_PATH, icon);

    if (FALSE == bResult)
    {
        return FALSE;
    }

    return WuWideToAnsi(szwTempPath, szCursorPath, cchCursorPath);
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
