#include "Memory.hpp"
#include <sstream>
#include <vector>

namespace Core
{
    size_t Memory::GetModuleSize(HMODULE hModule)
    {
        if (!hModule) return 0;

        auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return 0;

        auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(hModule) + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return 0;

        return static_cast<size_t>(ntHeaders->OptionalHeader.SizeOfImage);
    }

    uintptr_t Memory::FindPattern(uintptr_t start, size_t size, const unsigned char* pattern, const char* mask)
    {
        if (!start || !size || !pattern || !mask) return 0;

        size_t patternLen = strlen(mask);
        if (patternLen == 0 || size < patternLen) return 0;

        const unsigned char* pData = reinterpret_cast<const unsigned char*>(start);

        for (size_t i = 0; i <= size - patternLen; ++i)
        {
            bool found = true;
            for (size_t j = 0; j < patternLen; ++j)
            {
                if (mask[j] == 'x' && pData[i + j] != pattern[j])
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                return start + i;
            }
        }

        return 0;
    }

    uintptr_t Memory::FindPattern(HMODULE hModule, const unsigned char* pattern, const char* mask)
    {
        if (!hModule) return 0;
        size_t size = GetModuleSize(hModule);
        return FindPattern(reinterpret_cast<uintptr_t>(hModule), size, pattern, mask);
    }

    uintptr_t Memory::FindPattern(uintptr_t baseAddress, size_t searchSize, const char* signature)
    {
        if (!baseAddress || !searchSize || !signature) return 0;

        std::vector<unsigned char> bytes;
        std::string mask;

        std::stringstream ss(signature);
        std::string token;

        while (ss >> token)
        {
            if (token == "?" || token == "??")
            {
                bytes.push_back(0);
                mask.push_back('?');
            }
            else
            {
                unsigned char byte = static_cast<unsigned char>(std::stoul(token, nullptr, 16));
                bytes.push_back(byte);
                mask.push_back('x');
            }
        }

        return FindPattern(baseAddress, searchSize, bytes.data(), mask.c_str());
    }

    uintptr_t Memory::FindPattern(HMODULE hModule, const char* signature)
    {
        if (!hModule) return 0;
        size_t size = GetModuleSize(hModule);
        return FindPattern(reinterpret_cast<uintptr_t>(hModule), size, signature);
    }

    bool Memory::Patch(uintptr_t address, const void* data, size_t size)
    {
        if (!address || !data || size == 0) return false;

        DWORD oldProtect = 0;
        if (!VirtualProtect(reinterpret_cast<void*>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
            return false;

        bool success = false;
        __try
        {
            memcpy(reinterpret_cast<void*>(address), data, size);
            success = true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            success = false;
        }

        VirtualProtect(reinterpret_cast<void*>(address), size, oldProtect, &oldProtect);
        return success;
    }

    bool Memory::Nop(uintptr_t address, size_t size)
    {
        if (!address || size == 0) return false;

        std::vector<uint8_t> nops(size, 0x90);
        return Patch(address, nops.data(), size);
    }
}
