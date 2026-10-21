# Stalker_Internal_Suite (Shadow of Chernobyl & Call of Pripyat)

A high-performance internal modification suite, reverse engineering toolkit, and runtime SDK for S.T.A.L.K.E.R.: Shadow of Chernobyl (v1.0006, X-Ray 1.0) and S.T.A.L.K.E.R.: Call of Pripyat (v1.6.02, X-Ray 1.6) built with C++20, DirectX 9 Virtual Method Table (VMT) Hooking, DirectInput 8 Interception, Dear ImGui, and MinHook.

Operating directly inside the virtual address space of XR_3DA.exe (SoC) or xrEngine.exe (CoP), it achieves zero-latency frame-synchronized rendering, seamless input routing without game camera spin, direct memory interaction with proprietary X-Ray Engine classes, and crash-proof execution protected by Structured Exception Handling (SEH).

---

## Cross-Engine Support

The suite compiles into two dedicated 32-bit (x86) dynamic link libraries from a unified codebase:
- Stalker_Internal_SOC.dll: Tailored for S.T.A.L.K.E.R.: Shadow of Chernobyl (Patch 1.0006, XR_3DA.exe).
- Stalker_Internal_COP.dll: Tailored for S.T.A.L.K.E.R.: Call of Pripyat (Patch 1.6.02, xrEngine.exe, DirectX 9/10/11 backends via D3D9/DXGI hooks).

---

## Version 2.0 (Project Medusa Multi-Engine Edition)

Version 2.0 expands the project from a lightweight ESP and trainer into a comprehensive Rage, Spawner, and Visuals suite engineered for deep compatibility with both X-Ray 1.0006 and X-Ray 1.6 engine architectures.

---

## Core Capabilities

### 1. Ragebot and Targeting Engine
- 360-Degree Snap Aimbot: Locks onto valid targets anywhere within full 360-degree spherical space without angle restrictions.
- Rapid Fire Exploit: Negates weapon cycle and fire-rate delays, enabling high-rate continuous automatic fire.
- Faction and Relation Filtering: Utilizes dynamic engine lookup (ClassifyRelation) to selectively target or ignore Enemies, Neutrals, Allies, and Mutants.
- Engine-Level Raycast Wall Check: Queries CObjectSpace::RayTest against the level geometry to detect whether targets are obscured by walls or terrain.
- 3D Distance Priority: Target evaluation algorithm that prioritizes the closest imminent threat in 3D Euclidean space over screen crosshair distance.
- Hitbox Selection: Dynamically targets Head, Neck, Chest, or Pelvis based on bone matrix calculations.

