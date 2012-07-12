#ifndef LIBCLC_LOG_APPENDERS_H
#define LIBCLC_LOG_APPENDERS_H

#include <stdio.h>

#include "clc/support/Logger.h"
#include "clc/support/Debug.h"


namespace clc
{

class Logger;


/**
 *  Appender that discards messages.
 */
class LogAppenderNull : public LogAppender
{
public:
    void append(Buffer&) {}
};


/**
 *  Appender that sends the messages to the debugger (if any).
 */
class LogAppenderDebugger : public LogAppender
{
public:
    void append(Buffer& s) {
        Debugger::printf(s);
    }
};


/**
 *  Appender that appends to C-style FILE objects, such as stdout.
 */
class LogAppenderCFile : public LogAppender
{
public:
    LogAppenderCFile(FILE* f) : m_f(f) {}
    void append(Buffer& s) {
        fprintf(m_f, "%s", s.c_str());
    }

protected:
    FILE* m_f;
};


/**
 *  Appender that collects messages in memory, and can optionally flush them to another
 *  appender.
 */
class LogAppenderMemory : public LogAppender
{
public:
    // TODO
};

}

#endif

