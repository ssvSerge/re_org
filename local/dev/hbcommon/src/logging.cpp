#include "../include/logging.h"
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <stdarg.h>
#include <time.h>
#include "multitask.h"

#ifndef __linux__
    #ifdef _WIN32
    #include <fstream>
    #include <Windows.h>
    #endif
#endif

static unsigned verbosity = 7;

FILE* g_fFile = nullptr;

std::mutex g_i_mutex;

bool is_log_file_set()
{
    return g_fFile != nullptr;
}

void set_log_file(FILE* fFile)
{
    const std::lock_guard<std::mutex> lock(g_i_mutex);
    if(nullptr != g_fFile){
        fclose(g_fFile);
    }
    g_fFile = fFile;
}

void close_log_file()
{
    if(g_fFile){
        fclose(g_fFile);
    }
}

void set_verbosity(int32_t vrb)
{
    const std::lock_guard<std::mutex> lock(g_i_mutex);
    verbosity = vrb;
}

void _hb_msg(unsigned level, const char *fmt, ...)
{
    // Throw a lock guard around this sucker
    const std::lock_guard<std::mutex> lock(g_i_mutex);

    if ( g_fFile == nullptr){
        return;
    }

    if (level < 2)
        level = 2;
    else
    if (level > 7)
        level = 7;

    level = 7;

    if ( 1 )
    {
#ifdef __linux__
        int _errno = errno;
#elif defined _WIN32
        int _errno = GetLastError();
#endif
        va_list ap;
        std::string ts = get_timestamp().c_str();
        fprintf(g_fFile, "%s: ", ts.c_str());
        va_start(ap, fmt);
        vfprintf(g_fFile, fmt, ap);
        va_end(ap);

        fprintf(g_fFile, "\n");
        fflush(g_fFile);

        _errno = _errno;
    }
}

std::string get_timestamp()
{
    using namespace std;
    using namespace std::chrono;

    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream stream;
    stream << std::put_time(std::localtime(&t_c), "%H:%M:%S");
    stream << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return stream.str();
}

static const char levels[][8] = {
    {""},
    {""},
    {"crit:"},
    {"err: "},
    {"warn:"},
    {"note:"},
    {"info:"},
    {"dbg: "},
};
