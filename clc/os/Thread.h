#ifndef LIBCLC_THREAD_H
#define LIBCLC_THREAD_H

#ifdef USE_LIBTASK
#include <task.h>
#elif defined(__BEOS__) || defined(__HAIKU__)
#include <kernel/OS.h>
#else
#include <pthread.h>
typedef pthread_t thread_id;
#endif

#include "clc/data/Buffer.h"
#include "clc/os/Monitor.h"
#include "clc/support/Debug.h"


namespace clc
{

#ifdef USE_LIBTASK
    typedef void thread_status_t;
    typedef void* thread_id;
#elif defined(__BEOS__) || defined(__HAIKU__)
    typedef status_t thread_status_t;
#else
    typedef void* thread_status_t;
    typedef pthread_t thread_id;
#endif

/**
 *  @todo  is daemon a good idea?
 */
class Thread
{
public:

    /**
     *  Creates a thread in the JOINED state.
     */
    Thread();

    /**
     *  Creates a named thread in the JOINED state.
     */
    Thread(char const* fmt, ...);

    /**
     *  The thread must be in the JOINED state before being destructed.
     */
    virtual ~Thread();

    /**
     *  @return The thread ID corresponding to this Thread object.
     */
    thread_id getThreadId();

    /**
     *  @return The thread ID corresponding to the currently running thread.
     */
    static thread_id currentThreadId();

    /**
     *  Waits until the thread's run() routine has returned, and joins with it.
     *  After a thread is joined, it can be deleted or restarted.
     *  A thread may be joined multiple times (that is, joining is not racy and is idempotent).
     *  Attempting to join a daemon thread is undefined.
     */
    void join();

    /**
     */
    const Buffer getName() const;

    /**
     */
    void setDaemon(bool d);

    /**
     */
    void setName(char const* fmt, ...);

    /**
     *  Starts the thread.
     *  @throw std::bad_alloc if thread cannot be started.
     */
    void start();

    /**
     *  @return True iff the thread is STARTING or RUNNING.
     */
    bool isAlive() const;

    /**
     *  @return true if the thread is still running but has been interrupted.
     */
    bool isInterrupted() const;

    /**
     *  Sets a flag; the derived Thread class is expected to check this flag at convenient points
     *  and exit if it is set.
     */
    void interrupt();

    /**
     */
    void sleep(unsigned int ms);

    /**
     */
    static void _sleepUSec(unsigned int usec);

    /**
     *  Yields the thread, giving all other threads a chance to run.
     */
    void yield();

    static void yieldCurrent();

protected:
    virtual void run() = 0;

    void init();

    static thread_status_t bootstrap(void* _self);

    enum State
    {
        STARTING,
        RUNNING,
        STOPPED,
        JOINED
    };

    State m_state;
    Monitor m_stateChange;

    enum Flags
    {
        DAEMON      = 1,
        INTERRUPTED = 2,
    };
    unsigned int m_flags;

    Buffer m_name;

    thread_id m_thread;

private:
    /** Not implemented; Thread is not copyable. */
    Thread(const Thread&);
    /** Not implemented; Thread is not copyable. */
    Thread& operator=(const Thread&);
};


#if defined(USE_LIBTASK)
inline thread_id Thread::currentThreadId() { return (Thread*)*taskdata(); }
inline void Thread::yield() { ASSERT(currentThreadId() == m_thread); taskyield(); }
inline void Thread::yieldCurrent() { taskyield(); }
#elif defined(__BEOS__) || defined(__HAIKU__)
inline thread_id Thread::currentThreadId() { return find_thread(NULL); }
inline void Thread::yield() { ASSERT(currentThreadId() == m_thread); snooze(1); }
inline void Thread::yieldCurrent() { snooze(1); }
#else
inline thread_id Thread::currentThreadId() { return pthread_self(); }
inline void Thread::yield() { ASSERT(currentThreadId() == m_thread); sched_yield(); }
inline void Thread::yieldCurrent() { sched_yield(); }
#endif
inline bool Thread::isAlive() const { return m_state == STARTING || m_state == RUNNING; }
inline bool Thread::isInterrupted() const { return m_flags & INTERRUPTED; }

}

#endif

