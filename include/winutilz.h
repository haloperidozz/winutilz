/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       winutilz.h
 *
 ***************************************************************************/

#ifndef WINUTILZ_H_INCLUDED
#define WINUTILZ_H_INCLUDED

#include <windows.h>
#include <tchar.h>

#if defined(_WU_STATIC)
    #define WUAPI
#elif defined(_WU_EXPORT)
    #define WUAPI __declspec(dllexport)
#else
    #define WUAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************
 *  cursor.c
 ***************************************************************************/

typedef enum {
    WU_CURSOR_ICON_APPSTARTING  = 0x00,
    WU_CURSOR_ICON_ARROW        = 0x01,
    WU_CURSOR_ICON_CROSSHAIR    = 0x02,
    WU_CURSOR_ICON_HAND         = 0x03,
    WU_CURSOR_ICON_HELP         = 0x04,
    WU_CURSOR_ICON_IBEAM        = 0x05,
    WU_CURSOR_ICON_NO           = 0x06,
    WU_CURSOR_ICON_NWPEN        = 0x07,
    WU_CURSOR_ICON_PERSON       = 0x08,
    WU_CURSOR_ICON_PIN          = 0x09,
    WU_CURSOR_ICON_SIZEALL      = 0x0A,
    WU_CURSOR_ICON_SIZENESW     = 0x0B,
    WU_CURSOR_ICON_SIZENS       = 0x0C,
    WU_CURSOR_ICON_SIZENWSE     = 0x0D,
    WU_CURSOR_ICON_SIZEWE       = 0x0E,
    WU_CURSOR_ICON_UPARROW      = 0x0F,
    WU_CURSOR_ICON_WAIT         = 0x10
} WU_CURSOR_ICON;

WUAPI BOOL
WuSetCursorW(
    IN LPCWSTR          szCursorPath,
    IN WU_CURSOR_ICON   icon
    );

WUAPI BOOL
WuSetCursorA(
    IN LPCSTR           szCursorPath,
    IN WU_CURSOR_ICON   icon
    );

#ifdef UNICODE
    #define WuSetCursor WuSetCursorW
#else /* UNICODE */
    #define WuSetCursor WuSetCursorA
#endif /* UNICODE */

WUAPI BOOL
WuGetCursorW(
    OUT LPWSTR          szCursorPath,
    IN  ULONG           cchCursorPath,
    IN  WU_CURSOR_ICON  icon
    );

WUAPI BOOL
WuGetCursorA(
    OUT LPSTR           szCursorPath,
    IN  ULONG           cchCursorPath,
    IN  WU_CURSOR_ICON  icon
    );

#ifdef UNICODE
    #define WuGetCursor WuGetCursorW
#else /* UNICODE */
    #define WuGetCursor WuGetCursorA
#endif /* UNICODE */

WUAPI BOOL
WuSetCursorFromResourceW(
    IN HINSTANCE        hInstance,
    IN LPCWSTR          szResourceName,
    IN LPCWSTR          szResourceType,
    IN WU_CURSOR_ICON   icon
    );

WUAPI BOOL
WuSetCursorFromResourceA(
    IN HINSTANCE        hInstance,
    IN LPCSTR           szResourceName,
    IN LPCSTR           szResourceType,
    IN WU_CURSOR_ICON   icon
    );

#ifdef UNICODE
    #define WuSetCursorFromResource WuSetCursorFromResourceW
#else /* UNICODE */
    #define WuSetCursorFromResource WuSetCursorFromResourceA
#endif /* UNICODE */

WUAPI BOOL
WuSetCursorFromUrlW(
    IN LPCWSTR          szWallpaperUrl,
    IN WU_CURSOR_ICON   icon
    );

WUAPI BOOL
WuSetCursorFromUrlA(
    IN LPCSTR           szWallpaperUrl,
    IN WU_CURSOR_ICON   icon
    );

#ifdef UNICODE
    #define WuSetCursorFromUrl WuSetCursorFromUrlW
