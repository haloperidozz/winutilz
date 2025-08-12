/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       capture.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <versionhelpers.h>
#include <dwmapi.h>

#define PW_RENDERFULLCONTENT    0x00000002  /* definition for clang */

WUAPI PWUIMAGEDATA
WuCaptureScreen(
    VOID
    )
{
    return WuCaptureWindow(GetDesktopWindow());
}

WUAPI PWUIMAGEDATA
WuCaptureWindow(
    IN HWND hWnd
    )
{
    RECT         rcWindow;
    SIZE         windowSize;
    HDC          hWindowDC  = NULL;
    HDC          hMemoryDC  = NULL;
    HBITMAP      hbmCapture = NULL;
    HBITMAP      hbmOld     = NULL;
    PWUIMAGEDATA pImageData = NULL;
    BOOL         bCaptured  = FALSE;
    SIZE_T       cbImage    = 0;
    INT          i          = 0;

    if ((NULL == hWnd) || (IsWindow(hWnd) == FALSE))
    {
        return NULL;
    }

    if (GetWindowRect(hWnd, &rcWindow) == FALSE)
    {
        return NULL;
    }

    windowSize.cx = rcWindow.right - rcWindow.left;
    windowSize.cy = rcWindow.bottom - rcWindow.top;

    hWindowDC = GetWindowDC(hWnd);

    if (NULL == hWindowDC)
    {
        return NULL;
    }

    hMemoryDC = CreateCompatibleDC(hWindowDC);
    
    if (NULL == hMemoryDC)
    {
        goto cleanup;
    }

    hbmCapture = CreateCompatibleBitmap(
        hWindowDC,
        windowSize.cx,
        windowSize.cy);

    if (NULL == hbmCapture)
    {
        goto cleanup;
    }

    hbmOld = (HBITMAP) SelectObject(hMemoryDC, hbmCapture);

    if (IsWindows8OrGreater() == TRUE)
    {
        bCaptured = PrintWindow(hWnd, hMemoryDC, PW_RENDERFULLCONTENT);
    }

    if (FALSE == bCaptured)
    {
        if (FAILED(DwmIsCompositionEnabled(&bCaptured)))
        {
            goto dwm_failed;
        }

        if (TRUE == bCaptured)
        {
            bCaptured = PrintWindow(hWnd, hMemoryDC, 0);
        }
    }

dwm_failed:
    if (FALSE == bCaptured)
    {
        bCaptured = BitBlt(
            hMemoryDC, 0, 0,
            windowSize.cx, windowSize.cy,
            hWindowDC, 0, 0,
            SRCCOPY);
    }

    if (FALSE == bCaptured)
    {
        DeleteObject(hbmCapture);
        SelectObject(hMemoryDC, hbmOld);
        hbmCapture = NULL;
        goto cleanup;
    }

    SelectObject(hMemoryDC, hbmOld);

    pImageData = WuExtractImageDataFromHBITMAP(hbmCapture);

    if (IsWindows8OrGreater() == FALSE)
    {
        cbImage = pImageData->uHeight * pImageData->uWidth
            * WU_IMAGEDATA_BYTES_PER_PIXEL;

        for (i = 0; i < cbImage; i += WU_IMAGEDATA_BYTES_PER_PIXEL)
        {
            pImageData->abData[i + 3] = 0xFF; /* alpha offset */
        }
    }

cleanup:
    if (hMemoryDC != NULL)
    {
        DeleteDC(hMemoryDC);
    }

    if (hWindowDC != NULL)
    {
        ReleaseDC(hWnd, hWindowDC);
    }

    return pImageData;
}
