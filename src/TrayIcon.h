#pragma once

#include <windows.h>

class TrayIcon
{
public:
    TrayIcon() = default;
    ~TrayIcon();

    bool create(HWND hwnd, const wchar_t* tooltip);
    void remove();

    void setTooltip(const wchar_t* tooltip);

private:
    HWND hwnd_{};
    HICON icon_{};
    UINT id_{ 1 };
};