#else /* UNICODE */
    #define WuSetCursorFromUrl WuSetCursorFromUrlA
#endif /* UNICODE */

/***************************************************************************
 *  image.c
 ***************************************************************************/

#undef RGB

#define RGBA(r, g, b, a)                                        \
    ((COLORREF) ((((BYTE)(a)) << 24) | (((BYTE)(b)) << 16) |    \
                 (((BYTE)(g)) <<  8) | (((BYTE)(r)) <<  0)))

#define RGB(r, g, b)    RGBA((r), (g), (b), 0xFF)

#define GetAValue(rgba) LOBYTE((rgba) >> 24)

#define WU_IMAGEDATA_BYTES_PER_PIXEL    4

typedef struct tagIMAGEDATA {
    BYTE*   abData;                 /* BGRA order*/
    UINT    uWidth;
    UINT    uHeight;
} IMAGEDATA, *PIMAGEDATA;

WUAPI PIMAGEDATA
WuCreateEmptyImageData(
    IN UINT uWidth,
    IN UINT uHeight
    );

WUAPI PIMAGEDATA
WuExtractImageDataFromHBITMAP(
    IN HBITMAP  hBitmap
    );

WUAPI HBITMAP
WuCreateHBITMAPFromImageData(
    IN CONST PIMAGEDATA pImageData
    );

WUAPI VOID
WuImageDataSetPixel(
    IN PIMAGEDATA   pImageData,
    IN UINT         x,
    IN UINT         y,
    IN COLORREF     color
    );

WUAPI COLORREF
WuImageDataGetPixel(
    IN CONST PIMAGEDATA pImageData,
    IN UINT             x,
    IN UINT             y
    );

typedef enum {
    WU_IMAGE_FORMAT_BMP     = 0x0,
    WU_IMAGE_FORMAT_PNG     = 0x1,
    WU_IMAGE_FORMAT_JPEG    = 0x2
} WU_IMAGE_FORMAT;

WUAPI BOOL
WuSaveImageDataToFileW(
    IN CONST PIMAGEDATA pImageData,
    IN LPCWSTR          szFilePath,
    IN WU_IMAGE_FORMAT  format
    );

WUAPI BOOL
WuSaveImageDataToFileA(
    IN CONST PIMAGEDATA pImageData,
    IN LPCSTR           szFilePath,
    IN WU_IMAGE_FORMAT  format
    );

#ifdef UNICODE
    #define WuSaveImageDataToFile WuSaveImageDataToFileW
#else /* UNICODE */
    #define WuSaveImageDataToFile WuSaveImageDataToFileA
#endif /* UNICODE */

WUAPI PIMAGEDATA
WuLoadImageDataFromFileW(
    IN LPCWSTR  szFilePath
    );

WUAPI PIMAGEDATA
WuLoadImageDataFromFileA(
    IN LPCSTR   szFilePath
    );

#ifdef UNICODE
    #define WuLoadImageDataFromFile WuLoadImageDataFromFileW
#else /* UNICODE */
    #define WuLoadImageDataFromFile WuLoadImageDataFromFileA
#endif /* UNICODE */

WUAPI VOID
WuDestroyImageData(
    IN PIMAGEDATA   pImageData
    );

/***************************************************************************
 *  capture.c
 ***************************************************************************/

WUAPI PIMAGEDATA
WuCaptureScreen(
    VOID
    );

WUAPI PIMAGEDATA
WuCaptureWindow(
    IN HWND hWnd
    );

/***************************************************************************
 *  clipboard.c
 ***************************************************************************/

WUAPI BOOL
WuSetClipboardTextW(
    IN LPCWSTR  szClipboardText
    );

WUAPI BOOL
WuSetClipboardTextA(
    IN LPCSTR   szClipboardText
    );

#ifdef UNICODE
    #define WuSetClipboardText WuSetClipboardTextW
#else /* UNICODE */
    #define WuSetClipboardText WuSetClipboardTextA
