/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       inputbox.c
 *
 ***************************************************************************/

#include "winutilz.h"

#define IDC_OK      IDOK
#define IDC_CANCEL  IDCANCEL
#define IDC_PROMPT  1001
#define IDC_INPUT   1000

static CONST BYTE g_abRawInputBoxDialogTemplate[196] = {
    0x40, 0x00, 0xc8, 0x90, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xee, 0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x00, 0x6e, 0x00, 0x70, 0x00, 0x75, 0x00, 0x74, 0x00, 0x42,
    0x00, 0x6f, 0x00, 0x78, 0x00, 0x00, 0x00, 0x08, 0x00, 0x4d, 0x00,
    0x53, 0x00, 0x20, 0x00, 0x53, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6c,
    0x00, 0x6c, 0x00, 0x20, 0x00, 0x44, 0x00, 0x6c, 0x00, 0x67, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x81, 0x50, 0x00, 0x00, 0x00, 0x00, 0x07,
    0x00, 0x3a, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xe8, 0x03, 0xff, 0xff,
    0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
    0x50, 0x00, 0x00, 0x00, 0x00, 0xb5, 0x00, 0x06, 0x00, 0x32, 0x00,
    0x0e, 0x00, 0x01, 0x00, 0xff, 0xff, 0x80, 0x00, 0x4f, 0x00, 0x4b,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x50,
    0x00, 0x00, 0x00, 0x00, 0xb5, 0x00, 0x18, 0x00, 0x32, 0x00, 0x0e,
    0x00, 0x02, 0x00, 0xff, 0xff, 0x80, 0x00, 0x43, 0x00, 0x61, 0x00,
    0x6e, 0x00, 0x63, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x00, 0x02, 0x50, 0x00, 0x00, 0x00, 0x00,
    0x07, 0x00, 0x06, 0x00, 0x9f, 0x00, 0x30, 0x00, 0xe9, 0x03, 0xff,
    0xff, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static BOOL
InputBox_OnInitDialog(
    IN HWND             hDlg,
    IN PINPUTBOXPARAMSW pParams
    )
{
    WuCenterWindow(hDlg);

    if (pParams->szTitle != NULL)
    {
        SetWindowTextW(hDlg, pParams->szTitle);
    }

    if (pParams->szPrompt != NULL)
    {
        SetWindowTextW(GetDlgItem(hDlg, IDC_PROMPT), pParams->szPrompt);
    }
    
    if (pParams->szDefault != NULL)
    {
        SetWindowTextW(GetDlgItem(hDlg, IDC_INPUT), pParams->szDefault);
    }

    SetForegroundWindow(hDlg);
    return TRUE;
}

static BOOL
InputBox_OnPressOkButton(
    IN HWND             hDlg,
    IN PINPUTBOXPARAMSW pParams
    )
{
    UINT cchCopied = 0;

    if (pParams->szInput == NULL)
    {
        return EndDialog(hDlg, IDOK) != FALSE;
    }

    SetLastError(ERROR_SUCCESS);

    cchCopied = GetDlgItemTextW(
        hDlg,
        IDC_INPUT,
        pParams->szInput,
        pParams->cchInputMaxSize);

    if (cchCopied == 0)
    {
        if (GetLastError() != ERROR_SUCCESS)
        {
            return EndDialog(hDlg, 0) != FALSE;
        }
    }

    return EndDialog(hDlg, IDOK) != FALSE;
}

static BOOL
InputBox_OnPressCancelButton(
    IN HWND             hDlg,
    IN PINPUTBOXPARAMSW pParams
    )
{
    return EndDialog(hDlg, IDCANCEL) != FALSE;
}

static BOOL
InputBox_OnCommand(
    IN HWND             hDlg,
    IN PINPUTBOXPARAMSW pParams,
    IN WORD             wControlId,
    IN WORD             wNotifyCode,
    IN HWND             hControl
    )
{
    switch (wControlId)
    {
        case IDC_OK:
            return InputBox_OnPressOkButton(hDlg, pParams);
        case IDC_CANCEL:
            return InputBox_OnPressCancelButton(hDlg, pParams);
    }

    return FALSE;
}

static BOOL
InputBox_OnClose(
    IN HWND             hDlg,
    IN PINPUTBOXPARAMSW pParams
    )
{
    return EndDialog(hDlg, IDC_CANCEL) != FALSE;
}

static INT_PTR CALLBACK
InputBox_Proc(
    IN HWND     hDlg,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )
{
    PINPUTBOXPARAMSW pParams = NULL;

    pParams = (PINPUTBOXPARAMSW) GetWindowLongPtrW(hDlg, GWLP_USERDATA);

    if (uMsg == WM_INITDIALOG)
    {
        pParams = (PINPUTBOXPARAMSW) lParam;

        SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR) pParams);
    }

    if (pParams == NULL)
    {
        return FALSE;
    }

    switch (uMsg) 
    {
        case WM_INITDIALOG:
            return InputBox_OnInitDialog(hDlg, pParams);
        
        case WM_COMMAND:
            return InputBox_OnCommand(
                hDlg,
                pParams,
                LOWORD(wParam),
                HIWORD(wParam),
                (HWND) lParam);
        
        case WM_CLOSE:
            return InputBox_OnClose(hDlg, pParams);
    }

    return FALSE;
}

