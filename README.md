# Stalker_Internal_SOC ☢️

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![DirectX 9](https://img.shields.io/badge/DirectX-9-green.svg)](https://docs.microsoft.com/en-us/windows/win32/direct3d9/dx9-graphics)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Windows x86](https://img.shields.io/badge/Platform-Windows%20x86-lightgrey.svg)](https://www.microsoft.com/windows)
[![Engine: X-Ray 1.0006](https://img.shields.io/badge/Engine-X--Ray%201.0006-orange.svg)](https://github.com/OpenXRay/xray-16)

**Stalker_Internal_SOC** is a high-performance, modular internal modification suite, reverse engineering toolkit, and runtime SDK for **S.T.A.L.K.E.R.: Shadow of Chernobyl** (v1.0006, 32-bit x86, DirectX 9) built with **C++20**, **DirectX 9 Virtual Method Table (VMT) Hooking**, **DirectInput 8 Interception**, **Dear ImGui**, and **MinHook**.

Operating directly inside the virtual address space of `XR_3DA.exe`, it achieves zero-latency frame-synchronized rendering, seamless input routing without game camera spin, direct memory interaction with proprietary **X-Ray Engine** classes, and crash-proof execution protected by **Structured Exception Handling (SEH)**.

---

## 🌟 Key Features

### 1. Direct3D 9 Virtual Method Table (VMT) Hooking
- **Frame-Synchronized Rendering**: Intercepts `IDirect3DDevice9::EndScene` (VMT index 42) via MinHook, drawing visual overlays and HUD elements in perfect sync with the game's native render cycle.
- **Crash-Free Mode Switch & Resize**: Hooks `IDirect3DDevice9::Reset` (VMT index 16), correctly invalidating and restoring ImGui device textures (`InvalidateDeviceObjects` / `CreateDeviceObjects`) during `Alt+Tab`, resolution changes, and fullscreen toggles.
- **Non-Invasive Dummy Device**: Extracts authentic Direct3D 9 virtual method tables dynamically at runtime using a temporary Win32 window, requiring zero modification to system DirectX DLLs.

### 2. Dual-Engine Input Suppression & DirectInput 8 Hooking
- **Exclusive Mouse State Neutralization**: Intercepts `IDirectInputDevice8::GetDeviceState` (VMT index 9) and zeroes mouse deltas while the cheat menu is active. Completely prevents unwanted camera spinning and accidental weapon firing while interacting with UI sliders and buttons.
- **DirectInput Buffered Event Routing**: Hooks `IDirectInputDevice8::GetDeviceData` (VMT index 10) to intercept buffered mouse events (`DIMOFS_BUTTON0`, `DIMOFS_BUTTON1`, `DIMOFS_BUTTON2`, `DIMOFS_Z`).
- **Smooth Mouse Wheel Scrolling**: Translates DirectInput wheel axis deltas (`DIMOFS_Z`) directly into ImGui mouse wheel events (`io.AddMouseWheelEvent`), enabling full mouse-wheel scrolling across all menu tabs and combo lists.
- **Window Procedure (WndProc) Detour**: Intercepts window messages via `SetWindowLongPtrA(GWLP_WNDPROC)` with hardware-level asynchronous key polling (`GetAsyncKeyState`) to ensure 100% reliable menu toggling.
- **Software Cursor Routing**: Automatically frees Windows mouse cursor constraints (`ClipCursor(nullptr)`) and renders a customized ImGui software cursor.

### 3. Deep X-Ray Engine Reversing & Native SDK Integration
- **STLPort 5.0 Vector Alignment**: Accurately accounts for X-Ray's 32-bit STLPort memory layout (`xr_vector<T>` allocator proxy padding at offset `+0x00`, followed by `_M_start` at `+0x04`, `_M_finish` at `+0x08`, and `_M_end_of_storage` at `+0x0C`).
- **Engine-Native Object Enumeration**: Resolves and calls exported engine routines `?o_count@CObjectList@@QAEIXZ` and `?o_get_by_iterator@CObjectList@@QAEPAVCObject@@I@Z` directly, ensuring full visibility over all active and sleeping level entities.
- **Direct Shared String Reader**: Resolves X-Ray's atomic reference-counted `shared_str` handles (`cName`, `cNameSect`, `cNameVisual`) by reading the null-terminated string value buffer at offset `+0x0C` of `str_value`.
- **Precalculated Full Projection**: Directly retrieves the engine's precalculated 64-byte world-view-projection matrix (`mFullTransform`) from `CRenderDevice + 0x2BC`, eliminating matrix multiplication errors and ensuring pixel-perfect screen projections.
- **Structured Exception Handling (SEH)**: Wraps all game memory reads and entity dereferences in `__try / __except (EXCEPTION_EXECUTE_HANDLER)` blocks, completely preventing X-Ray engine crashes and crashdumps (the famous "Green Bug" / BugTrap).

### 4. Advanced Entity Classification & ESP Visuals Engine
- **Multi-Factor Entity Identification**: Evaluates 3D visual model paths (`cNameVisual` from `actors\`, `monsters\`, `weapons\`), spawn sections (`cNameSect`), and instance names (`cName`). Correctly recognizes all unique storyline NPCs (e.g. Wolf `esc_wolf`, Fanatic `esc_fanat`, Nimble `esc_shustryi`) and novices (`esc_novice`) without false rejections.
- **Faction & Alignment Detection**: Categorizes stalkers into Loners/Neutrals, Bandits, Military, Monolith, Mercenaries, Duty, Freedom, and Zombies with dedicated color profiles.
- **2D Bounding Boxes**: Adaptive Corner Boxes and Full Rectangles with black contrast outlines.
- **Dynamic Gradient Health Bars**: Smooth color gradient shifting from emerald green to bright red based on remaining health.
- **Directional Snaplines**: Traces origin lines from screen bottom or crosshair directly to target feet.
- **Atmospheric Entity Tags**: Displays entity role, instance name, and 3D Euclidean distance in meters (e.g. `[Stalker] esc_wolf 15m`).
- **Selective Entity Filters**: Independent checkboxes for Stalkers/Enemies, Mutants/Monsters, Artifacts, and Dropped Weapons.

### 5. High-Precision Mathematical World-to-Screen (W2S)
- Projects 3D coordinates from the Zone's world space into 2D screen viewport coordinates:
  $$\text{NDC}_x = \frac{\mathbf{v} \cdot \mathbf{M}_{*,1}}{\mathbf{v} \cdot \mathbf{M}_{*,4}}, \quad \text{NDC}_y = \frac{\mathbf{v} \cdot \mathbf{M}_{*,2}}{\mathbf{v} \cdot \mathbf{M}_{*,4}}$$
- Automatic near-plane camera clipping ($W \le 0.001$) preventing inverted artifact projection behind the player's view frustum.

### 6. Dynamic Relative Mouse Aimbot
- **Hardware-Level Relative Mouse Movement**: Employs `mouse_event(MOUSEEVENTF_MOVE, dx, dy)` to smoothly guide crosshairs toward targets without interfering with raw camera memory.
- **Target Hitbox Selection**: Target Head (`posY + 1.65m`) or Chest (`posY + 1.10m`) for human characters; adjusted heights for mutants.
- **Configurable FOV Circle**: Adjustable search radius with visual on-screen FOV ring.
- **Variable Smoothness Curve**: Linear easing slider preventing unnatural camera snapping.
- **Entity Filter Switches**: Option to target Stalkers/NPCs, Mutants, or both.

### 7. Singleplayer Gameplay Exploits & Memory Patches
- **Godmode (Immunity)**: Continuously sustains the player entity's health state at maximum.
- **Infinite Ammo & No Reload**: Runtime machine-code byte patch in `xrGame.dll` preventing weapon magazine decrement.
- **Weight Limit Multiplier**: Adjusts maximum carry capacity for unlimited inventory looting.
- **Zero Recoil & Movement Tweaks**: Removes weapon pitch kicking and allows custom sprint and jump modifications.

### 8. Atmospheric Liquid S.T.A.L.K.E.R. PDA Theme & Diagnostics
- Handcrafted visual styling echoing the iconic Chernobyl Exclusion Zone atmosphere: dark obsidian slate, radioactive amber headers, and military muted borders.
- **World & SDK Diagnostics Tab**: Displays live addresses of `CRenderDevice`, `g_pGameLevel`, player entity pointer (`CActor`), 3D camera coordinates $(X, Y, Z)$, and active/total object counts.

### 9. Clean Eject & Lifecycle Safety
- **One-Click Hotkey Eject**: Pressing `[END]` or clicking "Eject / Unhook DLL" safely unhooks DirectInput, restores original `WndProc`, invalidates ImGui objects, shuts down MinHook, closes the debug console, and calls `FreeLibraryAndExitThread`. Leaves the game running normally without crashes.

---

## 🏗 Architecture Overview

```text
Stalker_Internal_SOC/
├── CMakeLists.txt              # CMake x86 Win32 build configuration (DirectX 9, DirectInput 8)
├── build.bat                   # Automated one-click build script (MSVC Win32)
├── deps/
│   ├── imgui/                  # Dear ImGui core with DirectX 9 & Win32 backend implementations
│   └── minhook/                # Minimalistic x86 API hooking library
└── src/
    ├── main.cpp                # DllMain entry point, background thread lifecycle & safe ejection
    ├── core/
    │   ├── Globals.hpp / .cpp  # Global module handles, window instances & state flags
    │   ├── Config.hpp / .cpp   # Cheat state persistence, visual colors & toggle configuration
    │   ├── Logger.hpp / .cpp   # Color-coded Win32 debug console logger
    │   └── Memory.hpp / .cpp   # Signature scanner (IDA patterns), byte patcher & SEH memory guards
    ├── hooks/
    │   ├── D3D9Hook.hpp / .cpp # Dummy device creation & Direct3D 9 VMT extraction
    │   ├── DirectInputHook.hpp # IDirectInputDevice8 VMT hook (GetDeviceState, GetDeviceData)
    │   ├── WndProcHook.hpp/.cpp# Window procedure detour, input blocking & menu hotkey handler
    │   └── Hooks.hpp / .cpp    # Unified hook manager orchestrating EndScene & Reset callbacks
    ├── xray/
    │   ├── XRayTypes.hpp       # X-Ray math primitives (Fvector, Fvector2, Fmatrix, ClassID)
    │   ├── XRaySDK.hpp         # CRenderDevice, CObject, CEntityAlive, xr_vector (STLPort 5.0)
    │   ├── Offsets.hpp         # Singleton resolvers for Device, Level, and ObjectList exports
    │   └── WorldToScreen.hpp   # 3D world to 2D screen viewport projection algorithms
    ├── features/
    │   ├── Visuals.hpp / .cpp  # ESP render engine (2D boxes, health bars, tags, snaplines)
    │   ├── Aimbot.hpp / .cpp   # Target selection, FOV verification & relative mouse steering
    │   └── Misc.hpp / .cpp     # Singleplayer memory patches (infinite ammo, godmode, weight)
    └── ui/
        ├── Style.hpp / .cpp    # Atmospheric dark amber S.T.A.L.K.E.R. PDA theme
        └── Menu.hpp / .cpp     # ImGui tab rendering (Visuals, Aimbot, Misc, Diagnostics, Settings)
```

---

## 🚀 Building & Running

### Prerequisites
- **Operating System**: Windows 10 / Windows 11 (64-bit OS running 32-bit game)
- **Target Architecture**: **Win32 / x86** (Strict requirement for S.T.A.L.K.E.R. 1.0006 `XR_3DA.exe`)
- **Compiler**: Visual Studio 2022 (MSVC v143+) with C++ Desktop Development workload
- **Build System**: CMake 3.24+

### Build Instructions

#### Option 1: One-Click Build Script (Recommended)
1. Open PowerShell or Command Prompt in the repository folder.
2. Execute the automated build script:
   ```cmd
   build.bat
   ```
3. The compiled DLL will be located at:
   ```text
   build\Release\Stalker_Internal_SOC.dll
   ```

#### Option 2: Manual CMake Build
1. Generate the Visual Studio solution targeting **Win32 (x86)**:
   ```cmd
   cmake -B build -A Win32
   ```
2. Compile the Release binary:
   ```cmd
   cmake --build build --config Release
   ```

---

## 💉 Injecting into S.T.A.L.K.E.R.

1. Launch **S.T.A.L.K.E.R.: Shadow of Chernobyl** (Patch 1.0006, Steam, GOG, or Retail).
2. Load any savegame or start a new game (e.g. Cordon / Rookie Village).
3. Use any reputable 32-bit DLL injector:
   - **System Informer / Process Hacker**:  
     Locate `XR_3DA.exe` $\rightarrow$ Right-Click $\rightarrow$ `Miscellaneous` $\rightarrow$ `Inject DLL...` $\rightarrow$ select `Stalker_Internal_SOC.dll`.
   - **Xenos Injector**:  
     Select process `XR_3DA.exe`, set architecture to `x86`, select `Stalker_Internal_SOC.dll`, and press `Inject`.
   - **Cheat Engine**:  
     Open Process `XR_3DA.exe` $\rightarrow$ `Memory View` $\rightarrow$ `Tools` $\rightarrow$ `Inject DLL`.
4. The debug console will appear alongside the game, and the S.T.A.L.K.E.R. Internal menu will render over the game viewport.

---

## ⌨️ Controls & Hotkeys

| Key / Control | Action |
| :--- | :--- |
| **`[INSERT]`** | Toggle the Cheat Menu / HUD overlay |
| **`[END]`** | Clean Eject: completely unhooks all routines, restores input, and unloads DLL safely |
| **`[RMB]` (Hold)** | Activates Smooth Aimbot on the nearest target within FOV |
| **Mouse Wheel** | Scrolls tabs, entity lists, and settings dropdowns |
| **`World & SDK` Tab** | Displays real-time engine memory diagnostics and object counts |

---

## ⚠️ Anti-Cheat & Singleplayer Scope

- **Singleplayer Focus**: This project is developed strictly for offline, singleplayer gameplay in *S.T.A.L.K.E.R.: Shadow of Chernobyl* (v1.0006).
- **Educational Value**: Created solely to explore and document memory architectures, DirectX 9 VMT hooks, DirectInput 8 message pumps, and reverse engineering principles of the open-source **X-Ray Engine**.
- **No Multiplayer Disruption**: Does not support or target online multiplayer servers.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
Feel free to study, fork, and build upon this codebase for educational and game modding research.
