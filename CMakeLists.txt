cmake_minimum_required(VERSION 3.20)
project(Stalker_Internal_SOC LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# S.T.A.L.K.E.R. Shadow of Chernobyl is strictly a 32-bit (x86) DirectX 9 application
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Target architecture is 64-bit! S.T.A.L.K.E.R. requires 32-bit (x86) DLL. Use: cmake -A Win32 ..")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/minhook/include
)

# Source files
set(SOURCES_CORE
    src/core/Globals.cpp
    src/core/Config.cpp
    src/core/Logger.cpp
    src/core/Memory.cpp
)

set(SOURCES_HOOKS
    src/hooks/D3D9Hook.cpp
    src/hooks/WndProcHook.cpp
    src/hooks/DirectInputHook.cpp
    src/hooks/Hooks.cpp
)

set(SOURCES_XRAY
    src/xray/WorldToScreen.cpp
)

set(SOURCES_FEATURES
    src/features/Visuals.cpp
    src/features/Aimbot.cpp
    src/features/Misc.cpp
)

set(SOURCES_UI
    src/ui/Style.cpp
    src/ui/Menu.cpp
)

set(SOURCES_IMGUI
    deps/imgui/imgui.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/backends/imgui_impl_dx9.cpp
    deps/imgui/backends/imgui_impl_win32.cpp
)

set(SOURCES_MINHOOK
    deps/minhook/src/buffer.c
    deps/minhook/src/hook.c
    deps/minhook/src/trampoline.c
    deps/minhook/src/hde/hde32.c
    deps/minhook/src/hde/hde64.c
)

set(ALL_SOURCES
    src/main.cpp
    ${SOURCES_CORE}
    ${SOURCES_HOOKS}
    ${SOURCES_XRAY}
    ${SOURCES_FEATURES}
    ${SOURCES_UI}
    ${SOURCES_IMGUI}
    ${SOURCES_MINHOOK}
)

add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})

# Windows libraries (Direct3D 9, DirectInput 8, User32, Gdi32)
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d9
    dinput8
    dxguid
    user32
    gdi32
)

# Compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W3
        /MP
        /EHsc
    )
endif()

# Set output name
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "Stalker_Internal_SOC"
    PREFIX ""
)
