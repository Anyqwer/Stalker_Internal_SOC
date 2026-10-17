#include "Logger.hpp"
#include <cstdio>
#include <cstdarg>

namespace Core
{
    FILE* Logger::s_pStream = nullptr;
    HANDLE Logger::s_hConsole = INVALID_HANDLE_VALUE;
    bool Logger::s_bInitialized = false;

    void Logger::InitConsole(const char* consoleTitle)
    {
        if (s_bInitialized) return;

        if (AllocConsole())
        {
            freopen_s(&s_pStream, "CONOUT$", "w", stdout);
            freopen_s(&s_pStream, "CONOUT$", "w", stderr);
            freopen_s(&s_pStream, "CONIN$", "r", stdin);

            SetConsoleTitleA(consoleTitle);
            s_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

            // Enable ANSI colors if supported
            DWORD dwMode = 0;
            if (GetConsoleMode(s_hConsole, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(s_hConsole, dwMode);
            }

            s_bInitialized = true;

            SetConsoleTextAttribute(s_hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "========================================================\n";
            std::cout << "   S.T.A.L.K.E.R. Shadow of Chernobyl - Internal Base   \n";
            std::cout << "      Engine: X-Ray 1.0006 (DirectX 9 x86)             \n";
            std::cout << "========================================================\n\n";
            SetConsoleTextAttribute(s_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }

    void Logger::ShutdownConsole()
    {
        if (!s_bInitialized) return;

        if (s_pStream)
        {
            fclose(s_pStream);
            s_pStream = nullptr;
        }

        FreeConsole();
        s_bInitialized = false;
        s_hConsole = INVALID_HANDLE_VALUE;
    }

    void Logger::Log(LogLevel level, const char* format, ...)
    {
        if (!s_bInitialized || s_hConsole == INVALID_HANDLE_VALUE) return;

        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        switch (level)
        {
        case LogLevel::Info:
            SetConsoleTextAttribute(s_hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "[*] ";
            break;
        case LogLevel::Success:
            SetConsoleTextAttribute(s_hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "[+] ";
            break;
        case LogLevel::Warning:
            SetConsoleTextAttribute(s_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "[!] ";
            break;
        case LogLevel::Error:
            SetConsoleTextAttribute(s_hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            std::cout << "[-] ";
            break;
        case LogLevel::Debug:
            SetConsoleTextAttribute(s_hConsole, FOREGROUND_INTENSITY);
            std::cout << "[DEBUG] ";
            break;
        }

        SetConsoleTextAttribute(s_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << buffer << "\n";
    }

    void Logger::Info(const char* format, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log(LogLevel::Info, "%s", buffer);
    }

    void Logger::Success(const char* format, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log(LogLevel::Success, "%s", buffer);
    }

    void Logger::Warning(const char* format, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log(LogLevel::Warning, "%s", buffer);
    }

    void Logger::Error(const char* format, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log(LogLevel::Error, "%s", buffer);
    }

    void Logger::Debug(const char* format, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log(LogLevel::Debug, "%s", buffer);
    }
}