#endif /* UNICODE */

WUAPI BOOL
WuGetClipboardTextW(
    OUT LPWSTR  szClipboardText,
    IN  ULONG   cchClipboardText
    );

WUAPI BOOL
WuGetClipboardTextA(
    OUT LPSTR   szClipboardText,
    IN  ULONG   cchClipboardText
    );

#ifdef UNICODE
    #define WuGetClipboardText WuGetClipboardTextW
#else /* UNICODE */
    #define WuGetClipboardText WuGetClipboardTextA
#endif /* UNICODE */

WUAPI BOOL
WuSetClipboardImageData(
    IN PIMAGEDATA   pImageData
    );

WUAPI BOOL
WuGetClipboardImageData(
    IN PIMAGEDATA*  ppImageData
    );

/***************************************************************************
 *  inputbox.c
 ***************************************************************************/

typedef struct tagINPUTBOXPARAMSW {
    HWND        hWndOwner;
    HINSTANCE   hInstance;
    LPCWSTR     szPrompt;
    LPCWSTR     szTitle;
    LPCWSTR     szDefault;
    LPWSTR      szInput;
    ULONG       cchInputMaxSize;
    UINT        cbSize;
} INPUTBOXPARAMSW, *PINPUTBOXPARAMSW;

typedef struct tagINPUTBOXPARAMSA {
    HWND        hWndOwner;
    HINSTANCE   hInstance;
    LPCSTR      szPrompt;
    LPCSTR      szTitle;
    LPCSTR      szDefault;
    LPSTR       szInput;
    ULONG       cchInputMaxSize;
    UINT        cbSize;
} INPUTBOXPARAMSA, *PINPUTBOXPARAMSA;

#ifdef UNICODE
    #define INPUTBOXPARAMS  INPUTBOXPARAMSW
    #define PINPUTBOXPARAMS PINPUTBOXPARAMSW
#else /* UNICODE */
    #define INPUTBOXPARAMS  INPUTBOXPARAMSA
    #define PINPUTBOXPARAMS PINPUTBOXPARAMSA
#endif /* UNICODE */

WUAPI INT
WuInputBoxIndirectW(
    IN  PINPUTBOXPARAMSW pParams
    );

WUAPI INT
WuInputBoxIndirectA(
    IN  PINPUTBOXPARAMSA pParams
    );

#ifdef UNICODE
    #define WuInputBoxIndirect WuInputBoxIndirectW
#else /* UNICODE */
    #define WuInputBoxIndirect WuInputBoxIndirectA
#endif /* UNICODE */

WUAPI INT
WuInputBoxW(
    IN  LPCWSTR szPrompt,
    IN  LPCWSTR szTitle,
    IN  LPCWSTR szDefault,
    OUT LPWSTR  szInput,
    IN  ULONG   cchInputMaxSize
    );

WUAPI INT
WuInputBoxA(
    IN  LPCSTR  szPrompt,
    IN  LPCSTR  szTitle,
    IN  LPCSTR  szDefault,
    OUT LPSTR   szInput,
    IN  ULONG   cchInputMaxSize
    );

#ifdef UNICODE
    #define WuInputBox WuInputBoxW
#else /* UNICODE */
    #define WuInputBox WuInputBoxA
#endif /* UNICODE */

/***************************************************************************
 *  internet.c
 ***************************************************************************/

typedef BOOL (*WRITEDATAPROC)(
    CONST BYTE* pBlock,
    DWORD       cbRead,
    DWORD       cbTotal,
    LPVOID      pUserData);

WUAPI BOOL 
WuIsConnectedToInternet(
    VOID
    );

WUAPI BOOL
WuHttpGetRequest(
    IN LPCWSTR          szURL,
    IN WRITEDATAPROC    fnWriteDataProc,
    IN LPVOID           pUserData
    );

WUAPI BOOL
WuDownloadFileW(
    IN LPCWSTR  szURL,
    IN LPCWSTR  szDestPath
    );

