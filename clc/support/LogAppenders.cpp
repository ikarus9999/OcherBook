#include <new>

#include "clc/support/LogAppenderFile.h"
#include "clc/storage/Path.h"
#include "clc/storage/Filesystem.h"


namespace clc {

LogAppenderFile::LogAppenderFile(const char* filename, bool async) :
#ifndef SINGLE_THREADED
    Thread("LogAppenderFile"),
    m_async(async),
    m_queue(100),
#endif
    m_file(Path::join(getLogDirectory().c_str(), filename), "w+")
{
#ifndef SINGLE_THREADED
    if (async)
        start();
#endif
}

LogAppenderFile::~LogAppenderFile()
{
    detach();
#ifndef SINGLE_THREADED
    if (isAlive()) {
        sync();
        join();
    }
#endif
}

void LogAppenderFile::setAsync(bool async)
{
#ifndef SINGLE_THREADED
    lock();
    m_async = async;
    if (async && !isAlive()) {
        start();
        unlock();
    } else if (!async && isAlive()) {
        sync();
        unlock();
        join();
    }
#endif
}

#ifndef SINGLE_THREADED
void LogAppenderFile::sync()
{
    ASSERT(isAlive());
    interrupt();
    Buffer s;
    while (1) {
        try {
            m_queue.put(s);
        } catch (std::bad_alloc&) {
            if (isAlive()) {
                Thread::_sleepUSec(100);
                continue;
            }
        }
        break;
    }
}

void LogAppenderFile::run()
{
    bool i;
    do {
        while (!(i=isInterrupted()) || m_queue.length()) {
            Buffer s = m_queue.take();
            m_file.write(s.c_str(), s.length());
        }
        m_file.flush();
    } while (!i);
}
#endif

void LogAppenderFile::append(Buffer& s)
{
#ifdef SINGLE_THREADED
    m_file.write(s.c_str(), s.length());
    m_file.flush();
#else
    lock();
    if (m_async) {
        try {
            m_queue.put(s);
        } catch (std::bad_alloc&) {
            // sorry, was best-effort
        }
        unlock();
    }
    else {
        m_file.write(s.c_str(), s.length());
        unlock();
        m_file.flush();
    }
#endif
}

}
