/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       internet.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include <wininet.h>    /* TODO: WinHTTP? */

#include "version.h"
#include "internal.h"

#define USER_AGENT  L"WinUtilz/" PROJECT_VERSION_W

#define BLOCK_SIZE  4096

WUAPI BOOL 
WuIsConnectedToInternet(
    VOID
    )
{
    DWORD dwFlags = 0;

    return InternetGetConnectedState(&dwFlags, 0);
}

WUAPI BOOL
WuHttpGetRequest(
    IN LPCWSTR          szURL,
    IN WRITEDATAPROC    fnWriteDataProc,
    IN LPVOID           pUserData
    )
{
    BYTE      abBlock[BLOCK_SIZE];
    HINTERNET hInternet     = NULL;
    HINTERNET hInternetData = NULL;
    DWORD     cbTotal       = 0;
    DWORD     cbRead        = 0;
    BOOL      bResult       = FALSE;

    if ((NULL == szURL) || (NULL == fnWriteDataProc))
    {
        return FALSE;
    }

    if (WuIsConnectedToInternet() == FALSE)
    {
        return FALSE;
    }

    hInternet = InternetOpenW(
        USER_AGENT,
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0);
    
    if (NULL == hInternet)
    {
        goto cleanup;
    }

    hInternetData = InternetOpenUrlW(
        hInternet,
        szURL,
        NULL,
        0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
        0);
    
    if (NULL == hInternetData)
    {
        goto cleanup;
    }

    while (TRUE)
    {
        bResult = InternetReadFile(
            hInternetData,
            abBlock,
            BLOCK_SIZE,
            &cbRead);

        if (FALSE == bResult)
        {
            goto cleanup;
        }

        if (0 == cbRead)
        {
            break;
        }

        cbTotal += cbRead;

        bResult = fnWriteDataProc(abBlock, cbRead, cbTotal, pUserData);

        if (FALSE == bResult)
        {
            goto cleanup;
        }
    }

    bResult = TRUE;

cleanup:
    if (hInternetData != NULL)
    {
        InternetCloseHandle(hInternetData);
    }

    if (hInternet != NULL)
    {
        InternetCloseHandle(hInternet);
    }

    return bResult;
}

static BOOL
WuDownloadFileW_WriteDataProc(
    IN CONST BYTE*  pBlock,
    IN DWORD        cbRead,
    IN DWORD        cbTotal,
    IN HANDLE       hFile
    )
{
    DWORD dwWritten = 0;
    BOOL  bResult   = FALSE;

    bResult = WriteFile(hFile, pBlock, cbRead, &dwWritten, NULL);

    return ((bResult != FALSE) && (dwWritten == cbRead));
}

WUAPI BOOL
WuDownloadFileW(
    IN LPCWSTR  szURL,
    IN LPCWSTR  szDestPath
    )
{
    WCHAR  szDestPathTemp[MAX_PATH];
    HANDLE hOutputFile = INVALID_HANDLE_VALUE;
    BOOL   bResult     = FALSE;

    if ((NULL == szURL) || (NULL == szDestPath))
    {
        return FALSE;
    }

    bResult = _WuSafeExpandEnvironmentStrings(
        szDestPath,
        szDestPathTemp,
        MAX_PATH);

    if (FALSE == bResult)
    {
        return FALSE;
    }

    hOutputFile = CreateFileW(
        szDestPathTemp,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    if (INVALID_HANDLE_VALUE == hOutputFile)
    {
        goto cleanup;
    }

    bResult = WuHttpGetRequest(
        szURL,
        WuDownloadFileW_WriteDataProc,
        hOutputFile);

cleanup:
    if (INVALID_HANDLE_VALUE != hOutputFile)
    {
        CloseHandle(hOutputFile);
    }

    return bResult;
}

WUAPI BOOL
WuDownloadFileA(
    IN LPCSTR   szURL,
    IN LPCSTR   szDestPath
    )
{
    WCHAR  szwDestPath[MAX_PATH];
    LPWSTR szwURL  = NULL;
    BOOL   bResult = FALSE;

    if ((NULL == szURL) || (NULL == szDestPath))
    {
        return FALSE;
    }

    szwURL = WuAnsiToWideHeapAlloc(szURL);

    if (NULL == szwURL)
    {
        return FALSE;
    }

    if (WuAnsiToWide(szDestPath, szwDestPath, MAX_PATH) == FALSE)
    {
        goto cleanup;
    }

    bResult = WuDownloadFileW(szwURL, szwDestPath);

cleanup:
    if (szwURL != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szwURL);
    }

    return bResult;
}

