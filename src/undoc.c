/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       undoc.c
 *
 ***************************************************************************/

#include "undoc.h"

/*
    +-------------------------------------------------------------------+
    |   WINBRAND.DLL                                                    |
    +-------------------------------------------------------------------+
*/

typedef LPWSTR (WINAPI * BrandingFormatStringType)(LPCWSTR);

LPWSTR
BrandingFormatString(
    IN LPCWSTR  szFormat
    )
{
    static HMODULE                  hModule = NULL;
    static BrandingFormatStringType fpFunc  = NULL;
    
    if (NULL == fpFunc)
    {
        if (NULL == hModule)
        {
            hModule = LoadLibraryExA(
                "winbrand.dll",
                NULL,
                LOAD_LIBRARY_SEARCH_SYSTEM32);
        }
        
        if (hModule != NULL)
        {
            fpFunc = (BrandingFormatStringType) GetProcAddress(
                hModule,
                "BrandingFormatString");
        }
    }
    
    return (fpFunc != NULL) ? fpFunc(szFormat) : NULL;
}

/*
    +-------------------------------------------------------------------+
    |   SHLWAPI.DLL                                                     |
    +-------------------------------------------------------------------+
*/

typedef HWND (WINAPI * SHCreateWorkerWindowWType)(
    WNDPROC,
    HWND,
    DWORD,
    DWORD,
    HMENU,
    LONG_PTR);

HWND
SHCreateWorkerWindowW(
    IN WNDPROC  fnWndProc,
    IN HWND     hWndParent,
    IN DWORD    dwExStyle,
    IN DWORD    dwStyle,
    IN HMENU    hMenu,
    IN LONG_PTR wnd_extra)
{
    static HMODULE                   hModule = NULL;
    static SHCreateWorkerWindowWType fpFunc  = NULL;
    
    if (NULL == fpFunc)
    {
        if (NULL == hModule)
        {
            hModule = LoadLibraryExA(
                "shlwapi.dll",
                NULL,
                LOAD_LIBRARY_SEARCH_SYSTEM32);
        }
        
        if (hModule != NULL)
        {
            fpFunc = (SHCreateWorkerWindowWType) GetProcAddress(
                hModule,
                (LPCSTR) 278 /* ordinal */ );
        }
    }
    
    return (fpFunc != NULL)
        ? fpFunc(fnWndProc, hWndParent, dwExStyle, dwStyle, hMenu, wnd_extra)
        : NULL;
}
