#ifndef LIBCLC_LOGGER_H
#define LIBCLC_LOGGER_H

#include "clc/data/Buffer.h"
#include "clc/data/Hashtable.h"
#ifndef SINGLE_THREADED
#include "clc/os/RWLock.h"
#endif
#include "clc/data/Set.h"


namespace clc
{

class Logger;
class LogAppender;

/**
 *  Owns all Logger instances.
 */
class Loggers
{
public:
    Loggers();
    ~Loggers();

    /**
     *  Clears all Logger instances, and all references to all LogAppenders.
     */
    void clear();

    /**
     *  Clears all references to the LogAppender.
     */
    void clearAppender(const LogAppender*);

    /**
     *  Creates a default root Logger.
     */
    void setRoot();

    void put(Logger* l);

    /**
     *  Returns the named Logger.  If it does not yet exist, it is created.  Ownership remains
     *  with the Loggers.
     *  @param[in] name  The name of the logger, in dotted form.
     */
    Logger* get(const char* name);

protected:
#ifndef SINGLE_THREADED
    friend class Logger;  // Logger instances share the lock
    void rlock() { m_rwlock.readLock(); }
    void wlock() { m_rwlock.writeLock(); }
    void unlock() { m_rwlock.unlock(); }
    RWLock m_rwlock;
#endif

    void clearAppenderUnchecked(const LogAppender*);

    class LoggerHashtable : public Hashtable
    {
    public:
        ~LoggerHashtable();
        /**
         *  Puts the logger into the hashtable.  Best effort; on memory allocation errors
         *  the Logger is simply deleted.
         *  @throw nothing
         */
        void putLogger(const void* key, size_t len, Logger* l);
        Logger* getLogger(const void* key, size_t len) const;
    protected:
        void deleteValue(void* value) const;
    } m_loggers;
    bool m_init;  // To avoid using during shutdown after static instance is dead
};

/**
 *  Utility class to manage the global state of the logging system, and convenience logging
 *  functions.
 */
class Log
{
public:
    /**
     *  Log levels, to be passed to the various convenience logging methods.
     */
    enum Level {
        Unset,  ///< No level set; inherit from parent
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    /**
     *  Restores the inital state, destroying all Loggers and recreating the root Logger.
     *  Probably not useful from most programs, but useful for unit tests.
     */
    static void reset();

    /**
     *  Gets the named logger.  If the logger does not yet exist, it is created and the default
     *  logging level is inherited from its parent.
     *  @param[in] name  The name of the logger, in dotted form.  The parent is the logger named
     *      by stripping off the last dot-separated component.
     *  @return The specified Logger.
     */
    static Logger* get(const char* name);

    /**
     *  Convenience methods to log to a specific logger.  If the logger's level includes the
     *  specified level, the message is forwarded on to all Appenders up the heirarchy.
     * @param name  Identifies the receiving Logger.
     */
    static void log(const char* name, Log::Level, const char* fmt, va_list args);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void log(const char* name, Log::Level, const char* fmt, ...);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void trace(const char* name, const char* fmt, ...);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void debug(const char* name, const char* fmt, ...);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void info(const char* name, const char* fmt, ...);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void warn(const char* name, const char* fmt, ...);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void error(const char* name, const char* fmt, ...);
    /**
     * @param name  Identifies the receiving Logger.
     */
    static void fatal(const char* name, const char* fmt, ...);

protected:
    static Loggers loggers;
};


/**
 *  Abstract class which receives the formatted log messages.  A single instance may be set for
 *  any number of Logger instances within the same set of Loggers.  Ownership remains with the
 *  caller; it is not transferred to the Loggers (so that static LogAppenders are supported).
 *  Destructor causes it to unregister itself from all Logger instances.  Safe to destroy while
 *  logging is occurring.
 */
class LogAppender
{
public:
    LogAppender() : m_loggers(0) {}
    virtual ~LogAppender() { detach(); }

    /**
     *  Appends (or "accepts") the formatted log message.
     *  Derived classes must implement this in a threadsafe manner.
     *  Any exceptions thrown from this method will be consumed and ignored by the Logger.
     */
    virtual void append(Buffer& s) = 0;

    void setLoggers(Loggers* loggers) {
        ASSERT(!m_loggers || !loggers || m_loggers == loggers);
        m_loggers = loggers;
    }
protected:
    void detach() {
        if (m_loggers) {
            m_loggers->clearAppender(this);
            m_loggers = 0;
        }
    }

