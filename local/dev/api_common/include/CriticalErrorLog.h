#pragma once

#ifndef CRITICAL_ERROR_LOG
#define CRITICAL_ERROR_LOG

#include <ISELogger.h>
#include <IXServiceProvider.h>
#include <ISensorInstance.h>
#include <cstdarg>
#include <string>
#include <string.h>
#include <memory>

#if 0
static std::string log_format(const std::string fmt_str, ...) {

    int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */

    std::unique_ptr<char[]> formatted;

    va_list ap;

    while (1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }

    return std::string(formatted.get());
}
#endif

// NOTE: Enable only for debugging purposes, not to be enabled for release builds
//#define LOGMSG(...) ISensorInstance::GetInstance()->GetServiceProvider()->GetLogger()->Log_String(SEL_DEBUG, __FILE__, log_format(__VA_ARGS__))

#define LOGMSG_1(...) log_format(__VA_ARGS__)

// #ifndef LOG_DISABLED
//    #define LOGMSG(...) log_format(__VA_ARGS__)
// #else
//    #define LOGMSG(...)
// #endif

#endif
