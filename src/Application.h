#pragma once

#include <windows.h>
#include "MonitorDetector.h"
#include "PowerManager.h"
#include "TrayIcon.h"

class Application
{
public:
    explicit Application(HINSTANCE instance);
    ~Application();

    int run();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT handleMessage(HWND, UINT, WPARAM, LPARAM);

    bool createWindow();
    void registerDeviceNotifications();
    void unregisterDeviceNotifications();
    void refreshMonitorState();
    void applyLidPolicy(bool externalMonitor);
    void showContextMenu();
    void setStartWithWindows(bool enabled);
    bool isStartWithWindowsEnabled() const;
    void writeLog(const wchar_t* format, ...);

    HINSTANCE instance_{};
    HWND hwnd_{};
    HDEVNOTIFY monitorNotification_{};
    UINT_PTR refreshTimer_{0};
    bool externalMonitor_{false};
    bool startupEnabled_{false};

    MonitorDetector monitorDetector_;
    PowerManager powerManager_;
    TrayIcon tray_;

    static constexpr UINT WM_TRAY = WM_APP + 1;
    static constexpr UINT_PTR TIMER_REFRESH = 1;
    static constexpr UINT ID_TRAY_REFRESH = 1001;
    static constexpr UINT ID_TRAY_STARTUP = 1002;
    static constexpr UINT ID_TRAY_EXIT = 1003;
};
