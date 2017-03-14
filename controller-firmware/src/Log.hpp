#ifndef LOG_HPP
#define LOG_HPP

#include <string.h>

class Log {

public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        NONE
    };

    class LogHandler {
    public:
        LogHandler(Log::LogLevel minimumLevel) {
            setMinimumLevel(minimumLevel);
        }

        void setMinimumLevel(Log::LogLevel minimumLevel) {
            this->minimumLevel = minimumLevel;
        }

        void setMinimumLevel(const char *minimumLevel) {
            this->minimumLevel = parseLogLevel(minimumLevel);
        }

        LogLevel getMinimumLevel() {
            return minimumLevel;
        }

        bool shouldLog(LogLevel level) {
            return level >= minimumLevel;
        }

        virtual void handleLogMessage(LogLevel level, const char *component, const char *message) = 0;

        static const char *logLevelToName(Log::LogLevel level) {
    		switch (level) {
    			case Log::LogLevel::TRACE:
    				return "TRACE";

    			case Log::LogLevel::DEBUG:
    				return "DEBUG";

    			case Log::LogLevel::INFO:
    				return "INFO";

    			case Log::LogLevel::WARN:
    				return "WARN";

    			case Log::LogLevel::ERROR:
    				return "ERROR";

                case Log::LogLevel::NONE:
    				return "NONE";
    		}

    		return "UNKNOWN";
    	}

        static Log::LogLevel parseLogLevel(const char *name) {
            if (strcmp(name, "TRACE") == 0) {
                return Log::LogLevel::TRACE;
            } else if (strcmp(name, "DEBUG") == 0) {
                return Log::LogLevel::DEBUG;
            } else if (strcmp(name, "INFO") == 0) {
                return Log::LogLevel::INFO;
            } else if (strcmp(name, "WARN") == 0) {
                return Log::LogLevel::WARN;
            } else if (strcmp(name, "ERROR") == 0) {
                return Log::LogLevel::ERROR;
            } else if (strcmp(name, "NONE") == 0) {
                return Log::LogLevel::NONE;
            } else {
                // should not happen
                return Log::LogLevel::TRACE;
            }
        }

    protected:
        Log::LogLevel minimumLevel;
    };

    Log(const char *component);

    static Log getLog(const char *component);
    static void setLogHandler(LogHandler *logHandler);
    static Log::LogHandler *getLogHandler();

    void trace(const char *fmt, ...);
    void debug(const char *fmt, ...);
    void info(const char *fmt, ...);
    void warn(const char *fmt, ...);
    void error(const char *fmt, ...);

private:
    static const int LOG_BUFFER_SIZE = 1024;

    static LogHandler *logHandler;

    const char *component;
    char *logBuffer;

};

#endif
