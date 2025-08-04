/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       window.c
 *
 ***************************************************************************/

#include "winutilz.h"

WUAPI BOOL
WuCenterWindow(
    IN HWND hWnd
    )
{
    RECT  rcWnd, rcParentWnd;
    SIZE  sizeWnd, sizeParentWnd;
    POINT ptWndPosition;
    HWND  hParentWnd = NULL;

    if (hWnd == NULL || IsWindow(hWnd) == FALSE)
    {
        return FALSE;
    }

    if (GetWindowRect(hWnd, &rcWnd) == FALSE)
    {
        return FALSE;
    }

    sizeWnd.cx = rcWnd.right - rcWnd.left;
    sizeWnd.cy = rcWnd.bottom - rcWnd.top;

    if ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD) == 0)
    {
        sizeParentWnd.cx = GetSystemMetrics(SM_CXSCREEN);
        sizeParentWnd.cy = GetSystemMetrics(SM_CYSCREEN);

        ptWndPosition.x = (sizeParentWnd.cx - sizeWnd.cx) / 2;
        ptWndPosition.y = (sizeParentWnd.cy - sizeWnd.cy) / 2;
    }
    else
    {
        hParentWnd = GetParent(hWnd);

        if (hParentWnd == NULL)
        {
            hParentWnd = GetDesktopWindow();
        }

        if (GetWindowRect(hParentWnd, &rcParentWnd) == FALSE)
        {
            return FALSE;
        }

        sizeParentWnd.cx = rcParentWnd.right - rcParentWnd.left;
        sizeParentWnd.cy = rcParentWnd.bottom - rcParentWnd.top;

        ptWndPosition.x = rcParentWnd.right;
        /* offset from the right side of the parent window */
        ptWndPosition.x += (sizeParentWnd.cx - sizeWnd.cx) / 2;

        ptWndPosition.y = rcParentWnd.top;
        /* offset from the top side of the parent window */
        ptWndPosition.y += (sizeParentWnd.cy - sizeWnd.cy) / 2;
    }

    return SetWindowPos(
        hWnd, NULL,
        ptWndPosition.x, ptWndPosition.y,
        0, 0,
        SWP_NOSIZE | SWP_NOZORDER);
}

static BOOL
ModifyWindowStyle(
    IN HWND     hWnd,
    IN INT      nIndex,
    IN BOOL     bEnable,
    IN DWORD    dwSingleStyle
    )
{
    DWORD dwCurrentStyle = 0;
    DWORD dwNewStyle     = 0;

    if (hWnd == NULL || IsWindow(hWnd) == FALSE)
    {
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    dwCurrentStyle = (DWORD) GetWindowLongPtrW(hWnd, nIndex);

    if (dwCurrentStyle == 0 && GetLastError() != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (bEnable == TRUE)
    {
        dwNewStyle = dwCurrentStyle | dwSingleStyle;
    }
    else
    {
        dwNewStyle = dwCurrentStyle & ~dwSingleStyle;
    }

    if (dwNewStyle == dwCurrentStyle)
    {
        return TRUE;
    }

    SetLastError(ERROR_SUCCESS);

    dwCurrentStyle = (DWORD) SetWindowLongPtrW(hWnd, nIndex, dwNewStyle);

    if (dwCurrentStyle == 0 && GetLastError() != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return SetWindowPos(
        hWnd, NULL,
        0, 0,
        0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

WUAPI BOOL
WuModifyWindowStyle(
    IN HWND     hWnd,
    IN BOOL     bEnable,
    IN DWORD    dwStyleIndex
    )
{
    return ModifyWindowStyle(hWnd, GWL_STYLE, bEnable, dwStyleIndex);
}

WUAPI BOOL
WuModifyWindowExStyle(
    IN HWND     hWnd,
    IN BOOL     bEnable,
    IN DWORD    dwExStyleIndex
    )
{
    return ModifyWindowStyle(hWnd, GWL_EXSTYLE, bEnable, dwExStyleIndex);
}