typedef struct tagMEMORYBUFFER {
    BYTE*  pData;
    DWORD  cbSize;
    DWORD  cbAllocated;
} MEMORYBUFFER, *PMEMORYBUFFER;

static BOOL
WuDownloadToMemoryW_WriteDataProc(
    IN CONST BYTE*      pBlock,
    IN DWORD            cbRead,
    IN DWORD            cbTotal,
    IN PMEMORYBUFFER    pBuffer
    )
{
    BYTE* pNewData   = NULL;
    DWORD cbRequired = 0;
    DWORD cbNewAlloc = 0;

    cbRequired = pBuffer->cbSize + cbRead;

    if (cbRequired > pBuffer->cbAllocated)
    {
        cbNewAlloc = max(cbRequired, pBuffer->cbAllocated * 2);

        pNewData = (BYTE*) HeapReAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            pBuffer->pData,
            cbNewAlloc);
        
        if (NULL == pNewData)
        {
            return FALSE;
        }

        pBuffer->pData = pNewData;
        pBuffer->cbAllocated = cbNewAlloc;
    }

    CopyMemory(pBuffer->pData + pBuffer->cbSize, pBlock, cbRead);
    
    pBuffer->cbSize += cbRead;
    return TRUE;
}

WUAPI BOOL
WuDownloadToMemoryW(
    IN  LPCWSTR szURL,
    OUT BYTE**  ppHeapAllocatedData,
    OUT DWORD*  pcbSize
    )
{
    MEMORYBUFFER buffer;
    BOOL         bResult = FALSE;

    ZeroMemory(&buffer, sizeof(MEMORYBUFFER));

    if ((NULL == szURL) || (NULL == ppHeapAllocatedData) || (NULL == pcbSize))
    {
        return FALSE;
    }

    *ppHeapAllocatedData  = NULL;
    *pcbSize              = 0;

    buffer.pData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BLOCK_SIZE);
    
    if (NULL == buffer.pData)
    {
        return FALSE;
    }
    
    buffer.cbAllocated = BLOCK_SIZE;

    bResult = WuHttpGetRequest(
        szURL,
        (WRITEDATAPROC) WuDownloadToMemoryW_WriteDataProc,
        &buffer);

    if (TRUE == bResult)
    {
        *ppHeapAllocatedData = buffer.pData;
        *pcbSize             = buffer.cbSize;
    }
    else
    {
        if (buffer.pData != NULL)
        {
            HeapFree(GetProcessHeap(), 0, buffer.pData);
        }
    }

    return bResult;
}

WUAPI BOOL
WuDownloadToMemoryA(
    IN  LPCSTR  szURL,
    OUT BYTE**  ppHeapAllocatedData,
    OUT DWORD*  pcbSize
    )
{
    LPWSTR szwURL  = NULL;
    BOOL   bResult = FALSE;

    if (NULL == szURL)
    {
        return FALSE;
    }

    szwURL = WuAnsiToWideHeapAlloc(szURL);

    if (NULL == szwURL)
    {
        return FALSE;
    }

    bResult = WuDownloadToMemoryW(szwURL, ppHeapAllocatedData, pcbSize);

    HeapFree(GetProcessHeap(), 0, szwURL);

    return bResult;
}
