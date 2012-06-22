#ifndef LIBCLC_SUPPORT_LOG_APPENDER_FILE_H
#define LIBCLC_SUPPORT_LOG_APPENDER_FILE_H

#include "clc/storage/File.h"
#include "clc/support/LogAppenders.h"
#ifndef SINGLE_THREADED
#include "clc/data/BlockingQueue.h"
#include "clc/os/Thread.h"
#include "clc/os/Lock.h"
#endif

namespace clc
{

/**
 *  Appender that appends to clc::File objects, with optional threading for guaranteed
 *  non-blocking operation.
 */
class LogAppenderFile : public LogAppender
#ifndef SINGLE_THREADED
                        , public Thread
#endif
{
public:
    /**
     *  @param filename  The filename to log to.  Relative paths will be interpreted relative to
     *      the standard logging directory.
     *  @param async  Whether writes are done asynchronously (on a background thread if threaded,
     *      or ) or synchonously.
     */
    LogAppenderFile(const char* filename, bool async
#ifdef SINGLE_THREADED
            =false
#endif
            );
    ~LogAppenderFile();
    /**
     *  When changing from async to blocking, does not return until all pending writes are
     *  completed.
     */
    void setAsync(bool async);
    void append(Buffer& s);
protected:
#ifndef SINGLE_THREADED
    void sync();
    void run();
    bool m_async;
    Lock m_lock;
    BufQueue m_queue;
    inline void lock() { m_lock.lock(); }
    inline void unlock() { m_lock.unlock(); }
#else
    inline void lock() {}
    inline void unlock() {}
#endif
    File m_file;
};

}

#endif