    /**
     *  The set of Loggers using this Appender, or 0 if none.
     */
    Loggers* m_loggers;
};

/**
 *  An instance of a named logger.  Each distinct logger name referenced in the program (such as ""
 *  or "myapp" or "myapp.subsys") will have its own Logger instance.  One usage style would be for a class
 *  to get a reference to its Logger in its constructor.  This is most efficient.  Or for a more
 *  functional style, see the convenience methods in clc::Log.
 */
class Logger
{
public:
    friend class Loggers;

    /**
     *  @return The parent logger in the heirarchy, or 0 if this is the root ("") logger.
     */
    Logger* getParent();

    /**
     *  @return The full dotted name of this logger.  Empty string for the root logger.
     */
    const Buffer getName() const;

    /**
     *  Sets the minimum level that this logger will log.
     */
    void setLevel(Log::Level level);

    /**
     *  @return The current level this logger is logging at.
     */
    Log::Level getLevel() const;

    /**
     *  Sets the Appender to be used by this Logger.  Appenders can be shared among multiple
     *  Loggers.
     *  @param[in] appender  The Appender to use.  Ownership remains with the caller.
     */
    void setAppender(LogAppender* appender);

    /**
     *  Log the message at the specified level.
     */
    void log(Log::Level level, const char* fmt, ...);

    /**
     *  Log the message at the specified level.
     */
    void log(Log::Level level, const char* fmt, va_list args);

    /**
     */
    void trace(const char* fmt, ...);

    /**
     */
    void debug(const char* fmt, ...);

    /**
     */
    void info(const char* fmt, ...);

    /**
     */
    void warn(const char* fmt, ...);

    /**
     */
    void error(const char* fmt, ...);

    /**
     */
    void fatal(const char* fmt, ...);

protected:
    /**
     *  @param appender  The Appender to clear, or 0 for all.
     */
    void clearAppender(const LogAppender* appender=0);

    Logger(Loggers* loggers, Logger* parent, const char* name, int32_t nameLen = 0x7fffffff);

    /**
     *  If level is appropriate, logs s.  Recursively appends to parent.
     */
    void append(Log::Level level, Buffer& s);

    Loggers * const m_loggers;
    Logger* const m_parent;
    Buffer m_name;
    Log::Level m_level;
    Set m_appenders;
};

#if defined(CLC_LOG_LEVEL) && CLC_LOG_LEVEL >= 5
inline void Log::trace(const char* name, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(name, Trace, fmt, ap);
    va_end(ap);
}
inline void Logger::trace(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(clc::Log::Trace, fmt, ap);
    va_end(ap);
}
#else
inline void Log::trace(const char*, const char*, ...) {}
inline void Logger::trace(const char*, ...) {}
#endif

#if defined(CLC_LOG_LEVEL) && CLC_LOG_LEVEL >= 4
inline void Log::debug(const char* name, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(name, Debug, fmt, ap);
    va_end(ap);
}
inline void Logger::debug(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(clc::Log::Debug, fmt, ap);
    va_end(ap);
}
#else
inline void Log::debug(const char*, const char*, ...) {}
inline void Logger::debug(const char*, ...) {}
#endif

#if defined(CLC_LOG_LEVEL) && CLC_LOG_LEVEL >= 3
inline void Log::info(const char* name, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(name, Info, fmt, ap);
    va_end(ap);
}
inline void Logger::info(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(clc::Log::Info, fmt, ap);
    va_end(ap);
}
#else
inline void Log::info(const char*, const char*, ...) {}
inline void Logger::info(const char*, ...) {}
#endif

#if defined(CLC_LOG_LEVEL) && CLC_LOG_LEVEL >= 2
inline void Log::warn(const char* name, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(name, Warn, fmt, ap);
    va_end(ap);
}
inline void Logger::warn(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(clc::Log::Warn, fmt, ap);
    va_end(ap);
}
#else
inline void Log::warn(const char*, const char*, ...) {}
inline void Logger::warn(const char*, ...) {}
#endif

#if defined(CLC_LOG_LEVEL) && CLC_LOG_LEVEL >= 1
inline void Log::error(const char* name, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(name, Error, fmt, ap);
    va_end(ap);
}
inline void Logger::error(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(clc::Log::Error, fmt, ap);
    va_end(ap);
}
#else
inline void Log::error(const char*, const char*, ...) {}
inline void Logger::error(const char*, ...) {}
#endif

#if defined(CLC_LOG_LEVEL) && CLC_LOG_LEVEL >= 0
inline void Log::fatal(const char* name, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(name, Fatal, fmt, ap);
    va_end(ap);
}
inline void Logger::fatal(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(clc::Log::Fatal, fmt, ap);
    va_end(ap);
}
#else
inline void Log::fatal(const char*, const char*, ...) {}
inline void Logger::fatal(const char*, ...) {}
#endif

}

#endif

