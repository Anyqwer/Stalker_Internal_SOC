@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo       Building Stalker_Internal Suite (SOC + COP Release x86)
echo ========================================================

set "CMAKE_EXE=cmake"
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    if exist "G:\vs\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "CMAKE_EXE=G:\vs\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    ) else (
        echo [ERROR] CMake not found in PATH or G:\vs.
        exit /b 1
    )
)

echo Using CMake: %CMAKE_EXE%

if not exist build (
    mkdir build
)

echo [1/2] Configuring CMake project (Win32 x86)...
"%CMAKE_EXE%" -B build -A Win32
if %errorlevel% neq 0 (
    echo [ERROR] CMake configure failed.
    exit /b %errorlevel%
)

echo [2/2] Compiling Stalker_Internal Suite (Release)...
"%CMAKE_EXE%" --build build --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    exit /b %errorlevel%
)

echo ========================================================
echo Build Successful! Binaries located at:
echo build\Release\Stalker_Internal_SOC.dll (Shadow of Chernobyl)
echo build\Release\Stalker_Internal_COP.dll (Call of Pripyat)
echo ========================================================
