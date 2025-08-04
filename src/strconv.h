/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       strconv.h
 *
 ***************************************************************************/

#ifndef STRCONV_H_INCLUDED
#define STRCONV_H_INCLUDED

#include <windows.h>

BOOL
AnsiToWide(
    IN  LPCSTR  szAnsi,
    OUT LPWSTR  szWideBuffer,
    IN  ULONG   cchWideBufferMaxSize
    );

LPWSTR
AnsiToWideHeapAlloc(
    IN LPCSTR   szAnsi
    );

BOOL
WideToAnsi(
    IN  LPCWSTR szWide,
    OUT LPSTR   szAnsiBuffer,
    IN  ULONG   cchAnsiBufferMaxSize
    );

LPSTR
WideToAnsiHeapAlloc(
    IN LPCWSTR  szWide
    );

LPWSTR
AnsiResParamToWideHeapAlloc(
    IN LPCSTR   szAnsi
    );

VOID
SafeResParamHeapFree(
    IN LPVOID   lpParam
    );

#endif /* STRCONV_H_INCLUDED */
