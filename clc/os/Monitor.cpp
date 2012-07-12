#include <string.h>
#include <time.h>
#include <errno.h>

#include "clc/os/Clock.h"
#include "clc/os/Monitor.h"
#include "clc/support/Debug.h"
#include "clc/support/Exception.h"


namespace clc
{

Monitor::Monitor() : m_locked(false), m_waiting(false)
{
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    memset(&m_r, 0, sizeof(m_r));
#else
//    m_lock = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
    int r;
    r = pthread_mutex_init(&m_lock, NULL);
    ASSERT(r == 0);
    r = pthread_cond_init(&m_cond, NULL);
    ASSERT(r == 0);
#endif
}


Monitor::~Monitor()
{
    ASSERT(!m_locked);
    ASSERT(!m_waiting);
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    // nothing to do
#else
    int r;
    r = pthread_cond_destroy(&m_cond);
    ASSERT(r == 0);
    r = pthread_mutex_destroy(&m_lock);
    ASSERT(r == 0);
#endif
}


void
Monitor::notify()
{
    ASSERT(m_locked);
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    taskwakeup(&m_r);
#else
    int r;
    r = pthread_cond_signal(&m_cond);
    ASSERT(r == 0);
#endif
}


void
Monitor::notifyAll()
{
    ASSERT(m_locked);
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    taskwakeupall(&m_r);
#else
    int r;
    r = pthread_cond_broadcast(&m_cond);
    ASSERT(r == 0);
#endif
}


void
Monitor::wait(unsigned int timeoutUsec)
{
    ASSERT(m_locked);
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    m_waiting = true;
    m_locked = false;
    tasksleep(&m_r);
    m_locked = true;
    m_waiting = false;
#else
    m_locked = false;
    m_waiting = true;
    int r;
    if (timeoutUsec) {
        struct timespec absTime = Clock::futureUsec(timeoutUsec);
        r = pthread_cond_timedwait(&m_cond, &m_lock, &absTime);
        ASSERT(r == 0 || r == ETIMEDOUT);
    } else {
        r = pthread_cond_wait(&m_cond, &m_lock);
        ASSERT(r == 0);
    }
    m_locked = true;
    m_waiting = false;
#endif
}

bool Monitor::tryLock()
{
    bool lockAcquired = false;
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    // TODO
#else
    int r = pthread_mutex_trylock(&m_lock);
    if (r == 0) {
        m_locked = true;
        lockAcquired = true;
    } else {
        ASSERT(r == EBUSY);
    }
#endif
    return lockAcquired;
}

void Monitor::lock()
{
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    // nothing to do
#else
    int r;
    r = pthread_mutex_lock(&m_lock);
    ASSERT(r == 0);
#endif
    m_locked = true;
}

void Monitor::unlock()
{
    ASSERT(m_locked);
    m_locked = false;
#if defined(__BEOS__) || defined(__HAIKU__)
    // TODO
#elif defined(USE_LIBTASK)
    // nothing to do
#else
    int r;
    r = pthread_mutex_unlock(&m_lock);
    ASSERT(r == 0);
#endif
}

}
