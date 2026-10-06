#include "PowerManager.h"
#include <powrprof.h>

const GUID& PowerManager::subgroupButtons()
{
    static const GUID g{0x4f971e89,0xeebd,0x4455,{0xa8,0xde,0x9e,0x59,0x04,0x0e,0x73,0x47}};
    return g;
}

const GUID& PowerManager::settingLidAction()
{
    static const GUID g{0x5ca83367,0x6e45,0x459f,{0xa2,0x7b,0x47,0x6b,0x1d,0x01,0xc9,0x36}};
    return g;
}

bool PowerManager::setLidAction(LidAction action)
{
    GUID* active = nullptr;
    if (PowerGetActiveScheme(nullptr, &active) != ERROR_SUCCESS)
        return false;

    const DWORD value = static_cast<DWORD>(action);
    const DWORD ac = PowerWriteACValueIndex(nullptr, active, &subgroupButtons(), &settingLidAction(), value);
    const DWORD dc = PowerWriteDCValueIndex(nullptr, active, &subgroupButtons(), &settingLidAction(), value);

    bool ok = ac == ERROR_SUCCESS && dc == ERROR_SUCCESS;
    if (ok)
        ok = PowerSetActiveScheme(nullptr, active) == ERROR_SUCCESS;

    LocalFree(active);
    return ok;
}
