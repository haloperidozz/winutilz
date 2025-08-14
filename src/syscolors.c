 /***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       syscolors.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <strsafe.h>

#include "internal.h"

#define REGISTRY_PATH_COLORS    L"Control Panel\\Colors"

#define COLOR_MAX   31

#define COLORREF_STRING_SIZE_MAX    12              /* "RRR GGG BBB" + '\0' */
#define COLORREF_STRING_FORMAT      L"%u %u %u"

#define COLORS_THEME_SECTION    REGISTRY_PATH_COLORS

typedef struct tagSYSCOLOR {
    INT         iId;
    LPCWSTR     szName;
    COLORREF    crDefaultColor;
} SYSCOLOR;

static SYSCOLOR s_aSysColors[COLOR_MAX] = {
    { COLOR_SCROLLBAR,               L"Scrollbar",             RGB(200, 200, 200) },
    { COLOR_DESKTOP,                 L"Background",            RGB(000, 000, 000) },
    { COLOR_ACTIVECAPTION,           L"ActiveTitle",           RGB(153, 180, 209) },
    { COLOR_INACTIVECAPTION,         L"InactiveTitle",         RGB(191, 205, 219) },
    { COLOR_MENU,                    L"Menu",                  RGB(240, 240, 240) },
    { COLOR_WINDOW,                  L"Window",                RGB(255, 255, 255) },
    { COLOR_WINDOWFRAME,             L"WindowFrame",           RGB(100, 100, 100) },
    { COLOR_MENUTEXT,                L"MenuText",              RGB(000, 000, 000) },
    { COLOR_WINDOWTEXT,              L"WindowText",            RGB(000, 000, 000) },
    { COLOR_CAPTIONTEXT,             L"TitleText",             RGB(000, 000, 000) },
    { COLOR_ACTIVEBORDER,            L"ActiveBorder",          RGB(180, 180, 180) },
    { COLOR_INACTIVEBORDER,          L"InactiveBorder",        RGB(244, 247, 252) },
    { COLOR_APPWORKSPACE,            L"AppWorkspace",          RGB(171, 171, 171) },
    { COLOR_HIGHLIGHT,               L"Hilight",               RGB(000, 120, 215) },
    { COLOR_HIGHLIGHTTEXT,           L"HilightText",           RGB(255, 255, 255) },
    { COLOR_3DFACE,                  L"ButtonFace",            RGB(240, 240, 240) },
    { COLOR_3DSHADOW,                L"ButtonShadow",          RGB(160, 160, 160) },
    { COLOR_GRAYTEXT,                L"GrayText",              RGB(109, 109, 109) },
    { COLOR_BTNTEXT,                 L"ButtonText",            RGB(000, 000, 000) },
    { COLOR_INACTIVECAPTIONTEXT,     L"InactiveTitleText",     RGB(000, 000, 000) },
    { COLOR_3DHILIGHT,               L"ButtonHilight",         RGB(255, 255, 255) },
    { COLOR_3DDKSHADOW,              L"ButtonDkShadow",        RGB(105, 105, 105) },
    { COLOR_3DLIGHT,                 L"ButtonLight",           RGB(227, 227, 227) },
    { COLOR_INFOTEXT,                L"InfoText",              RGB(000, 000, 000) },
    { COLOR_INFOBK,                  L"InfoWindow",            RGB(255, 255, 255) },
    { COLOR_3DALTFACE,               L"ButtonAlternateFace",   RGB(000, 000, 000) },
    { COLOR_HOTLIGHT,                L"HotTrackingColor",      RGB(000, 102, 204) },
    { COLOR_GRADIENTACTIVECAPTION,   L"GradientActiveTitle",   RGB(185, 209, 234) },
    { COLOR_GRADIENTINACTIVECAPTION, L"GradientInactiveTitle", RGB(215, 228, 242) },
    { COLOR_MENUHILIGHT,             L"MenuHilight",           RGB(000, 102, 215) },
    { COLOR_MENUBAR,                 L"MenuBar",               RGB(240, 240, 240) }
};

static BOOL
ColorRefToString(
    OUT LPWSTR      szColor,
    IN  UINT        cchColorSize,
    IN  COLORREF    crColor
    )
{
    HRESULT hResult = S_OK;

    if ((NULL == szColor) || (cchColorSize < COLORREF_STRING_SIZE_MAX))
    {
        return FALSE;
    }

    hResult = StringCchPrintfW(
        szColor,
        cchColorSize,
        COLORREF_STRING_FORMAT,
        (UINT) GetRValue(crColor),      /* R */
        (UINT) GetGValue(crColor),      /* G */
        (UINT) GetBValue(crColor));     /* B */
    
    return SUCCEEDED(hResult);
}

