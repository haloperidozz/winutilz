/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       image.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <versionhelpers.h>
#include <wincodec.h>

#include "internal.h"

#define CLEANUP_IF_FAILED(hResult)                  \
    if (FAILED(hResult))                            \
    {                                               \
        goto cleanup;                               \
    }

#define SAFE_RELEASE_COM_OBJECT(hComObj)            \
    if (NULL != (hComObj))                          \
    {                                               \
        (hComObj)->lpVtbl->Release(hComObj);        \
        (hComObj) = NULL;                           \
    }

#define WU_IMAGE_FORMAT_MAX 3

static CONST GUID* g_aImageFormatToWicGuid[WU_IMAGE_FORMAT_MAX] = {
    &GUID_ContainerFormatBmp,   /* WU_IMAGE_FORMAT_BMP  */
    &GUID_ContainerFormatPng,   /* WU_IMAGE_FORMAT_PNG  */
    &GUID_ContainerFormatJpeg   /* WU_IMAGE_FORMAT_JPEG */
};

WUAPI PWUIMAGEDATA
WuCreateEmptyImageData(
    IN UINT uWidth,
    IN UINT uHeight
    )
{
    PWUIMAGEDATA pImageData = NULL;

    if ((uWidth == 0) || (uHeight == 0))
    {
        return NULL;
    }
 
    pImageData = HeapAlloc(GetProcessHeap(), 0, sizeof(WUIMAGEDATA));

    if (NULL == pImageData)
    {
        return NULL;
    }

    pImageData->uWidth  = uWidth;
    pImageData->uHeight = uHeight;

    pImageData->abData = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        uWidth * uHeight * WU_IMAGEDATA_BYTES_PER_PIXEL);
    
    if (NULL == pImageData->abData)
    {
        HeapFree(GetProcessHeap(), 0, pImageData);
        return NULL;
    }

    return pImageData;
}

WUAPI PWUIMAGEDATA
WuExtractImageDataFromHBITMAP(
    IN HBITMAP  hBitmap
    )
{
    BITMAP       bm;
    BITMAPINFO   bmi;
    HDC          hDC        = NULL;
    PWUIMAGEDATA pImageData = NULL;
    DWORD        dwLines    = 0;

    if (NULL == hBitmap)
    {
        return NULL;
    }

    ZeroMemory(&bm, sizeof(BITMAP));

    if (GetObject(hBitmap, sizeof(BITMAP), &bm) == 0)
    {
        return NULL;
    }

    pImageData = WuCreateEmptyImageData(bm.bmWidth, bm.bmHeight);

    if (NULL == pImageData)
    {
        return NULL;
    }

    hDC = GetDC(NULL);

    if (NULL == hDC)
    {
        WuDestroyImageData(pImageData);
        return NULL;
    }

    ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = pImageData->uWidth;
    bmi.bmiHeader.biHeight      = -((INT) pImageData->uHeight);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    dwLines = GetDIBits(
        hDC,
        hBitmap,
        0,
        pImageData->uHeight,
        pImageData->abData,
        &bmi,
        DIB_RGB_COLORS);

    if ((0 == dwLines) || (ERROR_INVALID_PARAMETER == dwLines))
    {
        ReleaseDC(NULL, hDC);
        WuDestroyImageData(pImageData);
        return NULL;
    }

    ReleaseDC(NULL, hDC);

    return pImageData;
}

