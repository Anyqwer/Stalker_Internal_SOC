#pragma once

#include <windows.h>
#include <string>
#include <iostream>

namespace Core
{
    enum class LogLevel
    {
        Info,
        Success,
        Warning,
        Error,
        Debug
    };

    class Logger
    {
    public:
        static void InitConsole(const char* consoleTitle = "S.T.A.L.K.E.R. Shadow of Chernobyl - Internal Debug Console");
        static void ShutdownConsole();

        static void Log(LogLevel level, const char* format, ...);
        static void Info(const char* format, ...);
        static void Success(const char* format, ...);
        static void Warning(const char* format, ...);
        static void Error(const char* format, ...);
        static void Debug(const char* format, ...);

    private:
        static FILE* s_pStream;
        static HANDLE s_hConsole;
        static bool s_bInitialized;
    };
}
