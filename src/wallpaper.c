/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       wallpaper.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <ntstatus.h>

#include "internal.h"

#define REGISTRY_PATH_DESKTOP   L"Control Panel\\Desktop"

#define REGISTRY_KEY_TILEWALLPAPER  L"TileWallpaper"
#define REGISTRY_KEY_WALLPAPERSTYLE L"WallpaperStyle"

#define CACHE_FILENAME  L"Wallpaper"

WUAPI BOOL
WuSetWallpaperW(
    IN LPCWSTR              szWallpaperPath,
    IN WU_WALLPAPER_STYLE   style   
    )
{
    WCHAR   szTempPath[MAX_PATH];
    HKEY    hDesktopKey      = NULL;
    LPCWSTR szWallpaperStyle = NULL;
    LPCWSTR szTileWallpaper  = NULL;
    LSTATUS lStatus          = STATUS_SUCCESS;
    DWORD   cchTempPath      = 0;

    if (szWallpaperPath == NULL)
    {
        return FALSE;
    }

    lStatus = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_DESKTOP,
        0,
        KEY_SET_VALUE,
        &hDesktopKey);

    if (lStatus != ERROR_SUCCESS || hDesktopKey == NULL)
    {
        return FALSE;
    }

/*
    Values taken from:                                           
    https://github.com/reujab/wallpaper.rs/blob/master/src/windows.rs
*/
    switch (style)
    {
        case WU_WALLPAPER_STYLE_CENTER:     /* WPSTYLE_CENTER       */
            szWallpaperStyle = L"0";
            szTileWallpaper  = L"0";
            break;
        case WU_WALLPAPER_STYLE_TILE:       /* WPSTYLE_TILE         */
            szWallpaperStyle = L"0";
            szTileWallpaper  = L"1";
            break;
        case WU_WALLPAPER_STYLE_STRETCH:    /* WPSTYLE_STRETCH      */
            szWallpaperStyle = L"2";
            szTileWallpaper  = L"0";
            break;
        case WU_WALLPAPER_STYLE_KEEPASPECT: /* WPSTYLE_KEEPASPECT   */
            szWallpaperStyle = L"6";
            szTileWallpaper  = L"0";
            break;
        case WU_WALLPAPER_STYLE_CROPTOFIT:  /* WPSTYLE_CROPTOFIT    */
            szWallpaperStyle = L"10";
            szTileWallpaper  = L"0";
            break;
        case WU_WALLPAPER_STYLE_SPAN:       /* WPSTYLE_SPAN         */
            szWallpaperStyle = L"22";
            szTileWallpaper  = L"0";
            break;
        default:
            return FALSE;
    }

    lStatus = RegSetValueExW(
        hDesktopKey,
        REGISTRY_KEY_TILEWALLPAPER,
        0,
        REG_SZ,
        (LPBYTE) szTileWallpaper,
        (lstrlenW(szTileWallpaper) + 1) * sizeof(WCHAR));
    
    if (lStatus != ERROR_SUCCESS)
    {
        RegCloseKey(hDesktopKey);
        return FALSE;
    }

    lStatus = RegSetValueExW(
        hDesktopKey,
        REGISTRY_KEY_WALLPAPERSTYLE,
        0,
        REG_SZ,
        (LPBYTE) szWallpaperStyle,
        (lstrlenW(szWallpaperStyle) + 1) * sizeof(WCHAR));
    
    if (lStatus != ERROR_SUCCESS)
    {
        RegCloseKey(hDesktopKey);
        return FALSE;
    }

    if (RegCloseKey(hDesktopKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    cchTempPath = ExpandEnvironmentStringsW(
        szWallpaperPath,
        szTempPath,
        MAX_PATH);

    if (cchTempPath == 0)
    {
        if (GetLastError() != ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    return SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        (PVOID) szTempPath,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

WUAPI BOOL
WuSetWallpaperA(
    IN LPCSTR               szWallpaperPath,
    IN WU_WALLPAPER_STYLE   style   
    )
{
    WCHAR szwPath[MAX_PATH];
    
    if (szWallpaperPath == NULL)
    {
        return FALSE;
    }

    if (WuAnsiToWide(szWallpaperPath, szwPath, MAX_PATH) == FALSE)
    {
        return FALSE;
    }

    return WuSetWallpaperW(szwPath, style);
}

WUAPI BOOL
WuGetWallpaperW(
    OUT LPWSTR  szWallpaperPath,
    IN  ULONG   cchWallpaperPath
    )
{
    if (szWallpaperPath == NULL || cchWallpaperPath <= 0)
    {
        return FALSE;
    }

    return SystemParametersInfoW(
        SPI_GETDESKWALLPAPER,
        (UINT) cchWallpaperPath,
        (PVOID) szWallpaperPath,
        0);
}

WUAPI BOOL
WuGetWallpaperA(
    OUT LPSTR   szWallpaperPath,
    IN  ULONG   cchWallpaperPath
    )
{
    if (szWallpaperPath == NULL || cchWallpaperPath <= 0)
    {
        return FALSE;
    }

    return SystemParametersInfoA(
        SPI_GETDESKWALLPAPER,
        (UINT) cchWallpaperPath,
        (PVOID) szWallpaperPath,
        0);
}

WUAPI BOOL
WuSetWallpaperStyle(
    IN WU_WALLPAPER_STYLE   style
    )
{
    WCHAR szWallpaperPath[MAX_PATH];
    BOOL  bResult = FALSE;

    bResult = WuGetWallpaperW(szWallpaperPath, MAX_PATH);

    if (bResult == FALSE)
    {
        return FALSE;
    }

    return WuSetWallpaperW(szWallpaperPath, style);
}

WUAPI BOOL
WuSetWallpaperFromImageData(
    IN PIMAGEDATA           pImageData,
    IN WU_WALLPAPER_STYLE   style
    )
{
    WCHAR szWallpaperPath[MAX_PATH];
    BOOL  bResult = FALSE;

    if (pImageData == NULL || pImageData->abData == NULL)
    {
        return FALSE;
    }

    bResult = GetWinUtilzCacheFileName(
        CACHE_FILENAME,
        szWallpaperPath,
        MAX_PATH);

    if (bResult == FALSE)
    {
        return FALSE;
    }

    bResult = WuSaveImageDataToFileW(
        pImageData,
        szWallpaperPath,
        WU_IMAGE_FORMAT_BMP);
    
    if (bResult == FALSE)
    {
        return FALSE;
    }
    
    return WuSetWallpaperW(szWallpaperPath, style);
}

WUAPI BOOL
WuSetWallpaperFromResourceW(
    IN HINSTANCE            hInstance,
    IN LPCWSTR              szResourceName,
    IN LPCWSTR              szResourceType,
    IN WU_WALLPAPER_STYLE   style
    )
{
    return SetSomethingFromResource(
        TRUE,
        CACHE_FILENAME,
        (SETFROMFILEPROC) WuSetWallpaperW,
        hInstance,
        (LPVOID) szResourceName,
        (LPVOID) szResourceType,
        (DWORD) style);
}

WUAPI BOOL
WuSetWallpaperFromResourceA(
    IN HINSTANCE            hInstance,
    IN LPCSTR               szResourceName,
    IN LPCSTR               szResourceType,
    IN WU_WALLPAPER_STYLE   style
    )
{
    return SetSomethingFromResource(
        FALSE,
        CACHE_FILENAME,
        (SETFROMFILEPROC) WuSetWallpaperW,
        hInstance,
        (LPVOID) szResourceName,
        (LPVOID) szResourceType,
        (DWORD) style);
}

WUAPI BOOL
WuSetWallpaperFromUrlW(
    IN LPCWSTR              szWallpaperUrl,
    IN WU_WALLPAPER_STYLE   style
    )
{
    return SetSomethingFromUrl(
        TRUE,
        CACHE_FILENAME,
        (SETFROMFILEPROC) WuSetWallpaperW,
        (LPVOID) szWallpaperUrl,
        (DWORD) style);
}

WUAPI BOOL
WuSetWallpaperFromUrlA(
    IN LPCSTR               szWallpaperUrl,
    IN WU_WALLPAPER_STYLE   style
    )
{
    return SetSomethingFromUrl(
        FALSE,
        CACHE_FILENAME,
        (SETFROMFILEPROC) WuSetWallpaperW,
        (LPVOID) szWallpaperUrl,
        (DWORD) style);
}

WUAPI BOOL
WuSetWallpaperBackgroundColor(
    IN COLORREF crColor
    )
{
    INT      aiElements[2] = { COLOR_BACKGROUND, COLOR_DESKTOP };
    COLORREF  acrValues[2] = { crColor, crColor };
    
    return WuSaveSysColors(2, aiElements, acrValues);
}

WUAPI COLORREF
WuGetWallpaperBackgroundColor(
    VOID
    )
{
    return GetSysColor(COLOR_DESKTOP);
}
