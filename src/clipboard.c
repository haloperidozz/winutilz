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

#include "strconv.h"
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
    INT  i               = 0;
    BOOL bResult         = FALSE;

    if (fnClipboardWorker == NULL)
    {
        return FALSE;
    }

    hWnd = WuCurrentProcessGetWindow();

    if (hWnd == NULL)
    {
        hWnd = SHCreateWorkerWindowW(NULL, NULL, 0, 0, NULL, 0);
        bIsWorkerWindow = TRUE;
    }

    if (hWnd == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < CLIPBOARD_RETRY_COUNT; ++i)
    {
        if (OpenClipboard(hWnd) == TRUE)
        {
            goto opened;
        }

        Sleep(CLIPBOARD_RETRY_DELAY_MS);
    }

    return FALSE;

opened:
    bResult = fnClipboardWorker(lpUserData);

    CloseClipboard();

    if (bIsWorkerWindow == TRUE)
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

    cbClipboardText = (lstrlenW(szClipboardText) + 1) * sizeof(WCHAR);

    hClipboardData = GlobalAlloc(GMEM_MOVEABLE, cbClipboardText);

    if (hClipboardData == NULL)
    {
        return FALSE;
    }

    szClipboardData = (LPWSTR) GlobalLock(hClipboardData);
    
    if (szClipboardData == NULL)
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
    if (szClipboardText == NULL)
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

    szwClipboardText = AnsiToWideHeapAlloc(szClipboardText);

    if (szwClipboardText == NULL)
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

    hClipboardData = GetClipboardData(CF_UNICODETEXT);
    
    if (hClipboardData == NULL)
    {
        return FALSE;
    }

    szClipboardData = (LPCWSTR) GlobalLock(hClipboardData);

    if (szClipboardData == NULL)
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

    if (szClipboardText == NULL || cchClipboardText == 0)
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

    if (szwClipboardText == NULL)
    {
        return FALSE;
    }

    bResult = WuGetClipboardTextW(
        szwClipboardText,
        cchClipboardText);

    if (bResult == FALSE)
    {
        goto cleanup;
    }

    bResult = WideToAnsi(
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
    IN PIMAGEDATA   pImageData
    )
{
    BITMAPINFOHEADER bmiHeader;
    SIZE_T           cbDibSize      = 0;
    HGLOBAL          hClipboardData = NULL;
    BYTE*            pClipboardData = NULL;

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

    if (hClipboardData == NULL)
    {
        return FALSE;
    }

    pClipboardData = (BYTE*) GlobalLock(hClipboardData);
    
    if (pClipboardData == NULL)
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
    IN PIMAGEDATA   pImageData
    )
{
    if (pImageData == NULL)
    {
        return FALSE;
    }

    return WithOpenedClipboard(
        (CLIPBOARDWORKPROC) WuSetClipboardImageData_WorkerProc,
        (LPVOID) pImageData);
}

static BOOL
WuGetClipboardImageData_WorkerProc(
    IN PIMAGEDATA*  ppImageData
    )
{
    HBITMAP    hClipboardData = NULL;
    PIMAGEDATA pImageData     = NULL;

    hClipboardData = (HBITMAP) GetClipboardData(CF_BITMAP);
    
    if (hClipboardData == NULL)
    {
        return FALSE;
    }

    pImageData = WuExtractImageDataFromHBITMAP(hClipboardData);

    if (pImageData == NULL)
    {
        return FALSE;
    }

    *ppImageData = pImageData;

    return TRUE;
}

WUAPI BOOL
WuGetClipboardImageData(
    IN PIMAGEDATA*  ppImageData
    )
{
    if (ppImageData == NULL)
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
