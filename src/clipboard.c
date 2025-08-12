/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       clipboard.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <strsafe.h>

#include "undoc.h"

#define CLIPBOARD_RETRY_COUNT       5
#define CLIPBOARD_RETRY_DELAY_MS    10

typedef BOOL (*CLIPBOARDWORKPROC)(LPVOID);

static BOOL
WithOpenedClipboard(
    IN CLIPBOARDWORKPROC    fnClipboardWorker,
    IN LPVOID               lpUserData
    )
{
    HWND hWnd            = NULL;
    BOOL bIsWorkerWindow = FALSE;
    BOOL bOpened         = FALSE;
    INT  i               = 0;
    BOOL bResult         = FALSE;

    if (NULL == fnClipboardWorker)
    {
        return FALSE;
    }

    hWnd = WuCurrentProcessGetWindow();

    if (NULL == hWnd)
    {
        hWnd = SHCreateWorkerWindowW(NULL, NULL, 0, 0, NULL, 0);
        bIsWorkerWindow = TRUE;
    }

    if (NULL == hWnd)
    {
        return FALSE;
    }

    for (i = 0; i < CLIPBOARD_RETRY_COUNT; ++i)
    {
        if (OpenClipboard(hWnd) == TRUE)
        {
            bOpened = TRUE;
            break;
        }

        Sleep(CLIPBOARD_RETRY_DELAY_MS);
    }

    if (TRUE == bOpened)
    {
        bResult = fnClipboardWorker(lpUserData);
        CloseClipboard();
    }

    if (TRUE == bIsWorkerWindow)
    {
        DestroyWindow(hWnd);
    }

    return bResult;
}

static BOOL
WuSetClipboardTextW_WorkerProc(
    IN LPCWSTR  szClipboardText
    )
{
    HGLOBAL hClipboardData  = NULL;
    SIZE_T  cbClipboardText = 0;
    LPWSTR  szClipboardData = NULL;

    if (NULL == szClipboardText)
    {
        return FALSE;
    }

    cbClipboardText = (lstrlenW(szClipboardText) + 1) * sizeof(WCHAR);

    hClipboardData = GlobalAlloc(GMEM_MOVEABLE, cbClipboardText);

    if (NULL == hClipboardData)
    {
        return FALSE;
    }

    szClipboardData = (LPWSTR) GlobalLock(hClipboardData);
    
    if (NULL == szClipboardData)
    {
        GlobalFree(hClipboardData);
        return FALSE;
    }

    CopyMemory(szClipboardData, szClipboardText, cbClipboardText);

    GlobalUnlock(hClipboardData);

    if (EmptyClipboard() == FALSE)
    {
        GlobalFree(hClipboardData);
        return FALSE;
    }

    if (SetClipboardData(CF_UNICODETEXT, hClipboardData) == FALSE)
    {
        GlobalFree(hClipboardData);
        return FALSE;
    }

    return TRUE;
}

WUAPI BOOL
WuSetClipboardTextW(
    IN LPCWSTR  szClipboardText
    )
{
    if (NULL == szClipboardText)
    {
        return FALSE;
    }

    return WithOpenedClipboard(
        (CLIPBOARDWORKPROC) WuSetClipboardTextW_WorkerProc,
        (LPVOID) szClipboardText);
}

WUAPI BOOL
WuSetClipboardTextA(
    IN LPCSTR   szClipboardText
    )
{
    LPWSTR szwClipboardText = NULL;
    BOOL   bResult          = FALSE;

    szwClipboardText = WuAnsiToWideHeapAlloc(szClipboardText);

    if (NULL == szwClipboardText)
    {
        return FALSE;
    }

    bResult = WuSetClipboardTextW(szwClipboardText);

    HeapFree(GetProcessHeap(), 0, szwClipboardText);

    return bResult;
}

typedef struct tagCLIPBOARDTEXT {
    ULONG   cchClipboardText;
    LPWSTR  szClipboardText;
} CLIPBOARDTEXT, *PCLIPBOARDTEXT;

static BOOL
WuGetClipboardTextW_WorkerProc(
    IN PCLIPBOARDTEXT   pClipboardText
    )
{
    HANDLE  hClipboardData  = NULL;
    LPCWSTR szClipboardData = NULL;
    HRESULT hResult         = S_OK;

    if (NULL == pClipboardText)
    {
        return FALSE;
    }

    hClipboardData = GetClipboardData(CF_UNICODETEXT);
    
    if (NULL == hClipboardData)
    {
        return FALSE;
    }

    szClipboardData = (LPCWSTR) GlobalLock(hClipboardData);

    if (NULL == szClipboardData)
    {
        return FALSE;
    }

    hResult = StringCchCopyW(
        pClipboardText->szClipboardText,
        pClipboardText->cchClipboardText,
        szClipboardData);

    GlobalUnlock(hClipboardData);

    return SUCCEEDED(hResult);
}

