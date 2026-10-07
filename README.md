# NoSleepIMWorking

**Automatically prevent your Windows laptop from going to sleep when the lid is closed and an external monitor is connected.**

![Windows](https://img.shields.io/badge/Windows-10%20%7C%2011-0078D4?logo=windows&logoColor=white)

If you use a laptop as a desktop computer with an external monitor, you've probably encountered this Windows behavior:

> You close the laptop lid → Windows puts the laptop to sleep → your external monitor goes dark.

Windows allows you to configure the lid action to **"Do nothing"**, but then the laptop also stays awake when you close the lid **without** an external monitor.

**NoSleepIMWorking** solves this automatically.

---

## 💻 Compatibility

- ✅ Windows 10
- ✅ Windows 11
- ✅ x64 systems

The application uses native Windows APIs available on both Windows 10 and Windows 11.

---

## ✨ How it works

NoSleepIMWorking runs quietly in the Windows system tray and continuously monitors your display configuration.

It automatically changes the Windows lid-close action depending on whether an external display is connected:

| Configuration                      | Lid action		|
| ---------------------------------- | --------------	|
| Laptop display only                | 💤 Sleep			|
| Laptop + external monitor          | 🖥️ Do nothing	|
| External monitor only (lid closed) | 🖥️ Do nothing	|
| External monitor disconnected      | 💤 Sleep			|

The application runs in the user's session rather than as a Windows service, allowing it to correctly access the Windows display configuration APIs.

---

## 📦 Installation

### Option 1 — Download a release

Go to the project's **Releases** page and download the latest version.

Extract the executable and run:

```text
NoSleepIMWorking.exe
```

The application will appear in the Windows system tray.

You can enable:

> **Start with Windows**

from the tray menu if you want it to run automatically after logging in.

### Option 2 — Build from source

Requirements:

* Windows 10 or Windows 11
* Visual Studio 2022 or newer
* C++ Desktop Development workload
* CMake
* Windows SDK

Clone the repository:

```powershell
git clone https://github.com/YOUR_USERNAME/NoSleepIMWorking.git
cd NoSleepIMWorking
```

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build --config Release
```

The executable will be generated in the build directory.

---

## ⚙️ Usage

The application does not requires any configuration.

Connect or disconnect an external monitor and NoSleepIMWorking will automatically update the lid behavior.

---
