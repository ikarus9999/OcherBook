#ifndef LIBCLC_MONITOR_H
#define LIBCLC_MONITOR_H

#include "clc/support/Debug.h"
#if defined(__BEOS__) || defined(__HAIKU__)
#include <be/kernel/OS.h>
#elif defined(USE_LIBTASK)
#include <task.h>
#else
#include <pthread.h>
#endif


namespace clc
{

/**
 *  A simple monitor (aka condition variable).
 */
class Monitor
{
public:
    Monitor();

    ~Monitor();

    /**
     *  Attempts to lock the Monitor, without blocking.  If the lock is available, locks it and
     *  returns true.  If the lock is busy, immediately returns false.
     *  @return True iff the lock was acquired.
     */
    bool tryLock();

    /**
     */
    void lock();

    /**
     */
    void unlock();

    /**
     *  Wakes up a single thread that is waiting on this monitor.  The lock must be held.  The
     *  awoken thread will not proceed until the lock is released.
     */
    void notify();

    /**
     *  Wakes up all threads that are waiting on this monitor.  The lock must be held.  The
     *  awoken threads will not proceed until the lock is released.
     */
    void notifyAll();

    /**
     *  Waits until notified, or the timeout expires, or (on some platforms) until a spurious
     *  wakeup occurs.  Waiters must be able to handle spurious wakeups.
     *  @param timeoutUsec  Maximum microseconds to wait.  0 implies waiting until notified.
     */
    void wait(unsigned int timeoutUsec=0);

private:
    bool m_locked;
    bool m_waiting;  // for debugging
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    struct Rendez m_r;
#else
    pthread_mutex_t m_lock;
    pthread_cond_t m_cond;
#endif
};

}

#endif
