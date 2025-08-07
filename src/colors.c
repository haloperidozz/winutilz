/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       colors.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <strsafe.h>
#include <ntstatus.h>

#define REGISTRY_PATH_COLORS    L"Control Panel\\Colors"

#define COLOR_MAX   31

static LPCWSTR s_aszColorRegistryNames[COLOR_MAX] = {
    L"Scrollbar",               /* COLOR_SCROLLBAR                 0  */
    L"Background",              /* COLOR_DESKTOP                   1  */ 
    L"ActiveTitle",             /* COLOR_ACTIVECAPTION             2  */ 
    L"InactiveTitle",           /* COLOR_INACTIVECAPTION           3  */ 
    L"Menu",                    /* COLOR_MENU                      4  */ 
    L"Window",                  /* COLOR_WINDOW                    5  */
    L"WindowFrame",             /* COLOR_WINDOWFRAME               6  */ 
    L"MenuText",                /* COLOR_MENUTEXT                  7  */ 
    L"WindowText",              /* COLOR_WINDOWTEXT                8  */ 
    L"TitleText",               /* COLOR_CAPTIONTEXT               9  */ 
    L"ActiveBorder",            /* COLOR_ACTIVEBORDER             10  */ 
    L"InactiveBorder",          /* COLOR_INACTIVEBORDER           11  */ 
    L"AppWorkspace",            /* COLOR_APPWORKSPACE             12  */ 
    L"Hilight",                 /* COLOR_HIGHLIGHT                13  */ 
    L"HilightText",             /* COLOR_HIGHLIGHTTEXT            14  */ 
    L"ButtonFace",              /* COLOR_3DFACE                   15  */ 
    L"ButtonShadow",            /* COLOR_3DSHADOW                 16  */ 
    L"GrayText",                /* COLOR_GRAYTEXT                 17  */ 
    L"ButtonText",              /* COLOR_BTNTEXT                  18  */ 
    L"InactiveTitleText",       /* COLOR_INACTIVECAPTIONTEXT      19  */ 
    L"ButtonHilight",           /* COLOR_3DHILIGHT                20  */ 
    L"ButtonDkShadow",          /* COLOR_3DDKSHADOW               21  */ 
    L"ButtonLight",             /* COLOR_3DLIGHT                  22  */ 
    L"InfoText",                /* COLOR_INFOTEXT                 23  */ 
    L"InfoWindow",              /* COLOR_INFOBK                   24  */ 
    L"ButtonAlternateFace",     /* COLOR_3DALTFACE                25  */ 
    L"HotTrackingColor",        /* COLOR_HOTLIGHT                 26  */ 
    L"GradientActiveTitle",     /* COLOR_GRADIENTACTIVECAPTION    27  */ 
    L"GradientInactiveTitle",   /* COLOR_GRADIENTINACTIVECAPTION  28  */ 
    L"MenuHilight",             /* COLOR_MENUHILIGHT              29  */ 
    L"MenuBar"                  /* COLOR_MENUBAR                  30  */ 
};

WUAPI BOOL
WuSaveSysColors(
    IN INT              cElements,
    IN CONST INT*       lpaElements,
    IN CONST LPCOLORREF lpaRgbValues
    )
{
    WCHAR   szRgbString[4 * 3 + 1];     /* "RRR GGG BBB" */
    HKEY    hColorsKey = NULL;
    LSTATUS lStatus    = STATUS_SUCCESS;
    HRESULT hResult    = S_OK;
    INT     i;

    if (cElements <= 0)
    {
        return FALSE;
    }

    if (lpaElements == NULL || lpaRgbValues == NULL)
    {
        return FALSE;
    }

    lStatus = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_PATH_COLORS,
        0,
        KEY_SET_VALUE,
        &hColorsKey);
    
    if (lStatus != ERROR_SUCCESS || hColorsKey == NULL)
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

        hResult = StringCchPrintfW(
            szRgbString,
            ARRAYSIZE(szRgbString),
            L"%ld %ld %ld",
            GetRValue(lpaRgbValues[i]),     /* R */
            GetGValue(lpaRgbValues[i]),     /* G */
            GetBValue(lpaRgbValues[i]));    /* B */
        
        if (FAILED(hResult))
        {
            goto on_error;
        }

        lStatus = RegSetValueExW(
            hColorsKey,
            s_aszColorRegistryNames[lpaElements[i]],
            0,
            REG_SZ,
            (CONST BYTE*) szRgbString,
            (lstrlenW(szRgbString) + 1) * sizeof(WCHAR));
        
        if (lStatus != ERROR_SUCCESS)
        {
            goto on_error;
        }
    }

    return RegCloseKey(hColorsKey) == ERROR_SUCCESS;

on_error:
    RegCloseKey(hColorsKey);
    return FALSE;
}
