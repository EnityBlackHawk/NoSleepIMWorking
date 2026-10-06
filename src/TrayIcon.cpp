#include "TrayIcon.h"

#include <shellapi.h>

namespace
{
    constexpr UINT kTrayMessage = WM_APP + 1;
}

TrayIcon::~TrayIcon()
{
    remove();
}

bool TrayIcon::create(HWND hwnd, const wchar_t* tooltip)
{
    hwnd_ = hwnd;

    icon_ = LoadIconW(nullptr, IDI_APPLICATION);

    if (!icon_)
        return false;

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = id_;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = kTrayMessage;
    nid.hIcon = icon_;

    wcsncpy_s(nid.szTip, tooltip, _TRUNCATE);

    return Shell_NotifyIconW(NIM_ADD, &nid) == TRUE;
}

void TrayIcon::remove()
{
    if (!hwnd_)
        return;

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = id_;

    Shell_NotifyIconW(NIM_DELETE, &nid);

    hwnd_ = nullptr;
}

void TrayIcon::setTooltip(const wchar_t* tooltip)
{
    if (!hwnd_)
        return;

    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = id_;
    nid.uFlags = NIF_TIP;

    wcsncpy_s(nid.szTip, tooltip, _TRUNCATE);

    Shell_NotifyIconW(NIM_MODIFY, &nid);
}