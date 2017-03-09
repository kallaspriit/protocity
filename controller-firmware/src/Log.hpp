#ifndef LOG_HPP
#define LOG_HPP

class Log {

public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    class LogHandler {
    public:
        virtual void handleLogMessage(LogLevel level, const char *component, const char *message) = 0;
    };

    Log(const char *component);

    static Log getLog(const char *component);
    static void setLogHandler(LogHandler *logHandler);

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