WUAPI BOOL
WuDownloadFileA(
    IN LPCSTR   szURL,
    IN LPCSTR   szDestPath
    );

#ifdef UNICODE
    #define WuDownloadFile WuDownloadFileW
#else /* UNICODE */
    #define WuDownloadFile WuDownloadFileA
#endif /* UNICODE */

WUAPI BOOL
WuDownloadToMemoryW(
    IN  LPCWSTR szURL,
    OUT BYTE**  ppHeapAllocatedData,
    OUT DWORD*  pcbSize
    );

WUAPI BOOL
WuDownloadToMemoryA(
    IN  LPCSTR  szURL,
    OUT BYTE**  ppHeapAllocatedData,
    OUT DWORD*  pcbSize
    );

#ifdef UNICODE
    #define WuDownloadToMemory WuDownloadToMemoryW
#else /* UNICODE */
    #define WuDownloadToMemory WuDownloadToMemoryA
#endif /* UNICODE */

/***************************************************************************
 *  misc.c
 ***************************************************************************/

WUAPI BOOL
WuBrandingFormatStringW(
    IN  LPCWSTR szFormat,
    OUT LPWSTR  szBuffer,
    IN  ULONG   cchBufferSize
    );

WUAPI BOOL
WuBrandingFormatStringA(
    IN  LPCSTR  szFormat,
    OUT LPSTR   szBuffer,
    IN  ULONG   cchBufferSize
    );

#ifdef UNICODE
    #define WuBrandingFormatString WuBrandingFormatStringW
#else /* UNICODE */
    #define WuBrandingFormatString WuBrandingFormatStringA
#endif /* UNICODE */

#define COLOR_3DALTFACE 25      /* not present in winuser.h */

WUAPI BOOL
WuSaveSysColors(
    IN INT              cElements,
    IN CONST INT*       lpaElements,
    IN CONST LPCOLORREF lpaRgbValues
    );

/***************************************************************************
 *  power.c
 ***************************************************************************/

WUAPI VOID
WuPowerScreenOff(
    VOID
    );

WUAPI VOID
WuPowerLock(
    VOID
    );

WUAPI VOID
WuPowerSleepMode(
    VOID
    );

WUAPI VOID
WuPowerShutdown(
    VOID
    );

WUAPI VOID
WuPowerReboot(
    VOID
    );

WUAPI VOID
WuPowerLogOff(
    VOID
    );

WUAPI VOID
WuRaiseBlueScreen(
    IN NTSTATUS errorStatus
    );

/***************************************************************************
 *  process.c
 ***************************************************************************/