WUAPI INT
WuInputBoxIndirectW(
    IN  PINPUTBOXPARAMSW pParams
    )
{
    if (pParams == NULL || pParams->cbSize != sizeof(INPUTBOXPARAMSW))
    {
        return 0;
    }

    return (INT) DialogBoxIndirectParamW(
        pParams->hInstance,
        (LPCDLGTEMPLATEW) g_abRawInputBoxDialogTemplate,
        pParams->hWndOwner,
        InputBox_Proc,
        (LPARAM) pParams);
}

WUAPI INT
WuInputBoxIndirectA(
    IN  PINPUTBOXPARAMSA pParams
    )
{
    INPUTBOXPARAMSW wParams;
    LPWSTR          szPrompt        = NULL;
    LPWSTR          szTitle         = NULL;
    LPWSTR          szDefault       = NULL;
    LPWSTR          szInput         = NULL;
    INT             iInputBoxResult = 0;
    BOOL            bResult         = FALSE;
    
    if (pParams == NULL || pParams->cbSize != sizeof(INPUTBOXPARAMSA))
    {
        return 0;
    }

    ZeroMemory(&wParams, sizeof(INPUTBOXPARAMSW));

    szPrompt  = WuAnsiToWideHeapAlloc(pParams->szPrompt);
    szTitle   = WuAnsiToWideHeapAlloc(pParams->szTitle);
    szDefault = WuAnsiToWideHeapAlloc(pParams->szDefault);

    if (pParams->szInput != NULL)
    {
        szInput = HeapAlloc(
            GetProcessHeap(),
            0,
            pParams->cchInputMaxSize * sizeof(WCHAR));
        
        if (szInput == NULL)
        {
            goto cleanup;
        }
    }
    
    wParams.cbSize          = sizeof(INPUTBOXPARAMSW);
    wParams.hWndOwner       = pParams->hWndOwner;
    wParams.hInstance       = pParams->hInstance;
    wParams.szPrompt        = szPrompt;
    wParams.szTitle         = szTitle;
    wParams.szDefault       = szDefault;
    wParams.szInput         = szInput;
    wParams.cchInputMaxSize = pParams->cchInputMaxSize;
    
    iInputBoxResult = WuInputBoxIndirectW(&wParams);

    if (szInput != NULL && szInput != NULL)
    {
        bResult = WuWideToAnsi(
            szInput,
            pParams->szInput,
            pParams->cchInputMaxSize);
        
        if (bResult == FALSE)
        {
            iInputBoxResult = 0;
        }
    }

cleanup:
    if (szInput != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szInput);
    }

    if (szDefault != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szDefault);
    }

    if (szTitle != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szTitle);
    }

    if (szPrompt != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szPrompt);
    }

    return iInputBoxResult;
}

WUAPI INT
WuInputBoxW(
    IN  LPCWSTR szPrompt,
    IN  LPCWSTR szTitle,
    IN  LPCWSTR szDefault,
    OUT LPWSTR  szInput,
    IN  ULONG   cchInputMaxSize
    )
{
    INPUTBOXPARAMSW params;

    ZeroMemory(&params, sizeof(INPUTBOXPARAMSW));

    params.cbSize          = sizeof(INPUTBOXPARAMSW);
    params.hWndOwner       = NULL;
    params.hInstance       = NULL;
    params.szPrompt        = szPrompt;
    params.szTitle         = szTitle;
    params.szDefault       = szDefault;
    params.szInput         = szInput;
    params.cchInputMaxSize = cchInputMaxSize;

    return WuInputBoxIndirectW(&params);
}

WUAPI INT
WuInputBoxA(
    IN  LPCSTR  szPrompt,
    IN  LPCSTR  szTitle,
    IN  LPCSTR  szDefault,
    OUT LPSTR   szInput,
    IN  ULONG   cchInputMaxSize
    )
{
    INPUTBOXPARAMSA params;

    ZeroMemory(&params, sizeof(INPUTBOXPARAMSA));

    params.cbSize          = sizeof(INPUTBOXPARAMSA);
    params.hWndOwner       = NULL;
    params.hInstance       = NULL;
    params.szPrompt        = szPrompt;
    params.szTitle         = szTitle;
    params.szDefault       = szDefault;
    params.szInput         = szInput;
    params.cchInputMaxSize = cchInputMaxSize;

    return WuInputBoxIndirectA(&params);
}