WUAPI HBITMAP
WuCreateHBITMAPFromImageData(
    IN CONST PWUIMAGEDATA   pImageData
    )
{
    BITMAPINFO bmi;
    HDC        hDC     = NULL;
    HBITMAP    hBitmap = NULL; 
    VOID*      pBits   = NULL;
    SIZE_T     cbImage = 0;

    if (NULL == pImageData)
    {
        return NULL;
    }

    hDC = GetDC(NULL);

    if (NULL == hDC)
    {
        return NULL;
    }

    ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = pImageData->uWidth;
    bmi.bmiHeader.biHeight      = -((INT) pImageData->uHeight);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

    if ((NULL == hBitmap) || (NULL == pBits))
    {
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    cbImage = pImageData->uWidth * pImageData->uHeight
        * WU_IMAGEDATA_BYTES_PER_PIXEL;

    CopyMemory(pBits, pImageData->abData, cbImage);

    ReleaseDC(NULL, hDC);

    return hBitmap;
}

WUAPI BOOL
WuSaveImageDataToFileW(
    IN CONST PWUIMAGEDATA   pImageData,
    IN LPCWSTR              szFilePath,
    IN WU_IMAGE_FORMAT      format
    )
{
    WCHAR                  szTempPath[MAX_PATH];
    WICPixelFormatGUID     pixelFormat;
    IWICImagingFactory*    pWicFactory = NULL;
    IWICStream*            pWicStream  = NULL;
    IWICBitmapEncoder*     pWicEncoder = NULL;
    IWICBitmapFrameEncode* pWicFrame   = NULL;
    IStream*               pStream     = NULL;
    CONST GUID*            pGuidFormat = NULL;
    BOOL                   bResult     = FALSE;
    BOOL                   bNeedUninit = FALSE; 
    HRESULT                hResult     = S_OK;

    if ((NULL == pImageData) || (NULL == szFilePath))
    {
        return FALSE;
    }

    if (format >= WU_IMAGE_FORMAT_MAX)
    {
        return FALSE;
    }

    /* WIC minimum supported client: Windows XP with SP2 */
    if (IsWindowsXPSP2OrGreater() == FALSE)
    {
        return FALSE;
    }

    hResult = CoInitialize(NULL);

    bNeedUninit = SUCCEEDED(hResult) && (hResult == S_OK);

    if (FAILED(hResult))
    {
        return FALSE;
    }

    hResult = CoCreateInstance(
        &CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,
        (LPVOID*) &pWicFactory);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFactory->lpVtbl->CreateStream(pWicFactory, &pWicStream);

    CLEANUP_IF_FAILED(hResult);

    bResult = _WuSafeExpandEnvironmentStrings(
        szFilePath,
        szTempPath,
        MAX_PATH);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    hResult = pWicStream->lpVtbl->InitializeFromFilename(
        pWicStream,
        szTempPath,
        GENERIC_WRITE);
    
    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFactory->lpVtbl->CreateEncoder(
        pWicFactory,
        g_aImageFormatToWicGuid[format],
        NULL,
        &pWicEncoder);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicStream->lpVtbl->QueryInterface(
        pWicStream,
        &IID_IStream,
        (VOID**) &pStream);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicEncoder->lpVtbl->Initialize(
        pWicEncoder,
        pStream,
        WICBitmapEncoderNoCache);
    
    CLEANUP_IF_FAILED(hResult);

    hResult = pWicEncoder->lpVtbl->CreateNewFrame(
        pWicEncoder,
        &pWicFrame,
        NULL);
    
    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFrame->lpVtbl->Initialize(pWicFrame, NULL);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFrame->lpVtbl->SetSize(
        pWicFrame,
        pImageData->uWidth,
        pImageData->uHeight);
    
    CLEANUP_IF_FAILED(hResult);

    pixelFormat = GUID_WICPixelFormat32bppBGRA;

    hResult = pWicFrame->lpVtbl->SetPixelFormat(
        pWicFrame,
        &pixelFormat);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFrame->lpVtbl->WritePixels(
        pWicFrame,
        pImageData->uHeight,
        pImageData->uWidth * WU_IMAGEDATA_BYTES_PER_PIXEL,
        pImageData->uWidth * pImageData->uHeight
            * WU_IMAGEDATA_BYTES_PER_PIXEL,
        pImageData->abData);
    
    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFrame->lpVtbl->Commit(pWicFrame);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicEncoder->lpVtbl->Commit(pWicEncoder);

cleanup:
    SAFE_RELEASE_COM_OBJECT(pWicFrame);
    SAFE_RELEASE_COM_OBJECT(pWicEncoder);
    SAFE_RELEASE_COM_OBJECT(pWicStream);
    SAFE_RELEASE_COM_OBJECT(pWicFactory);

    if (TRUE == bNeedUninit)
    {
        CoUninitialize();
    }

    return SUCCEEDED(hResult);
}

WUAPI BOOL
WuSaveImageDataToFileA(
    IN CONST PWUIMAGEDATA   pImageData,
    IN LPCSTR               szFilePath,
    IN WU_IMAGE_FORMAT      format
    )
{
    WCHAR szwFilePath[MAX_PATH];

    if ((NULL == pImageData) || (NULL == szFilePath))
    {
        return FALSE;
    }

    if (WuAnsiToWide(szFilePath, szwFilePath, MAX_PATH) == FALSE)
    {
        return FALSE;
    }

    return WuSaveImageDataToFileW(pImageData, szwFilePath, format);
}

WUAPI PWUIMAGEDATA
WuLoadImageDataFromFileW(
    IN LPCWSTR  szFilePath
    )
{
    WCHAR                  szTempPath[MAX_PATH];
    IWICImagingFactory*    pWicFactory   = NULL;
    IWICBitmapDecoder*     pWicDecoder   = NULL;
    IWICBitmapFrameDecode* pWicFrame     = NULL;
    IWICFormatConverter*   pWicConverter = NULL;
    IWICBitmapSource*      pWicBitmapSrc = NULL;
    PWUIMAGEDATA           pImageData    = NULL;
    UINT                   uWidth        = 0;
    UINT                   uHeight       = 0;
    BOOL                   bResult       = FALSE;
    BOOL                   bNeedUninit   = FALSE; 
    HRESULT                hResult       = S_OK;

    if (NULL == szFilePath)
    {
        return FALSE;
    }

    /* WIC minimum supported client: Windows XP with SP2 */
    if (IsWindowsXPSP2OrGreater() == FALSE)
    {
        return FALSE;
    }

    hResult = CoInitialize(NULL);

    bNeedUninit = (SUCCEEDED(hResult) && (hResult == S_OK));

    if (FAILED(hResult))
    {
        return NULL;
    }

    hResult = CoCreateInstance(
        &CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory,
        (LPVOID*) &pWicFactory);

    CLEANUP_IF_FAILED(hResult);

    bResult = _WuSafeExpandEnvironmentStrings(
        szFilePath,
        szTempPath,
        MAX_PATH);

    if (FALSE == bResult)
    {
        goto cleanup;
    }

    hResult = pWicFactory->lpVtbl->CreateDecoderFromFilename(
        pWicFactory,
        szTempPath,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pWicDecoder);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicDecoder->lpVtbl->GetFrame(pWicDecoder, 0, &pWicFrame);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFrame->lpVtbl->GetSize(pWicFrame, &uWidth, &uHeight);

    CLEANUP_IF_FAILED(hResult);

    pImageData = WuCreateEmptyImageData(uWidth, uHeight);

    if (NULL == pImageData)
    {
        goto cleanup;
    }

    hResult = pWicFactory->lpVtbl->CreateFormatConverter(
        pWicFactory,
        &pWicConverter);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicFrame->lpVtbl->QueryInterface(
        pWicFrame,
        &IID_IWICBitmapSource,
        (VOID**) &pWicBitmapSrc);
    
    CLEANUP_IF_FAILED(hResult);

    hResult = pWicConverter->lpVtbl->Initialize(
        pWicConverter,
        pWicBitmapSrc,
        &GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.0,
        WICBitmapPaletteTypeCustom);

    CLEANUP_IF_FAILED(hResult);

    hResult = pWicConverter->lpVtbl->CopyPixels(
        pWicConverter,
        NULL,
        pImageData->uWidth * WU_IMAGEDATA_BYTES_PER_PIXEL,
        pImageData->uWidth * pImageData->uHeight
            * WU_IMAGEDATA_BYTES_PER_PIXEL,
        pImageData->abData);

cleanup:
    if ((pImageData != NULL) && FAILED(hResult))
    {
        WuDestroyImageData(pImageData);
    }

    SAFE_RELEASE_COM_OBJECT(pWicConverter);
    SAFE_RELEASE_COM_OBJECT(pWicFrame);
    SAFE_RELEASE_COM_OBJECT(pWicDecoder);
    SAFE_RELEASE_COM_OBJECT(pWicFactory);

    if (bNeedUninit == TRUE)
    {
        CoUninitialize();
    }

    return pImageData;
}

WUAPI PWUIMAGEDATA
WuLoadImageDataFromFileA(
    IN LPCSTR   szFilePath
    )
{
    WCHAR szwFilePath[MAX_PATH];

    if (NULL == szFilePath)
    {
        return FALSE;
    }

    if (WuAnsiToWide(szFilePath, szwFilePath, MAX_PATH) == FALSE)
    {
        return FALSE;
    }

    return WuLoadImageDataFromFileW(szwFilePath);
}

WUAPI VOID
WuDestroyImageData(
    IN PWUIMAGEDATA pImageData
    )
{
    if (NULL == pImageData)
    {
        return;
    }

    if (pImageData->abData != NULL)
    {
        HeapFree(GetProcessHeap(), 0, pImageData->abData);
    }

    HeapFree(GetProcessHeap(), 0, pImageData);
}
