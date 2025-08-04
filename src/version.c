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

WUAPI LPCWSTR
WuGetVersionW(
    VOID
    )
{
    return PROJECT_VERSION_W;
}

WUAPI LPCSTR
WuGetVersionA(
    VOID
    )
{
    return PROJECT_VERSION_A;
}
