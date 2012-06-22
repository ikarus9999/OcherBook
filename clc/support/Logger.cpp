#include <memory>

#include "clc/os/Clock.h"
#include "clc/support/LogAppenders.h"
#include "clc/support/Logger.h"
#ifndef SINGLE_THREADED
#include "clc/os/Monitor.h"
#include "clc/os/ThreadLocal.h"
#endif


namespace clc
{

Loggers Log::loggers;


Loggers::LoggerHashtable::~LoggerHashtable()
{
    clear();
}

void Loggers::LoggerHashtable::putLogger(const void* key, size_t len, Logger* l)
{
    try {
        Hashtable::put(key, len, (void*)l);
    } catch(std::bad_alloc&) {
        // sorry, was best-effort
        delete l;
    }
}

Logger* Loggers::LoggerHashtable::getLogger(const void* key, size_t len) const
{
    return (Logger*)Hashtable::get(key, len);
}

void Loggers::LoggerHashtable::deleteValue(void* value) const
{
    delete (Logger*)value;
}

Loggers::Loggers()
{
    m_init = true;
    setRoot();
}

Loggers::~Loggers()
{
    clear();
    m_init = false;
}

void Loggers::clearAppenderUnchecked(const LogAppender* logAppender)
{
    HashtableIter iter(m_loggers);
    while (iter.hasNext())
        ((Logger*)iter.next())->clearAppender(logAppender);
}

void Loggers::clear()
{
    if (m_init) {
#ifndef SINGLE_THREADED
        m_rwlock.writeLock();
#endif
        // Break ties to all LogAppenders, so that destruction order of statics does not matter.
        clearAppenderUnchecked(0);
        m_loggers.clear();
#ifndef SINGLE_THREADED
        m_rwlock.unlock();
#endif
    }
}

void Loggers::clearAppender(const LogAppender* logAppender)
{
    if (m_init) {
#ifndef SINGLE_THREADED
        m_rwlock.writeLock();
#endif
        clearAppenderUnchecked(logAppender);
#ifndef SINGLE_THREADED
        m_rwlock.unlock();
#endif
    }
}

void Loggers::setRoot()
{
    // Calling setRoot from another static is bad because I can't guarantee ordering.  Asserting
    // because I can't honor the contract.  Just don't do it.
    ASSERT(m_init);
    if (m_init) {
        Logger* root = new Logger(this, 0, "", 0);
        root->setLevel(Log::Warn);
#ifndef SINGLE_THREADED
        m_rwlock.writeLock();
#endif
        put(root);
#ifndef SINGLE_THREADED
        m_rwlock.unlock();
#endif
        ASSERT(m_loggers.getLogger("", 0));
    }
}

/**
 *  writelock must already be held.
 */
void Loggers::put(Logger* l)
{
    m_loggers.putLogger(l->getName().c_str(), l->getName().length(), l);
}

Logger* Loggers::get(const char* name)
{
    if (! m_init)
        return (Logger*)0;
    unsigned int nameLen = strlen(name);
#ifndef SINGLE_THREADED
    m_rwlock.readLock();
#endif
    Logger* logger = m_loggers.getLogger(name, nameLen);
#ifndef SINGLE_THREADED
    m_rwlock.unlock();
#endif
    if (! logger) {
        ASSERT(*name);  // Root logger should have been created in setRoot.
#ifndef SINGLE_THREADED
        m_rwlock.writeLock();
#endif
        Logger* parent = m_loggers.getLogger("", 0);
        ASSERT(parent);
        const char* end;
        unsigned int searchOffset = 0;
        do {
            unsigned int subnameLen;
            end = strchr(name+searchOffset, '.');
            if (end) {
                subnameLen = end - name;
                searchOffset = subnameLen + 1;
            } else {
                subnameLen = nameLen;
            }
            logger = m_loggers.getLogger(name, subnameLen);
            if (! logger) {
                logger = new Logger(this, parent, name, subnameLen);
                put(logger);
            }
            parent = logger;
        } while (end);
#ifndef SINGLE_THREADED
        m_rwlock.unlock();
#endif
    }
    return logger;
}


void Log::reset()
{
    loggers.clear();
    loggers.setRoot();
}

Logger* Log::get(const char* name)
{
    return loggers.get(name);
}

void Log::log(const char* name, Log::Level level, const char* fmt, va_list args)
{
    get(name)->log(level, fmt, args);
}

void Log::log(const char* name, Log::Level level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    get(name)->log(level, fmt, ap);
    va_end(ap);
}


Logger::Logger(Loggers* loggers, Logger* parent, const char* name, int32_t nameLen) :
    m_loggers(loggers),
    m_parent(parent),
    m_name(name, nameLen),
    m_level(Log::Unset)
{
}

void Logger::setLevel(Log::Level level)
{
    if (m_parent || level != Log::Unset)  // Can't unset root logger
        m_level = level;
}

Log::Level Logger::getLevel() const
{
    if (m_level != Log::Unset)
        return m_level;
    return m_parent->getLevel();
}

Logger* Logger::getParent()
{
    return m_parent;
}

const Buffer Logger::getName() const
{
    return m_name;
}

void Logger::setAppender(LogAppender* a)
{
    ASSERT(a);
#ifndef SINGLE_THREADED
    m_loggers->wlock();
#endif
    m_appenders.add(a);
    a->setLoggers(m_loggers);
#ifndef SINGLE_THREADED
    m_loggers->unlock();
#endif
}

void Logger::clearAppender(const LogAppender* logAppender)
{
    if (logAppender) {
        m_appenders.remove((void*)logAppender);
    } else {
        while (m_appenders.size()) {
            LogAppender* a = (LogAppender*)m_appenders.remove();
            a->setLoggers(0);
        }
    }
}

void Logger::append(Log::Level level, Buffer& s)
{
#ifndef SINGLE_THREADED
    m_loggers->rlock();
#endif
    SetIterator si(m_appenders);
    while (si.hasNext())
        ((LogAppender*)si.next())->append(s);
#ifndef SINGLE_THREADED
    m_loggers->unlock();
#endif
    if (m_parent)
        m_parent->append(level, s);
}

static const char* levelStr[] = {
    "",
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

void Logger::log(Log::Level level, const char* fmt, va_list ap)
{
    // Check for and warn about recursive logging.
#ifdef DEBUG
#ifdef SINGLE_THREADED
    static int recursive = 0;
    if (recursive) {
        printf("Attempted recursive use of the logger\n");
        return;
    }
    recursive = 1;
#else
    static ThreadLocal recursive;
    if (recursive.get()) {
        printf("Attempted recursive use of the logger\n");
        return;
    }
    recursive.set((void*)1);
#endif
#endif
    try {
        if (this && getLevel() <= level) {
            uint64_t usec64 = Clock::monotonicUSec();
            Buffer s;
            unsigned int usec = (unsigned int)(usec64 % 1000000);
            unsigned int sec = (unsigned int)(usec64 / 1000000);
            unsigned int min = sec / 60;
            sec = sec % 60;
            unsigned int hour = min / 60;
            min = min % 60;
            s.format("%02d:%02d:%02d.%06d %-5s %-10s ", hour, min, sec, usec, levelStr[level], m_name.c_str());
            s.appendFormatList(fmt, ap);
            s += "\n";
            append(level, s);
        }
    } catch (...) {
        // Logging is best effort.
    }
#ifdef DEBUG
#ifdef SINGLE_THREADED
    recursive = 0;
#else
    recursive.set((void*)0);
#endif
#endif
}

void Logger::log(Log::Level level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log(level, fmt, ap);
    va_end(ap);
}

}
