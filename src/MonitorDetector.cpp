#include "MonitorDetector.h"

#include <windows.h>

#include <cstdio>
#include <string>
#include <vector>

namespace
{

    const wchar_t* technologyToString(
        DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY technology)
    {
        switch (technology)
        {
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER:
            return L"Other";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15:
            return L"VGA";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO:
            return L"S-Video";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO:
            return L"Composite";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO:
            return L"Component";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI:
            return L"DVI";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI:
            return L"HDMI";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS:
            return L"LVDS";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN:
            return L"D-JPN";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI:
            return L"SDI";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL:
            return L"DisplayPort External";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED:
            return L"DisplayPort Embedded";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL:
            return L"UDI External";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED:
            return L"UDI Embedded";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE:
            return L"SDTV Dongle";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_MIRACAST:
            return L"Miracast";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_WIRED:
            return L"Indirect Wired";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_VIRTUAL:
            return L"Indirect Virtual";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_USB_TUNNEL:
            return L"DisplayPort USB Tunnel";

        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL:
            return L"Internal";

        default:
            return L"Unknown";
        }
    }


    bool isInternalDisplay(
        DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY technology)
    {
        switch (technology)
        {
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS:
            return true;

        default:
            return false;
        }
    }


    bool isExternalDisplay(
        DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY technology)
    {
        switch (technology)
        {
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_MIRACAST:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_WIRED:
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_USB_TUNNEL:
            return true;

        default:
            return false;
        }
    }

    void logTargetInfo(
        const DISPLAYCONFIG_PATH_INFO& path)
    {
        DISPLAYCONFIG_TARGET_DEVICE_NAME target{};

        target.header.type =
            DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;

        target.header.size =
            sizeof(target);

        target.header.adapterId =
            path.targetInfo.adapterId;

        target.header.id =
            path.targetInfo.id;

        const LONG result =
            DisplayConfigGetDeviceInfo(&target.header);

        if (result != ERROR_SUCCESS)
        {
            wchar_t buffer[256];

            swprintf_s(
                buffer,
                L"[MonitorDetector] "
                L"Display target %u - "
                L"DisplayConfigGetDeviceInfo failed: %ld\n",
                path.targetInfo.id,
                result);

            OutputDebugStringW(buffer);

            return;
        }

        wchar_t buffer[1024];

        swprintf_s(
            buffer,
            L"[MonitorDetector] "
            L"Monitor='%s' "
            L"Technology='%s' "
            L"EDID=%04X:%04X\n",
            target.monitorFriendlyDeviceName,
            technologyToString(target.outputTechnology),
            target.edidManufactureId,
            target.edidProductCodeId);

        OutputDebugStringW(buffer);
    }

}


bool MonitorDetector::hasExternalMonitor() const
{
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;

    LONG result =
        GetDisplayConfigBufferSizes(
            QDC_ONLY_ACTIVE_PATHS,
            &pathCount,
            &modeCount);

    if (result != ERROR_SUCCESS)
    {
        wchar_t buffer[256];

        swprintf_s(
            buffer,
            L"[MonitorDetector] "
            L"GetDisplayConfigBufferSizes failed: %ld\n",
            result);

        OutputDebugStringW(buffer);

        return false;
    }

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    result =
        QueryDisplayConfig(
            QDC_ONLY_ACTIVE_PATHS,
            &pathCount,
            paths.data(),
            &modeCount,
            modes.data(),
            nullptr);

    if (result != ERROR_SUCCESS)
    {
        wchar_t buffer[256];

        swprintf_s(
            buffer,
            L"[MonitorDetector] "
            L"QueryDisplayConfig failed: %ld\n",
            result);

        OutputDebugStringW(buffer);

        return false;
    }

    bool externalMonitor = false;

    for (UINT32 i = 0; i < pathCount; ++i)
    {
        const auto technology =
            paths[i].targetInfo.outputTechnology;

        logTargetInfo(paths[i]);

        wchar_t buffer[256];

        swprintf_s(
            buffer,
            L"[MonitorDetector] "
            L"Path %u: technology=%s (%u)\n",
            i,
            technologyToString(technology),
            static_cast<unsigned>(technology));

        OutputDebugStringW(buffer);

        if (isExternalDisplay(technology))
        {
            externalMonitor = true;
        }
    }

    wchar_t buffer[256];

    swprintf_s(
        buffer,
        L"[MonitorDetector] Result: external monitor = %s\n",
        externalMonitor ? L"YES" : L"NO");

    OutputDebugStringW(buffer);

    return externalMonitor;
}