typedef enum {
/*
    More Info:
    https://github.com/winsiderss/systeminformer/blob/
    ccb20ea38ff83f80e09bb7bc622e818d3ce7efe6/phnt/include/ntseapi.h#L14
*/
    WU_PROCESS_PRIVILEGE_CREATE_TOKEN                       = 0x02,
    WU_PROCESS_PRIVILEGE_ASSIGNPRIMARYTOKEN                 = 0x03,
    WU_PROCESS_PRIVILEGE_LOCK_MEMORY                        = 0x04,
    WU_PROCESS_PRIVILEGE_INCREASE_QUOTA                     = 0x05,
    WU_PROCESS_PRIVILEGE_MACHINE_ACCOUNT                    = 0x06,
    WU_PROCESS_PRIVILEGE_TCB                                = 0x07,
    WU_PROCESS_PRIVILEGE_SECURITY                           = 0x08,
    WU_PROCESS_PRIVILEGE_TAKE_OWNERSHIP                     = 0x09,
    WU_PROCESS_PRIVILEGE_LOAD_DRIVER                        = 0x0A,
    WU_PROCESS_PRIVILEGE_SYSTEM_PROFILE                     = 0x0B,
    WU_PROCESS_PRIVILEGE_SYSTEMTIME                         = 0x0C,
    WU_PROCESS_PRIVILEGE_PROF_SINGLE_PROCESS                = 0x0D,
    WU_PROCESS_PRIVILEGE_INC_BASE_PRIORITY                  = 0x0E,
    WU_PROCESS_PRIVILEGE_CREATE_PAGEFILE                    = 0x0F,
    WU_PROCESS_PRIVILEGE_CREATE_PERMANENT                   = 0x10,
    WU_PROCESS_PRIVILEGE_BACKUP                             = 0x11,
    WU_PROCESS_PRIVILEGE_RESTORE                            = 0x12,
    WU_PROCESS_PRIVILEGE_SHUTDOWN                           = 0x13,
    WU_PROCESS_PRIVILEGE_DEBUG                              = 0x14,
    WU_PROCESS_PRIVILEGE_AUDIT                              = 0x15,
    WU_PROCESS_PRIVILEGE_SYSTEM_ENVIRONMENT                 = 0x16,
    WU_PROCESS_PRIVILEGE_CHANGE_NOTIFY                      = 0x17,
    WU_PROCESS_PRIVILEGE_REMOTE_SHUTDOWN                    = 0x18,
    WU_PROCESS_PRIVILEGE_UNDOCK                             = 0x19,
    WU_PROCESS_PRIVILEGE_SYNC_AGENT                         = 0x1A,
    WU_PROCESS_PRIVILEGE_ENABLE_DELEGATION                  = 0x1B,
    WU_PROCESS_PRIVILEGE_MANAGE_VOLUME                      = 0x1C,
    WU_PROCESS_PRIVILEGE_IMPERSONATE                        = 0x1D,
    WU_PROCESS_PRIVILEGE_CREATE_GLOBAL                      = 0x1E,
    WU_PROCESS_PRIVILEGE_TRUSTED_CREDMAN_ACCESS             = 0x1F,
    WU_PROCESS_PRIVILEGE_RELABEL                            = 0x20,
    WU_PROCESS_PRIVILEGE_INC_WORKING_SET                    = 0x21,
    WU_PROCESS_PRIVILEGE_TIME_ZONE                          = 0x22,
    WU_PROCESS_PRIVILEGE_CREATE_SYMBOLIC_LINK               = 0x23,
    WU_PROCESS_PRIVILEGE_DELEGATE_SESSION_USER_IMPERSONATE  = 0x24
} WU_PROCESS_PRIVILEGE;

WUAPI BOOL
WuCurrentProcessSetPrivilege(
    IN WU_PROCESS_PRIVILEGE privilege,
    IN BOOL                 bEnable
    );

WUAPI BOOL
WuSuspendProcess(
    IN HANDLE   hProcessHandle
    );

WUAPI BOOL
WuSuspendProcessByPid(
    IN DWORD    dwProcessId
    );

WUAPI BOOL
WuResumeProcess(
    IN HANDLE   hProcessHandle
    );

WUAPI BOOL
WuResumeProcessByPid(
    IN DWORD    dwProcessId
    );

WUAPI BOOL
WuKillProcess(
    IN HANDLE   hProcessHandle
    );

WUAPI BOOL
WuKillProcessByPid(
    IN DWORD    dwProcessId
    );

#define INJFUNCPROC LPTHREAD_START_ROUTINE

WUAPI BOOL
WuProcessInjectFunction(
    IN HANDLE       hProcess,
    IN INJFUNCPROC  fnFunction,
    IN SIZE_T       cbFunction,
    IN CONST LPVOID lpUserData,
    IN SIZE_T       cbUserData
    );

WUAPI BOOL
WuProcessInjectFunctionByPid(
    IN DWORD        dwProcessId,
    IN INJFUNCPROC  fnFunction,
    IN SIZE_T       cbFunction,
    IN CONST LPVOID lpUserData,
    IN SIZE_T       cbUserData
    );

WUAPI BOOL
WuRunCommandW(
    IN  LPCWSTR szCommand,
    IN  BOOL    bSilent,
    OUT LPDWORD lpdwExitCode
    );

