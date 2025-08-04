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

#define CALL_LAZY_DLL_FUNC(ret_var, dll_name, address, func_type, ...)  \
    do                                                                  \
    {                                                                   \
        static HMODULE   hModule = NULL;                                \
        static func_type fpFunc  = NULL;                                \
                                                                        \
        if (fpFunc == NULL)                                             \
        {                                                               \
            if (hModule == NULL)                                        \
            {                                                           \
                hModule = LoadLibraryExA(                               \
                    (dll_name),                                         \
                    NULL,                                               \
                    LOAD_LIBRARY_SEARCH_SYSTEM32);                      \
            }                                                           \
            if (hModule != NULL)                                        \
            {                                                           \
                fpFunc = (func_type) GetProcAddress(                    \
                    hModule,                                            \
                    (LPCSTR) address);                                  \
            }                                                           \
        }                                                               \
                                                                        \
        (ret_var) = (fpFunc ? fpFunc(__VA_ARGS__) : 0);                 \
    }                                                                   \
    while (FALSE)

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
    LPWSTR szFormatResult = NULL;

    CALL_LAZY_DLL_FUNC(
        szFormatResult,
        "winbrand.dll",
        "BrandingFormatString",
        BrandingFormatStringType,
        szFormat);

    return szFormatResult;
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
    HWND hWorkerWnd = NULL;

    CALL_LAZY_DLL_FUNC(
        hWorkerWnd,
        "shlwapi.dll",
        278,                            /* ordinal */
        SHCreateWorkerWindowWType,
        fnWndProc,
        hWndParent,
        dwExStyle,
        dwStyle,
        hMenu,
        wnd_extra);

    return hWorkerWnd;
}