static BOOL
StringToColorRef(
    IN  LPCWSTR     szColor,
    IN  UINT        cchColorSize,
    OUT LPCOLORREF  lpcrColor
    )
{
    WCHAR szBuffer[COLORREF_STRING_SIZE_MAX];
    INT   iCount = 0;
    UINT  r, g, b;

    if ((NULL == szColor) || (NULL == lpcrColor))
    {
        return FALSE;
    }

    if (cchColorSize < COLORREF_STRING_SIZE_MAX)
    {
        return FALSE;
    }

    iCount = _snwscanf(
        szColor,
        cchColorSize,
        COLORREF_STRING_FORMAT,
        &r, &g, &b);
    
    if (iCount != 3)
    {
        return FALSE;
    }

    if ((r > 255) || (g > 255) || (b > 255))
    {
        return FALSE;
    }

    *lpcrColor = RGB(r, g, b);

    return TRUE; 
}

WUAPI BOOL
WuSaveSysColors(
    IN INT              cElements,
    IN CONST INT*       lpaElements,
    IN CONST LPCOLORREF lpaRgbValues
    )
{
    WCHAR szColorString[COLORREF_STRING_SIZE_MAX];
    HKEY  hColorsKey = NULL;
    BOOL  bResult    = FALSE;
    LONG  lResult    = ERROR_SUCCESS;
    INT   i          = 0;

    if ((NULL == lpaElements) || (NULL == lpaRgbValues) || (cElements <= 0))
    {
        return FALSE;
    }

    lResult = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_COLORS,
        0,
        KEY_SET_VALUE,
        &hColorsKey);
    
    if ((lResult != ERROR_SUCCESS) || (NULL == hColorsKey))
    {
        return FALSE;
    }

    if (SetSysColors(cElements, lpaElements, lpaRgbValues) == FALSE)
    {
        goto on_error;
    }

    for (i = 0; i < cElements; ++i)
    {
        if (lpaElements[i] < 0 || lpaElements[i] >= COLOR_MAX)
        {
            goto on_error;
        }

        bResult = ColorRefToString(
            szColorString,
            COLORREF_STRING_SIZE_MAX,
            lpaRgbValues[i]);

        if (FALSE == bResult)
        {
            goto on_error;
        }

        lResult = RegSetValueExW(
            hColorsKey,
            s_aSysColors[lpaElements[i]].szName,
            0,
            REG_SZ,
            (CONST BYTE*) szColorString,
            COLORREF_STRING_SIZE_MAX * sizeof(WCHAR));
        
        if (lResult != ERROR_SUCCESS)
        {
            goto on_error;
        }
    }

    RegCloseKey(hColorsKey);
    return TRUE;

on_error:
    RegCloseKey(hColorsKey);
    return FALSE;
}

WUAPI BOOL
WuResetSysColors(
    IN INT          cElements,
    IN CONST INT*   lpaElements
    )
{
    WCHAR     szColor[COLORREF_STRING_SIZE_MAX];
    COLORREF* acrColors = NULL;
    BOOL      bResult = FALSE;
    INT       i       = 0;

    if ((NULL == lpaElements) || (cElements <= 0))
    {
        return FALSE;
    }

    acrColors = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        cElements * sizeof(COLORREF));
    
    if (acrColors == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < cElements; ++i)
    {
        if (lpaElements[i] < 0 || lpaElements[i] >= COLOR_MAX)
        {
            bResult = FALSE;
            goto cleanup;
        }

        bResult = _WuCurrentThemeGetStringProperty(
            COLORS_THEME_SECTION,
            s_aSysColors[lpaElements[i]].szName,
            szColor,
            COLORREF_STRING_SIZE_MAX);
        
        if (TRUE == bResult)
        {
            bResult = StringToColorRef(
                szColor,
                COLORREF_STRING_SIZE_MAX,
                &(acrColors[i]));
            
            if (TRUE == bResult)
            {
                continue;
            }
        }
        
        acrColors[i] = s_aSysColors[lpaElements[i]].crDefaultColor;
    }

    bResult = WuSaveSysColors(cElements, lpaElements, acrColors);

cleanup:
    HeapFree(GetProcessHeap(), 0, acrColors);
    return bResult;
}