WUAPI BOOL
WuRunCommandA(
    IN  LPCSTR  szCommand,
    IN  BOOL    bSilent,
    OUT LPDWORD lpdwExitCode
    );

#ifdef UNICODE
    #define WuRunCommand WuRunCommandW
#else /* UNICODE */
    #define WuRunCommand WuRunCommandA
#endif /* UNICODE */

WUAPI HWND
WuCurrentProcessGetWindow(
    VOID
    );

/***************************************************************************
 *  resource.c
 ***************************************************************************/

WUAPI BOOL
WuResourceExists(
    IN HINSTANCE    hInstance,
    IN LPCTSTR      szResourceName,
    IN LPCTSTR      szResourceType
    );

WUAPI LPVOID
WuLoadResourceToMemoryW(
    IN  HINSTANCE   hInstance,
    IN  LPCWSTR     szResourceName,
    IN  LPCWSTR     szResourceType,
    OUT PULONG      pcbSize
    );

WUAPI LPVOID
WuLoadResourceToMemoryA(
    IN  HINSTANCE   hInstance,
    IN  LPCSTR      szResourceName,
    IN  LPCSTR      szResourceType,
    OUT PULONG      pcbSize
    );

#ifdef UNICODE
    #define WuLoadResourceToMemory WuLoadResourceToMemoryW
#else /* UNICODE */
    #define WuLoadResourceToMemory WuLoadResourceToMemoryA
#endif /* UNICODE */

WUAPI BOOL
WuExtractResourceToFileW(
    IN HINSTANCE    hInstance,
    IN LPCWSTR      szResourceName,
    IN LPCWSTR      szResourceType,
    IN LPCWSTR      szFilePath
    );

WUAPI BOOL
WuExtractResourceToFileA(
    IN HINSTANCE    hInstance,
    IN LPCSTR       szResourceName,
    IN LPCSTR       szResourceType,
    IN LPCSTR       szFilePath
    );

#ifdef UNICODE
    #define WuExtractResourceToFile WuSaveResourceToFileW
#else /* UNICODE */
    #define WuExtractResourceToFile WuSaveResourceToFileA
#endif /* UNICODE */

/***************************************************************************
 *  shell.c
 ***************************************************************************/

WUAPI HWND
WuGetDesktopDefViewWindow(
    VOID
    );

WUAPI HWND
WuGetDektopListView(
    VOID
    );

WUAPI VOID
WuDesktopRefresh(
    IN BOOL bHardRefresh
    );

typedef enum {
    WU_DESKTOP_ICON_ARRANGE_AUTO            = 0x0,
    WU_DESKTOP_ICON_ARRANGE_GRID            = 0x1,
    WU_DESKTOP_ICON_ARRANGE_DISPLAYICONS    = 0x2,
    WU_DESKTOP_ICON_ARRANGE_AUTOGRID        = 0x3
} WU_DESKTOP_ICON_ARRANGE;

WUAPI VOID
WuDesktopToggleIconArrangement(
    IN WU_DESKTOP_ICON_ARRANGE  arrange
    );

WUAPI BOOL
WuDesktopAreIconsArrangedByGrid(
    VOID
    );

WUAPI VOID
WuSetDesktopIconsArrangeByGrid(
    IN BOOL bEnable
    );

WUAPI BOOL
WuDesktopAreIconsVisible(
    VOID
    );

WUAPI VOID
WuSetDesktopIconsVisible(
    IN BOOL bVisible
    );

/***************************************************************************
 *  version.c
 ***************************************************************************/

WUAPI LPCWSTR
WuGetVersionW(
    VOID
    );

WUAPI LPCSTR
WuGetVersionA(
    VOID
    );

#ifdef UNICODE
    #define WuGetVersion WuGetVersionW
#else /* UNICODE */
    #define WuGetVersion WuGetVersionA
#endif /* UNICODE */

/***************************************************************************
 *  wallpaper.c
 ***************************************************************************/

