/***************************************************************************
 * 
 *  Copyright (c) 2025 haloperidozz
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 * 
 *  File:       power.c
 *
 ***************************************************************************/

#include "winutilz.h"

#include "undoc.h"

WUAPI VOID
WuPowerScreenOff(
    VOID
    )
{
    SendMessageW(HWND_TOPMOST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
}

WUAPI VOID
WuPowerLock(
    VOID
    )
{
    LockWorkStation();
}

WUAPI VOID
WuPowerSleepMode(
    VOID
    )
{
    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_SHUTDOWN, TRUE);

    NtInitiatePowerAction(
        PowerActionSleep,
        PowerSystemSleeping1,
        POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED,
        FALSE);
}

WUAPI VOID
WuPowerShutdown(
    VOID
    )
{
    SYSTEM_POWER_CAPABILITIES spc;
    NTSTATUS                  status;

    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_SHUTDOWN, TRUE);

    status = NtPowerInformation(
        SystemPowerCapabilities,
        NULL,
        0,
        &spc,
        sizeof(spc));

    if (NT_SUCCESS(status) == FALSE)
    {
        return;
    }

    ExitWindowsEx(spc.SystemS5 ? EWX_POWEROFF : EWX_SHUTDOWN, 0);
}

WUAPI VOID
WuPowerReboot(
    VOID
    )
{
    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_SHUTDOWN, TRUE);
    
    ExitWindowsEx(EWX_REBOOT, 0);
}

WUAPI VOID
WuPowerLogOff(
    VOID
    )
{
    ExitWindowsEx(EWX_LOGOFF, 0);
}

WUAPI VOID
WuRaiseBlueScreen(
    IN NTSTATUS errorStatus
    )
{
    HARDERROR_RESPONSE response = 0;

    WuCurrentProcessSetPrivilege(WU_PROCESS_PRIVILEGE_SHUTDOWN, TRUE);

    NtRaiseHardError(
        errorStatus,
        0,
        0,
        NULL,
        OptionShutdownSystem,
        &response);
}
