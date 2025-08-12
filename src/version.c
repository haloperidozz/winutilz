/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       version.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include "version.h"

#define INIT_WUVERSION(verstr) {        \
        (verstr),                       \
        PROJECT_VERSION_MAJOR,          \
        PROJECT_VERSION_MINOR,          \
        PROJECT_VERSION_PATCH           \
    }

static WUVERSIONA g_wuVersionA = INIT_WUVERSION(PROJECT_VERSION_A);
static WUVERSIONW g_wuVersionW = INIT_WUVERSION(PROJECT_VERSION_W);

WUAPI CONST PWUVERSIONW
WuGetVersionExW(
    LPVOID  lpReserved
    )
{
    return &g_wuVersionW;
}

WUAPI CONST PWUVERSIONA
WuGetVersionExA(
    LPVOID  lpReserved
    )
{
    return &g_wuVersionA;
}

WUAPI LPCWSTR
WuGetVersionW(
    VOID
    )
{
    return g_wuVersionW.szVersion;
}

WUAPI LPCSTR
WuGetVersionA(
    VOID
    )
{
    return g_wuVersionA.szVersion;
}
