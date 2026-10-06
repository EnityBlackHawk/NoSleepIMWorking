# NoSleepIMWorking

Win32 tray application that changes the laptop lid-close action depending on whether an external monitor is active.

## Lid policy

External monitor present:

- AC: Do Nothing
- DC: Do Nothing

No external monitor:

- AC: Sleep
- DC: Sleep

The project currently writes the active power scheme. It does not save and restore a user's previous custom lid settings.

## Build

From a Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Run:

```powershell
.\build\Release\LaptopLidTray.exe
```

No administrator privileges are required for the tray application itself.

## Startup

Right-click the tray icon and enable **Start with Windows**. This writes the executable path to the current user's:

`HKCU\Software\Microsoft\Windows\CurrentVersion\Run`