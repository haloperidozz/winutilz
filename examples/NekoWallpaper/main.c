#include <windows.h>

#include <winutilz.h>

#define CJSON_HIDE_SYMBOLS
#include "cJSON.h"

#define NB_NEKO_ENDPOINT    L"https://nekos.best/api/v2/neko"

#define WALLPAPER_STYLE     WU_WALLPAPER_STYLE_CROPTOFIT

static LPSTR
Utf8DataToAnsiHeapAlloc(
    CONST BYTE* pUtf8Data,
    DWORD       cbSize
    );

INT WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    INT         nShowCmd
    )
{
    BYTE*  pData         = NULL;
    DWORD  cbSize        = 0;
    LPSTR  szResponse    = NULL;
    LPSTR  szImageUrl    = NULL;
    cJSON* pJsonResponse = NULL;
    cJSON* pJsonResults  = NULL;
    cJSON* pJsonResult   = NULL;
    cJSON* pJsonUrl      = NULL;
    INT    iResult       = -1;

    if (WuIsConnectedToInternet() == FALSE)
    {
        return -1;
    }

    if (WuDownloadToMemoryW(NB_NEKO_ENDPOINT, &pData, &cbSize) == FALSE)
    {
        return -1;
    }

    szResponse = Utf8DataToAnsiHeapAlloc(pData, cbSize);

    if (szResponse == NULL)
    {
        goto cleanup;
    }

    /* Example Response: {"results":[{<...>,"url":"..."}]} */

    pJsonResponse = cJSON_Parse(szResponse);

    if (pJsonResponse == NULL)
    {
        goto cleanup;
    }

    pJsonResults = cJSON_GetObjectItem(pJsonResponse, "results");

    if (cJSON_IsArray(pJsonResults) == FALSE)
    {
        goto cleanup;
    }

    pJsonResult = cJSON_GetArrayItem(pJsonResults, 0);

    if (pJsonResult == NULL)
    {
        goto cleanup;
    }

    pJsonUrl = cJSON_GetObjectItem(pJsonResult, "url");

    if (pJsonUrl == NULL || cJSON_IsString(pJsonUrl) == FALSE)
    {
        goto cleanup;
    }

    szImageUrl = pJsonUrl->valuestring;

    if (WuSetWallpaperFromUrlA(szImageUrl, WALLPAPER_STYLE) == FALSE)
    {
        goto cleanup;
    }

    iResult = 0; /* success */
    
cleanup:
    if (pJsonResponse != NULL)
    {
        cJSON_Delete(pJsonResponse);
    }

    if (szResponse != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szResponse);
    }

    if (pData != NULL)
    {
        HeapFree(GetProcessHeap(), 0, pData);
    }

    return iResult;
}

static LPSTR
Utf8DataToAnsiHeapAlloc(
    CONST BYTE* pUtf8Data,
    DWORD       cbSize
    )
{
    INT    cchWide = 0;
    INT    cbWide  = 0;
    LPWSTR szWide  = NULL;
    INT    cbAnsi  = 0;
    LPSTR  szAnsi  = NULL;

    if (pUtf8Data == NULL || cbSize == 0)
    {
        return NULL;
    }

    /* Step 1: UTF-8 --> UTF-16 (WIDE) */

    cchWide = MultiByteToWideChar(
        CP_UTF8,
        0,
        (LPCCH) pUtf8Data,
        cbSize,
        NULL,
        0);

    if (cchWide <= 0)
    {
        return NULL;
    }

    cbWide = (cchWide + 1) * sizeof(WCHAR);

    szWide = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbWide);
    
    if (szWide == NULL)
    {
        return NULL;
    }

    cchWide = MultiByteToWideChar(
        CP_UTF8,
        0,
        (LPCCH) pUtf8Data,
        cbSize,
        szWide,
        cchWide);

    if (cchWide == 0)
    {
        goto cleanup;
    }

    /* Step 2: UTF-16 (WIDE) --> ANSI (MULTIBYTE) */

    cbAnsi = WideCharToMultiByte(
        CP_ACP,
        0,
        szWide,
        -1,
        NULL,
        0,
        NULL,
        NULL);

    if (cbAnsi <= 0)
    {
        goto cleanup;
    }

    szAnsi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbAnsi);

    if (szAnsi == NULL)
    {
        goto cleanup;
    }

    cbAnsi = WideCharToMultiByte(
        CP_ACP,
        0,
        szWide,
        -1,
        szAnsi,
        cbAnsi,
        NULL,
        NULL);

    if (cbAnsi == 0)
    {
        HeapFree(GetProcessHeap(), 0, szAnsi);
        szAnsi = NULL;
    }

cleanup:
    if (szWide != NULL)
    {
        HeapFree(GetProcessHeap(), 0, szWide);
    }
    
    return szAnsi;
}