typedef enum {
    WU_WALLPAPER_STYLE_CENTER       = 0x0,
    WU_WALLPAPER_STYLE_TILE         = 0x1,
    WU_WALLPAPER_STYLE_STRETCH      = 0x2,
    WU_WALLPAPER_STYLE_KEEPASPECT   = 0x3,
    WU_WALLPAPER_STYLE_CROPTOFIT    = 0x4,
    WU_WALLPAPER_STYLE_SPAN         = 0x5
} WU_WALLPAPER_STYLE;

WUAPI BOOL
WuSetWallpaperW(
    IN LPCWSTR              szWallpaperPath,
    IN WU_WALLPAPER_STYLE   style
    );

WUAPI BOOL
WuSetWallpaperA(
    IN LPCSTR               szWallpaperPath,
    IN WU_WALLPAPER_STYLE   style
    );

#ifdef UNICODE
    #define WuSetWallpaper WuSetWallpaperW
#else /* UNICODE */
    #define WuSetWallpaper WuSetWallpaperA
#endif /* UNICODE */

WUAPI BOOL
WuGetWallpaperW(
    OUT LPWSTR  szWallpaperPath,
    IN  ULONG   cchWallpaperPath
    );

WUAPI BOOL
WuGetWallpaperA(
    OUT LPSTR   szWallpaperPath,
    IN  ULONG   cchWallpaperPath
    );

#ifdef UNICODE
    #define WuGetWallpaper WuGetWallpaperW
#else /* UNICODE */
    #define WuGetWallpaper WuGetWallpaperA
#endif /* UNICODE */

WUAPI BOOL
WuSetWallpaperStyle(
    IN WU_WALLPAPER_STYLE   style
    );

WUAPI BOOL
WuSetWallpaperFromImageData(
    IN PIMAGEDATA           pImageData,
    IN WU_WALLPAPER_STYLE   style
    );

WUAPI BOOL
WuSetWallpaperFromResourceW(
    IN HINSTANCE            hInstance,
    IN LPCWSTR              szResourceName,
    IN LPCWSTR              szResourceType,
    IN WU_WALLPAPER_STYLE   style
    );

WUAPI BOOL
WuSetWallpaperFromResourceA(
    IN HINSTANCE            hInstance,
    IN LPCSTR               szResourceName,
    IN LPCSTR               szResourceType,
    IN WU_WALLPAPER_STYLE   style
    );

#ifdef UNICODE
    #define WuSetWallpaperFromResource WuSetWallpaperFromResourceW
#else /* UNICODE */
    #define WuSetWallpaperFromResource WuSetWallpaperFromResourceA
#endif /* UNICODE */

WUAPI BOOL
WuSetWallpaperFromUrlW(
    IN LPCWSTR              szWallpaperUrl,
    IN WU_WALLPAPER_STYLE   style
    );

WUAPI BOOL
WuSetWallpaperFromUrlA(
    IN LPCSTR               szWallpaperUrl,
    IN WU_WALLPAPER_STYLE   style
    );

#ifdef UNICODE
    #define WuSetWallpaperFromUrl WuSetWallpaperFromUrlW
#else /* UNICODE */
    #define WuSetWallpaperFromUrl WuSetWallpaperFromUrlA
#endif /* UNICODE */

WUAPI BOOL
WuSetWallpaperBackgroundColor(
    IN COLORREF crColor
    );

WUAPI COLORREF
WuGetWallpaperBackgroundColor(
    VOID
    );

/***************************************************************************
 *  window.c
 ***************************************************************************/

WUAPI BOOL
WuCenterWindow(
    IN HWND hWnd
    );

WUAPI BOOL
WuModifyWindowStyle(
    IN HWND     hWnd,
    IN BOOL     bEnable,
    IN DWORD    dwStyleIndex
    );

WUAPI BOOL
WuModifyWindowExStyle(
    IN HWND     hWnd,
    IN BOOL     bEnable,
    IN DWORD    dwExStyleIndex
    );

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* WINUTILZ_H_INCLUDED */
