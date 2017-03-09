#ifndef LOG_HPP
#define LOG_HPP

class Log {

public:
    Log(const char *component);

    static Log getLog(const char *component);

    void trace(const char *fmt, ...);
    void debug(const char *fmt, ...);
    void info(const char *fmt, ...);
    void warn(const char *fmt, ...);
    void error(const char *fmt, ...);

private:
    const char *component;
    static const int LOG_BUFFER_SIZE = 1024;
    char *logBuffer;

};

#endif
