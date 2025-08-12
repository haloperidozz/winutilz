/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       shell.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <versionhelpers.h>
#include <ntstatus.h>

#define WC_DEFVIEW  L"SHELLDLL_DefView"

#define WM_REFRESH          0x7103
#define WM_HARD_REFRESH     0x7104

#define ICON_ARRANGE_MAX    4

static CONST UINT s_auIconArrangeCommandWin7[ICON_ARRANGE_MAX] = {
    0x7071,     /* DESKTOP_ICON_ARRANGE_AUTO            */
    0x7072,     /* DESKTOP_ICON_ARRANGE_GRID            */
    0x7073,     /* DESKTOP_ICON_ARRANGE_DISPLAYICONS    */
    0x7074      /* DESKTOP_ICON_ARRANGE_AUTOGRID        */
};

static CONST UINT s_auIconArrangeCommandWinXp[ICON_ARRANGE_MAX] = {
    0x7051,     /* DESKTOP_ICON_ARRANGE_AUTO            */
    0x7052,     /* DESKTOP_ICON_ARRANGE_GRID            */
    0x7053,     /* DESKTOP_ICON_ARRANGE_DISPLAYICONS    */
    0x7054      /* DESKTOP_ICON_ARRANGE_AUTOGRID        */
};

static BOOL CALLBACK
FindDefViewWindow_EnumWindowsProc(
    IN HWND     hWnd,
    IN LPARAM   lParam
    )
{
    HWND hNextWin     = FindWindowExW(hWnd, 0, WC_DEFVIEW, 0);
    HWND hNextNextWin = NULL;
    HWND hNextPrevWin = NULL;

    /* https://stackoverflow.com/a/60856252 */
    
    if (hNextWin == NULL)
    {
        return TRUE;
    }

    hNextNextWin = GetNextWindow(hNextWin, GW_HWNDNEXT);
    hNextPrevWin = GetNextWindow(hNextWin, GW_HWNDPREV);

    if ((hNextNextWin != NULL) || (hNextPrevWin != NULL))
    {
        return TRUE;
    }

    *((HWND*) lParam) = hNextWin;

    return FALSE;
}

WUAPI HWND
WuGetDesktopDefViewWindow(
    VOID
    )
{
    HWND hDefView = NULL;
    HWND hProgMan = GetShellWindow();
    
    if (hProgMan == NULL)
    {
        return NULL;
    }

    hDefView = FindWindowExW(hProgMan, NULL, WC_DEFVIEW, NULL);

    if (hDefView != NULL)
    {
        return hDefView;
    }

    EnumWindows(FindDefViewWindow_EnumWindowsProc, (LPARAM) &hDefView);

    return hDefView;
}

static VOID
DefViewSendCommandId(
    IN UINT uID
    )
{
    HWND hDefView = WuGetDesktopDefViewWindow();

    if (hDefView == NULL)
    {
        return;
    }
/*
    The DefView window procedure is capable of processing some
    undocumented WM_COMMAND message params
*/
    SendMessageW(hDefView, WM_COMMAND, GET_WM_COMMAND_MPS(uID, 0, 0));
}

WUAPI HWND
WuGetDesktopListView(
    VOID
    )
{
    HWND hDefView = WuGetDesktopDefViewWindow();

    if (hDefView == NULL)
    {
        return NULL;
    }

    return FindWindowExW(hDefView, NULL, WC_LISTVIEWW, NULL);
}

WUAPI VOID
WuDesktopRefresh(
    IN BOOL bHardRefresh
    )
{
    DefViewSendCommandId((bHardRefresh) ? WM_HARD_REFRESH : WM_REFRESH);
}

WUAPI VOID
WuDesktopToggleIconArrangement(
    IN WU_DESKTOP_ICON_ARRANGE  arrange
    )
{
    if (arrange >= ICON_ARRANGE_MAX)
    {
        return;
    }

    if (IsWindows7OrGreater() != FALSE)
    {
        DefViewSendCommandId(s_auIconArrangeCommandWin7[arrange]);
    }
    else
    {
        DefViewSendCommandId(s_auIconArrangeCommandWinXp[arrange]);
    }
}

WUAPI BOOL
WuDesktopAreIconsArrangedByGrid(
    VOID
    )
{
    HWND  hListView = WuGetDesktopListView();
    DWORD dwExStyle = 0;

    if (hListView == NULL)
    {
        return FALSE;
    }

    dwExStyle = (DWORD) SendMessageW(
        hListView,
        LVM_GETEXTENDEDLISTVIEWSTYLE,
        0,
        0);

    return dwExStyle & LVS_EX_SNAPTOGRID;
}

WUAPI VOID
WuSetDesktopIconsArrangeByGrid(
    IN BOOL bEnable
    )
{
    if (WuDesktopAreIconsArrangedByGrid() == bEnable)
    {
        return;
    }

    WuDesktopToggleIconArrangement(WU_DESKTOP_ICON_ARRANGE_GRID);
}

WUAPI BOOL
WuDesktopAreIconsVisible(
    VOID
    )
{
    SHELLSTATEA shellState;

    ZeroMemory(&shellState, sizeof(SHELLSTATE));
    
    SHGetSetSettings(&shellState, SSF_HIDEICONS, FALSE);
    
    return shellState.fHideIcons == 0;
}

WUAPI VOID
WuSetDesktopIconsVisible(
    IN BOOL bVisible
    )
{
    if (WuDesktopAreIconsVisible() == bVisible)
    {
        return;
    }

    WuDesktopToggleIconArrangement(WU_DESKTOP_ICON_ARRANGE_DISPLAYICONS);
}