WUAPI BOOL
WuGetClipboardTextW(
    OUT LPWSTR  szClipboardText,
    IN  ULONG   cchClipboardText
    )
{
    CLIPBOARDTEXT clipboardText;

    if ((NULL == szClipboardText) || (0 == cchClipboardText))
    {
        return FALSE;
    }

    if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE)
    {
        return FALSE;
    }

    clipboardText.szClipboardText  = szClipboardText;
    clipboardText.cchClipboardText = cchClipboardText;

    return WithOpenedClipboard(
        (CLIPBOARDWORKPROC) WuGetClipboardTextW_WorkerProc,
        &clipboardText);
}

WUAPI BOOL
WuGetClipboardTextA(
    OUT LPSTR   szClipboardText,
    IN  ULONG   cchClipboardText
    )
{
    LPWSTR szwClipboardText = NULL;
    BOOL   bResult          = FALSE;

    szwClipboardText = (LPWSTR) HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        cchClipboardText * sizeof(WCHAR));

    if (NULL == szwClipboardText)
    {
        return FALSE;
    }

    bResult = WuGetClipboardTextW(
        szwClipboardText,
        cchClipboardText);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    bResult = WuWideToAnsi(
        szwClipboardText,
        szClipboardText,
        cchClipboardText);

cleanup:
    if (szwClipboardText != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwClipboardText);
    }
    
    return bResult;
}

static BOOL
WuSetClipboardImageData_WorkerProc(
    IN PWUIMAGEDATA   pImageData
    )
{
    BITMAPINFOHEADER bmiHeader;
    SIZE_T           cbDibSize      = 0;
    HGLOBAL          hClipboardData = NULL;
    BYTE*            pClipboardData = NULL;

    if (NULL == pImageData)
    {
        return FALSE;
    }

    ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));

    bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth       = pImageData->uWidth;
    bmiHeader.biHeight      = -(LONG) pImageData->uHeight;
    bmiHeader.biPlanes      = 1;
    bmiHeader.biBitCount    = 32;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage   = pImageData->uWidth * pImageData->uHeight
        * WU_IMAGEDATA_BYTES_PER_PIXEL;

    cbDibSize = sizeof(BITMAPINFOHEADER) + bmiHeader.biSizeImage;

    hClipboardData = GlobalAlloc(GMEM_MOVEABLE, cbDibSize);

    if (NULL == hClipboardData)
    {
        return FALSE;
    }

    pClipboardData = (BYTE*) GlobalLock(hClipboardData);
    
    if (NULL == pClipboardData)
    {
        GlobalFree(hClipboardData);
        return FALSE;
    }

    CopyMemory(pClipboardData, &bmiHeader, bmiHeader.biSize);

    CopyMemory(
        pClipboardData + bmiHeader.biSize,
        pImageData->abData,
        bmiHeader.biSizeImage);

    GlobalUnlock(hClipboardData);

    if (EmptyClipboard() == FALSE)
    {
        GlobalFree(hClipboardData);
        return FALSE;
    }

    if (SetClipboardData(CF_DIB, hClipboardData) == FALSE)
    {
        GlobalFree(hClipboardData);
        return FALSE;
    }

    return TRUE;
}

WUAPI BOOL
WuSetClipboardImageData(
    IN PWUIMAGEDATA pImageData
    )
{
    if (NULL == pImageData)
    {
        return FALSE;
    }

    return WithOpenedClipboard(
        (CLIPBOARDWORKPROC) WuSetClipboardImageData_WorkerProc,
        (LPVOID) pImageData);
}

static BOOL
WuGetClipboardImageData_WorkerProc(
    IN PWUIMAGEDATA*    ppImageData
    )
{
    HBITMAP      hClipboardData = NULL;
    PWUIMAGEDATA pImageData     = NULL;

    hClipboardData = (HBITMAP) GetClipboardData(CF_BITMAP);
    
    if (NULL == hClipboardData)
    {
        return FALSE;
    }

    pImageData = WuExtractImageDataFromHBITMAP(hClipboardData);

    if (NULL == pImageData)
    {
        return FALSE;
    }

    *ppImageData = pImageData;

    return TRUE;
}

WUAPI BOOL
WuGetClipboardImageData(
    IN PWUIMAGEDATA*    ppImageData
    )
{
    if (NULL == ppImageData)
    {
        return FALSE;
    }

    if (IsClipboardFormatAvailable(CF_BITMAP) == FALSE)
    {
        return FALSE;
    }

    return WithOpenedClipboard(
        (CLIPBOARDWORKPROC) WuGetClipboardImageData_WorkerProc,
        (LPVOID) ppImageData);
}
