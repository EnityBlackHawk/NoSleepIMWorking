#pragma once

#include <windows.h>

class PowerManager
{
public:
    enum class LidAction : DWORD { DoNothing = 0, Sleep = 1, Hibernate = 2, Shutdown = 3 };
    bool setLidAction(LidAction action);

private:
    static const GUID& subgroupButtons();
    static const GUID& settingLidAction();
};
