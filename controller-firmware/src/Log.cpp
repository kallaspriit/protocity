#include "Log.hpp"

#include "mbed.h"
#include <stdarg.h>
#include <stdio.h>
#include <string>

// char Log::logBuffer[LOG_BUFFER_SIZE];

Log::Log(const char *component) :
    component(component)
{
    logBuffer = new char[LOG_BUFFER_SIZE];
}

Log Log::getLog(const char *component) {
    return Log(component);
}

void Log::trace(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    printf("# %-5s | %-35s | %s\n", "TRACE", component, logBuffer);
}

void Log::debug(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    printf("# %-5s | %-35s | %s\n", "DEBUG", component, logBuffer);
}

void Log::info(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    printf("# %-5s | %-35s | %s\n", "INFO", component, logBuffer);
}

void Log::warn(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    printf("# %-5s | %-35s | %s\n", "WARN", component, logBuffer);
}

void Log::error(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    error("# %-5s | %-35s | %s\n", "ERROR", component, logBuffer);
}
