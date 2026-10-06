#include "Application.h"

#include <dbt.h>
#include <shellapi.h>
#include <strsafe.h>
#include <cstdarg>


namespace
{
// TODO: Get from CMake
constexpr wchar_t kWindowClass[] = L"NoSleepIMWorkingWindow";
constexpr wchar_t kWindowTitle[] = L"No Sleep, I'm working";
constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kRunValue[] = L"NoSleepIMWorking";
constexpr wchar_t kMutexName[] = L"Local\\NoSleepIMWorking.SingleInstance";
constexpr UINT kPeriodicRefreshMs = 1500;
}

namespace
{
    constexpr GUID kGuidDevInterfaceMonitor =
    {
        0xe6f07b5f,
        0xee97,
        0x4a90,
        {
            0xb0,
            0x76,
            0x33,
            0xf5,
            0x7b,
            0xf4,
            0xea,
            0xa7
        }
    };
}

Application::Application(HINSTANCE instance)
    : instance_(instance)
{
}

Application::~Application()
{
    unregisterDeviceNotifications();
    if (refreshTimer_ != 0 && hwnd_)
        KillTimer(hwnd_, TIMER_REFRESH);
    tray_.remove();
    if (hwnd_)
        DestroyWindow(hwnd_);
}

int Application::run()
{
    HANDLE mutex = CreateMutexW(nullptr, TRUE, kMutexName);
    if (!mutex)
        return 1;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(mutex);
        return 0;
    }

    if (!createWindow())
    {
        CloseHandle(mutex);
        return 1;
    }

    tray_.create(hwnd_, L"Laptop Lid");
    startupEnabled_ = isStartWithWindowsEnabled();
    registerDeviceNotifications();
    refreshMonitorState();
    refreshTimer_ = SetTimer(hwnd_, TIMER_REFRESH, kPeriodicRefreshMs, nullptr);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CloseHandle(mutex);
    return static_cast<int>(msg.wParam);
}

bool Application::createWindow()
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &Application::WndProc;
    wc.hInstance = instance_;
    wc.lpszClassName = kWindowClass;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kWindowClass, kWindowTitle, 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void Application::registerDeviceNotifications()
{
    DEV_BROADCAST_DEVICEINTERFACE_W filter{};
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = kGuidDevInterfaceMonitor;

    monitorNotification_ = RegisterDeviceNotificationW(
        hwnd_, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    if (!monitorNotification_)
        writeLog(L"RegisterDeviceNotificationW failed: %lu", GetLastError());
    else
        writeLog(L"Registered monitor device notifications.");
}

void Application::unregisterDeviceNotifications()
{
    if (monitorNotification_)
    {
        UnregisterDeviceNotification(monitorNotification_);
        monitorNotification_ = nullptr;
    }
}

void Application::refreshMonitorState()
{
    const bool detected = monitorDetector_.hasExternalMonitor();

    if (detected != externalMonitor_)
    {
        externalMonitor_ = detected;
        applyLidPolicy(externalMonitor_);
    }

    wchar_t tooltip[128]{};
    StringCchPrintfW(
        tooltip, 128, L"NoSleepIMWorking - %s",
        externalMonitor_ ? L"External monitor detected" : L"Internal display only");
    tray_.setTooltip(tooltip);

    writeLog(L"Monitor state: external=%s", externalMonitor_ ? L"true" : L"false");
}

void Application::applyLidPolicy(bool externalMonitor)
{
    const auto action = externalMonitor
        ? PowerManager::LidAction::DoNothing
        : PowerManager::LidAction::Sleep;

    if (!powerManager_.setLidAction(action))
    {
        writeLog(L"Failed to change lid action.");
        return;
    }

    writeLog(L"Lid action changed to: %s",
             externalMonitor ? L"Do Nothing" : L"Sleep");
}

void Application::showContextMenu()
{
    POINT pt{};
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();
    if (!menu)
        return;

    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0,
                externalMonitor_ ? L"External monitor: detected"
                                 : L"External monitor: not detected");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_REFRESH, L"Refresh");
    AppendMenuW(menu, MF_STRING | (startupEnabled_ ? MF_CHECKED : 0),
                ID_TRAY_STARTUP, L"Start with Windows");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hwnd_);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                   pt.x, pt.y, 0, hwnd_, nullptr);
    PostMessageW(hwnd_, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

void Application::setStartWithWindows(bool enabled)
{
    HKEY key{};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
    {
        writeLog(L"Could not open startup registry key.");
        return;
    }

    if (enabled)
    {
        wchar_t path[MAX_PATH]{};
        if (GetModuleFileNameW(nullptr, path, MAX_PATH) != 0)
        {
            const DWORD bytes = static_cast<DWORD>((wcslen(path) + 1) * sizeof(wchar_t));
            RegSetValueExW(key, kRunValue, 0, REG_SZ,
                           reinterpret_cast<const BYTE*>(path), bytes);
        }
    }
    else
    {
        RegDeleteValueW(key, kRunValue);
    }

    RegCloseKey(key);
    startupEnabled_ = enabled;
}

bool Application::isStartWithWindowsEnabled() const
{
    HKEY key{};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return false;

    wchar_t path[MAX_PATH]{};
    DWORD type = 0;
    DWORD size = sizeof(path);
    const LONG result = RegQueryValueExW(key, kRunValue, nullptr, &type,
                                         reinterpret_cast<BYTE*>(path), &size);
    RegCloseKey(key);
    return result == ERROR_SUCCESS && type == REG_SZ;
}

void Application::writeLog(const wchar_t* format, ...)
{
    wchar_t buffer[1024]{};
    va_list args;
    va_start(args, format);
    StringCchVPrintfW(buffer, 1024, format, args);
    va_end(args);
    OutputDebugStringW(buffer);
    OutputDebugStringW(L"\r\n");
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Application* self = reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE)
    {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<Application*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }

    return self ? self->handleMessage(hwnd, msg, wParam, lParam)
                : DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Application::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DEVICECHANGE:
            if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
                SetTimer(hwnd, TIMER_REFRESH, 300, nullptr);
            return TRUE;

        case WM_DISPLAYCHANGE:
            SetTimer(hwnd, TIMER_REFRESH, 300, nullptr);
            return 0;

        case WM_TIMER:
            if (wParam == TIMER_REFRESH)
            {
                KillTimer(hwnd, TIMER_REFRESH);
                refreshMonitorState();
            }
            return 0;

        case WM_POWERBROADCAST:
            if (wParam == PBT_APMRESUMEAUTOMATIC ||
                wParam == PBT_APMRESUMESUSPEND ||
                wParam == PBT_POWERSETTINGCHANGE)
                SetTimer(hwnd, TIMER_REFRESH, 500, nullptr);
            return TRUE;

        case WM_TRAY:
            if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
            {
                showContextMenu();
                return 0;
            }
            if (lParam == WM_LBUTTONDBLCLK)
            {
                refreshMonitorState();
                return 0;
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_TRAY_REFRESH:
                    refreshMonitorState();
                    return 0;
                case ID_TRAY_STARTUP:
                    setStartWithWindows(!startupEnabled_);
                    return 0;
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