### 2. Built-in Zone Lord Item Spawner
- Integrated In-Menu Spawner: Direct in-game item spawner operating through the ImGui interface.
- 14 Categorized Inventories:
  - Pistols (Fort-12, Beretta 92FS, Browning HPSA, Walther P99, Colt 1911, SIG P220, USP, Desert Eagle)
  - Shotguns and Submachine Guns (MP5, BM-16, TOZ-34, Winchester 1300, SPAS-12)
  - Assault Rifles (AKS-74U, AK-74, AN-94 Abakan, L85, LR-300, SIG SG 550, G36, OTs-14 Groza, AS VAL, FN F2000)
  - Sniper Rifles (VSS Vintorez, SVD, SVU, Gauss Rifle)
  - Heavy Weapons and Explosives (RPG-7, RG-6 Bulldog, F-1 Grenades, RGD-5 Grenades)
  - Unique and Quest Weapons (Fast-firing AKS-74U, Big Ben Desert Eagle, Sniper LR-300, Modified Groza 5.45)
  - Weapon Attachments (PSO-1 Scope, SUSAT Scope, Universal Silencer, GP-25 Launcher, M203 Launcher)
  - Ammunition Types (Standard and armor-piercing calibers for all firearms)
  - Armors and Suits (Leather Jackets, Stalker Sunrise Suits, SEVA Environmental Suits, Military Beryll, Exoskeletons)
  - Artifacts (Medusa, Flash, Crystal, Moonlight, Soul, Urchin, Goldfish, Fireball)
  - Monster Trophies (Bloodsucker tentacles, Snork feet, Boar hooves, Pseudodog tails, Controller hands)
  - Medical Supplies (First Aid Kits, Army Kits, Scientific Kits, Bandages, Rad-Free, Antirad)
  - Food and Rations (Tourist's Delight, Bread, Diet Sausage, Vodka, Energy Drinks)
  - Devices and Quest Items (Scientific Detectors, Flash Drives, PDA Documents, Decoders)
- Direct Engine Spawning: Leverages the engine's internal Level().spawn_item() and CInventory::CreateItem routines to instantiate objects directly into the player entity's inventory.

### 3. DirectX 9 Chams and ESP Visuals
- 6 Chams Rendering Styles:
  - Flat / Solid Color: High-visibility solid fill for clear identification through geometry.
  - Wireframe Mesh: Skeletal polygon overlay highlighting model meshes.
  - Shaded Diffuse: Ambient-shaded material overlay with depth contours.
  - Pulsating Glow: Dynamic sinusoidal breathing intensity glow effect.
  - Glass Hologram: Translucent low-alpha holographic projection.
  - Rainbow Chromatic: Continuous hue-cycling chromatic visualization.
- State-Preserving Draw Hook: Comprehensive backup and restoration of Direct3D 9 device render states (Z-Buffer, Alpha Blending, Stencil, CullMode, Lighting, Vertex and Pixel Shaders) inside IDirect3DDevice9::DrawIndexedPrimitive to completely eliminate font, text, and HUD corruption.
- Faction-Colored 3D Bone Skeletons: Dynamic bone hierarchy ESP with bones colored according to faction relations (Hostile = Red, Neutral = Yellow, Ally = Green, Mutant = Orange).
- Custom Procedural Crosshair Engine: 6 selectable styles (Classic Cross, Center Dot, Circle, Precision Target, T-Shape, CS-Style) with configurable size, gap, thickness, and outline contrast.

### 4. Movement and Teleportation Exploits
- Blink Teleport: Instantly phases the player entity forward through solid barriers, doors, and walls along the camera look vector.
- Backstab Teleport: Calculates coordinates behind the currently locked target and repositions the player entity for immediate tactical advantage.
- Super Jump: Configurable jump speed parameter modifying the player's vertical velocity.
- Speedhack: Multiplier for walk, run, and sprint acceleration parameters.

### 5. Singleplayer Trainers and Memory Patches
- Non-Decaying Godmode: Sustains health at 100% and continuously neutralizes radiation, hunger (satiety maintained at 100%), bleeding, alcohol, and psy-health degradation.
- Infinite Carry Weight: Entry-point bypass patch for CActorCondition::IsCantWalkWeight at xrGame.dll + 0x1DD740.
- Infinite Ammo: Machine-code byte patch disabling magazine decrement and cartridge popping in xrGame.dll.
- Zero Recoil: Suppresses camera angle delta in CWeaponShotEffector::GetDeltaAngle.
- Third-Person Camera: Toggleable 3rd-person perspective modes with console synchronization (cam_1, cam_2, cam_3).
- FOV Changer: Adjusts Field of View using the camera and render device projection parameters.

### 6. Engine Stability and Usability Hardening
- Level Transition Crash Protection: Drawing routines and memory accesses are protected by in-game state validation (XRay::Engine::IsInGame()) and Structured Exception Handling (SEH) across all xr_vector and entity lookups.
- Alt-Tab Optimization: Strips WS_EX_TOPMOST from the game window upon initialization, allowing standard multi-tasking without window freezing.
- Cyrillic Glyph Rendering: Native ImGui font configuration using io.Fonts->GetGlyphRangesCyrillic() for complete UTF-8 localization.

---

## Architecture Overview

```text
Stalker_Internal_SOC/
├── CMakeLists.txt              # CMake x86 Win32 build configuration
├── build.bat                   # Automated one-click build script (MSVC x86)
├── deps/
│   ├── imgui/                  # Dear ImGui core with DirectX 9 & Win32 backends
│   └── minhook/                # x86 API hooking library
└── src/
    ├── main.cpp                # DllMain entry point and lifecycle management
    ├── core/
    │   ├── Globals.hpp / .cpp  # Global module handles and window references
    │   ├── Config.hpp / .cpp   # Profile persistence (.ini configuration system)
    │   ├── Logger.hpp / .cpp   # Console logging system
    │   └── Memory.hpp / .cpp   # Signature scanner, byte patcher, and SEH wrappers
    ├── hooks/
    │   ├── D3D9Hook.hpp / .cpp # Dummy device creation & Direct3D 9 VMT extraction
    │   ├── DirectInputHook.hpp # IDirectInputDevice8 VMT interception
    │   ├── WndProcHook.hpp/.cpp# Window procedure detour and input routing
    │   └── Hooks.hpp / .cpp    # Unified hook manager (EndScene, Reset, DrawIndexedPrimitive)
    ├── xray/
    │   ├── XRayTypes.hpp       # X-Ray math primitives (Fvector, Fmatrix, ClassID)
    │   ├── XRaySDK.hpp         # CRenderDevice, CObject, CEntityAlive, xr_vector (STLPort 5.0)
    │   ├── Offsets.hpp         # Singleton resolvers for Device, Level, and ObjectList
    │   └── WorldToScreen.hpp   # 3D world to 2D screen viewport projection
    ├── features/
    │   ├── Visuals.hpp / .cpp  # ESP render engine (2D boxes, health bars, skeletons, crosshairs)
    │   ├── Aimbot.hpp / .cpp   # Target selection, raycast checks, and ragebot mechanics
    │   └── Misc.hpp / .cpp     # Singleplayer memory patches (godmode, ammo, weight, camera)
    └── ui/
        ├── Style.hpp / .cpp    # S.T.A.L.K.E.R. PDA visual theme
        └── Menu.hpp / .cpp     # Menu interface with Zone Lord spawner and configuration tabs
```

---

## Building

### Requirements
- Operating System: Windows 10 / Windows 11 (64-bit OS running 32-bit game)
- Target Architecture: Win32 / x86 (Strict requirement for X-Ray 32-bit engines)
- Compiler: Visual Studio 2022 (MSVC v143+) with C++ Desktop Development tools
- Build System: CMake 3.24+

### Build Steps

#### Option 1: Automated Script
Run the automated build script from the repository root:
```cmd
build.bat
```
The compiled libraries will be output to:
```text
build\Release\Stalker_Internal_SOC.dll (Shadow of Chernobyl)
build\Release\Stalker_Internal_COP.dll (Call of Pripyat)
```

#### Option 2: Manual CMake Configuration
```cmd
cmake -B build -A Win32
cmake --build build --config Release
```

---

## Injection Instructions

### S.T.A.L.K.E.R.: Shadow of Chernobyl
1. Launch S.T.A.L.K.E.R.: Shadow of Chernobyl (Patch 1.0006, Steam, GOG, or Retail).
2. Load any saved game or start a new game session.
3. Use a standard 32-bit DLL injector (such as System Informer, Process Hacker, or Xenos):
   - Target Process: XR_3DA.exe
   - Module Architecture: x86 (32-bit)
   - Selected Library: build\Release\Stalker_Internal_SOC.dll
4. The debug console will initialize and the user interface will render over the game viewport.

### S.T.A.L.K.E.R.: Call of Pripyat
1. Launch S.T.A.L.K.E.R.: Call of Pripyat (Patch 1.6.02, Steam, GOG, or Retail).
2. Load any saved game or start a new game session.
3. Use a standard 32-bit DLL injector (such as System Informer, Process Hacker, or Xenos):
   - Target Process: xrEngine.exe
   - Module Architecture: x86 (32-bit)
   - Selected Library: build\Release\Stalker_Internal_COP.dll
4. Press INSERT in-game to toggle the Project Medusa menu.

---

## Controls and Keybinds

| Key / Input | Function |
| :--- | :--- |
| INSERT | Toggle Cheat Menu and HUD Overlay |
| END | Clean Eject: safely unhooks all hooks, restores input, and unloads DLL |
| RMB (Hold) | Activates Aimbot on the nearest valid target |
| F5 | Toggle Third-Person Camera Mode |
| Mouse Wheel | Scrolls through menu tabs, item spawner categories, and configuration lists |

---

## Scope and Disclaimer

- Singleplayer Scope: This software is developed strictly for offline, singleplayer gameplay in S.T.A.L.K.E.R.: Shadow of Chernobyl (v1.0006).
- Research and Educational Value: Developed to explore and document memory architectures, DirectX 9 VMT hooks, and reverse engineering methodologies for the X-Ray Engine.
- Non-Disruptive: Does not target or support online multiplayer servers.

---

## License

This project is licensed under the MIT License.
