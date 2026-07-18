#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>

//#ifdef CONSOLE_ENABLED

namespace logger
{
    inline bool m_save_to_file = false;
    inline char m_log_file_path[MAX_PATH] = { 0 };
    inline char m_base_directory[MAX_PATH] = { 0 };

    inline void set_base_directory(HMODULE module)
    {
        GetModuleFileNameA(module, m_base_directory, MAX_PATH);
        char* last_slash = strrchr(m_base_directory, '\\');
        if (last_slash)
            *last_slash = '\0';
        else
            strcpy_s(m_base_directory, ".");
    }

    inline void set_output_directory(const char* directory)
    {
        if (directory && directory[0] != '\0')
            strcpy_s(m_base_directory, directory);
    }

    inline const char* executable_directory()
    {
        if (m_base_directory[0] == '\0')
            set_base_directory(NULL);

        return m_base_directory;
    }

    inline void build_path(char* out, size_t out_size, const char* file_name)
    {
        sprintf_s(out, out_size, "%s\\%s", executable_directory(), file_name);
    }

    inline void initialize(bool save_to_file = false)
    {
        m_save_to_file = save_to_file;
        if (m_save_to_file)
            build_path(m_log_file_path, sizeof(m_log_file_path), "ttapp_log.txt");

        AllocConsole();
        SetConsoleTitleA(xorstr_("ttapp"));
        FILE* dummy = nullptr;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        freopen_s(&dummy, "CONIN$", "r", stdin);
    }

    inline void shutdown()
    {
        PostMessageA(GetConsoleWindow(), WM_CLOSE, 0, 0);
        FreeConsole();
    }

    inline void log(const char* file, int line, WORD color, const char* fmt, ...)
    {
        SYSTEMTIME time;
        GetLocalTime(&time);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD originalAttrs = consoleInfo.wAttributes;

        printf(xorstr_("[%02d:%02d:%02d] "), time.wHour, time.wMinute, time.wSecond);

        const char* file_name = strrchr(file, '\\') ? (strrchr(file, '\\') + 1) : file;

        SetConsoleTextAttribute(hConsole, color);
        printf(xorstr_("%s:%d:"), file_name, line);

        SetConsoleTextAttribute(hConsole, originalAttrs);
        printf(" ");

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        printf("\n");

        if (m_save_to_file && m_log_file_path[0] != '\0') {
            FILE* logFile = nullptr;
            fopen_s(&logFile, m_log_file_path, "a");
            if (logFile) {
                fprintf(logFile, "[%02d:%02d:%02d] %s:%d: ", time.wHour, time.wMinute, time.wSecond, file_name, line);
                va_list args2;
                va_start(args2, fmt);
                vfprintf(logFile, fmt, args2);
                va_end(args2);
                fprintf(logFile, "\n");
                fclose(logFile);
            }
        }
    }
}

#define LOG(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_RED, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN, fmt, __VA_ARGS__)
#define LOG_SUCCESS(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_GREEN, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) logger::log(__FILE__, __LINE__, FOREGROUND_INTENSITY | FOREGROUND_BLUE, fmt, __VA_ARGS__)
//
//#else
//
//namespace logger
//{
//    inline void initialize() {  }
//    inline void shutdown() {  }
//}
//
//#define LOG(fmt, ...) ((void)0)
//#define LOG_ERROR(fmt, ...) ((void)0)
//#define LOG_WARNING(fmt, ...) ((void)0)
//#define LOG_SUCCESS(fmt, ...) ((void)0)
//#define LOG_INFO(fmt, ...) ((void)0)
//
//#endif
