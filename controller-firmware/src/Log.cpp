#include "Log.hpp"

#include "mbed.h"
#include <stdarg.h>
#include <stdio.h>
#include <string>

// char Log::logBuffer[LOG_BUFFER_SIZE];

Log::LogHandler *Log::logHandler = NULL;

Log::Log(const char *component) :
    component(component)
{
    logBuffer = new char[LOG_BUFFER_SIZE];
}

Log Log::getLog(const char *component) {
    return Log(component);
}

void Log::setLogHandler(LogHandler *logHandler) {
    Log::logHandler = logHandler;
}

Log::LogHandler *Log::getLogHandler() {
    return Log::logHandler;
}

void Log::trace(const char *fmt, ...) {
    if (logHandler == NULL || !logHandler->shouldLog(LogLevel::TRACE)) {
        return;
    }

    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    Log::logHandler->handleLogMessage(LogLevel::TRACE, component, logBuffer);
}

void Log::debug(const char *fmt, ...) {
    if (Log::logHandler == NULL || !logHandler->shouldLog(LogLevel::DEBUG)) {
        return;
    }

    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    Log::logHandler->handleLogMessage(LogLevel::DEBUG, component, logBuffer);
}

void Log::info(const char *fmt, ...) {
    if (Log::logHandler == NULL || !logHandler->shouldLog(LogLevel::INFO)) {
        return;
    }

    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    Log::logHandler->handleLogMessage(LogLevel::INFO, component, logBuffer);
}

void Log::warn(const char *fmt, ...) {
    if (Log::logHandler == NULL || !logHandler->shouldLog(LogLevel::WARN)) {
        return;
    }

    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    Log::logHandler->handleLogMessage(LogLevel::WARN, component, logBuffer);
}

void Log::error(const char *fmt, ...) {
    if (Log::logHandler == NULL || !logHandler->shouldLog(LogLevel::ERROR)) {
        return;
    }

    va_list args;

    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    Log::logHandler->handleLogMessage(LogLevel::ERROR, component, logBuffer);
}
