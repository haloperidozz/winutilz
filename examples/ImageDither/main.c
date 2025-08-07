#include <windows.h>

#include <winutilz.h>

#define FILE_FILTER L"PNG Files (*.png)\0*.png\0"                   \
                    L"BMP Files (*.bmp)\0*.bmp\0"                   \
                    L"JPEG Files (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0\0"

static CONST INT g_aiBayerMatrix4x4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

static inline DOUBLE
GetColorLuminance(
    WUCOLOR color
    )
{
    BYTE r = WuGetColorR(color);
    BYTE g = WuGetColorG(color);
    BYTE b = WuGetColorB(color);

    /* https://stackoverflow.com/a/596243 */

    return 0.299 * r + 0.587 * g + 0.114 * b;
}

static VOID
ApplyBayerDitheringToImageData(
    PWUIMAGEDATA    pImageData
    )
{
    WUCOLOR pixel;
    BYTE    bPixelColor;
    DOUBLE  dbLuminance;
    DOUBLE  dbThreshold;
    UINT    x, y;

    if (pImageData == NULL || pImageData->abData == NULL)
    {
        return;
    }

    for (y = 0; y < pImageData->uWidth; ++y)
    {
        for (x = 0; x < pImageData->uWidth; ++x)
        {
            pixel = WuImageDataGetPixel(pImageData, x, y);

            dbLuminance = GetColorLuminance(pixel);
            dbThreshold = g_aiBayerMatrix4x4[y % 4][x % 4] * 255.0 / 16.0;

            /* black or white */
            bPixelColor = (BYTE) (dbLuminance > dbThreshold) ? 0xFF : 0;

            WuImageDataSetPixel(
                pImageData,
                x, y,
                WU_RGB(bPixelColor, bPixelColor, bPixelColor));
        }
    }
}

static BOOL
ShowOpenFileDialog(
    LPWSTR              szFilePath,
    DWORD               cchFilePath
    )
{
    OPENFILENAMEW ofn;

    if (szFilePath == NULL || cchFilePath <= 0)
    {
        return FALSE;
    }

    ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

    ofn.lStructSize  = sizeof(OPENFILENAMEW);
    ofn.lpstrFilter  = FILE_FILTER;
    ofn.lpstrFile    = szFilePath;
    ofn.nMaxFile     = cchFilePath;
    ofn.Flags        = OFN_EXPLORER | OFN_FILEMUSTEXIST;

    return GetOpenFileNameW(&ofn);
}

static BOOL
ShowSaveFileDialog(
    LPWSTR              szFilePath,
    DWORD               cchFilePath,
    WU_IMAGE_FORMAT*    pFormat
    )
{
    OPENFILENAMEW ofn;

    if (szFilePath == NULL || pFormat == NULL || cchFilePath <= 0)
    {
        return FALSE;
    }

    ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

    ofn.lStructSize  = sizeof(OPENFILENAMEW);
    ofn.lpstrFilter  = FILE_FILTER;
    ofn.lpstrFile    = szFilePath;
    ofn.nMaxFile     = cchFilePath;
    ofn.Flags        = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
    ofn.nFilterIndex = 1;

    if (GetSaveFileNameW(&ofn) == FALSE)
    {
        return FALSE;
    }

    switch (ofn.nFilterIndex)
    {
        case 1:
            *pFormat = WU_IMAGE_FORMAT_PNG;
            break;
        case 2:
            *pFormat = WU_IMAGE_FORMAT_BMP;
            break;
        case 3:
            *pFormat = WU_IMAGE_FORMAT_JPEG;
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

INT WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    INT         nShowCmd
    )
{
    WCHAR           szFilePath[MAX_PATH];
    PWUIMAGEDATA    pImageData = NULL;
    WU_IMAGE_FORMAT format     = WU_IMAGE_FORMAT_BMP;

    ZeroMemory(szFilePath, sizeof(szFilePath));

    if (ShowOpenFileDialog(szFilePath, MAX_PATH) == FALSE)
    {
        return -1;
    }

    pImageData = WuLoadImageDataFromFileW(szFilePath);

    if (pImageData == NULL)
    {
        return -1;
    }

    ApplyBayerDitheringToImageData(pImageData);

    if (ShowSaveFileDialog(szFilePath, MAX_PATH, &format) == FALSE)
    {
        return -1;
    }

    if (WuSaveImageDataToFileW(pImageData, szFilePath, format) == FALSE)
    {
        return -1;
    }

    return 0;
}
