#pragma once

#include <windows.h>
#include <vector>
#include <cstdint>

namespace Core
{
    class Memory
    {
    public:
        // Pattern scanning using IDA-style byte string ("55 8B EC 83 EC ? ? 89")
        static uintptr_t FindPattern(HMODULE hModule, const char* signature);
        static uintptr_t FindPattern(uintptr_t baseAddress, size_t searchSize, const char* signature);

        // Pattern scanning with byte array and mask (e.g. "\x55\x8B\xEC", "xxx")
        static uintptr_t FindPattern(uintptr_t start, size_t size, const unsigned char* pattern, const char* mask);
        static uintptr_t FindPattern(HMODULE hModule, const unsigned char* pattern, const char* mask);

        // Retrieve the size of a loaded PE image in memory
        static size_t GetModuleSize(HMODULE hModule);

        // Safe reading using SEH (Structured Exception Handling)
        template <typename T>
        static bool SafeRead(uintptr_t address, T& outVal)
        {
            if (!address) return false;

            __try
            {
                outVal = *reinterpret_cast<const T*>(address);
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        // Safe writing with virtual protection manipulation
        template <typename T>
        static bool SafeWrite(uintptr_t address, const T& inVal)
        {
            if (!address) return false;

            DWORD oldProtect = 0;
            if (!VirtualProtect(reinterpret_cast<void*>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
                return false;

            bool success = false;
            __try
            {
                *reinterpret_cast<T*>(address) = inVal;
                success = true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                success = false;
            }

            VirtualProtect(reinterpret_cast<void*>(address), sizeof(T), oldProtect, &oldProtect);
            return success;
        }

        // Memory patching (write bytes to protected memory)
        static bool Patch(uintptr_t address, const void* data, size_t size);

        // NOP instructions
        static bool Nop(uintptr_t address, size_t size);
    };
}
