#ifndef LIBCLC_LOCK_H
#define LIBCLC_LOCK_H

#ifndef SINGLE_THREADED
#include "clc/support/Debug.h"

#ifdef USE_LIBTASK
#include <task.h>
#elif defined(__BEOS__) || defined(__HAIKU__)
#include <be/support/Locker.h>
#else
#include <pthread.h>
#endif
#endif

namespace clc
{

#ifndef SINGLE_THREADED
/**
 *  A lock (binary semaphore).
 *  @note  For an exception-safe version of the lock (locks only a scope), see clc::Locker.
 */
class Lock {
public:
    /**
     *  Constructor.  Lock starts out unlocked.
     *  @throws std::bad_alloc
     */
    Lock();

    /**
     *  Destructor.  Behavior is undefined if the lock is still locked.
     */
    ~Lock();

    /**
     *  Locks the lock, blocking until it can be acquired.
     */
    void lock();

    /**
     *  Locks the lock, blocking until it can be acquired or until the timeout expires.
     *  @param usec  Maximum time to block while attempting to acquire the lock.  0 implies
     *      no blocking, and so only succeeds if the lock is immediately available.
     *  @return  true if lock acquired, else false.
     */
    bool lockWithTimeout(unsigned int usec);

    /**
     *  Unlocks the lock.  The lock must be locked by the current thread (one thread is not allowed
     *  to unlock on behalf of another.)
     */
    void unlock();

protected:
#ifdef USE_LIBTASK
    QLock m_lock;
#elif defined(__BEOS__) || defined(__HAIKU__)
    BLocker m_lock;
#else
    pthread_mutex_t m_lock;
#endif

private:
    // Unimplemented
    Lock(Lock const&);
    Lock& operator=(Lock const&);
};

inline void Lock::lock()
{
#ifdef USE_LIBTASK
    qlock(&m_lock);
#elif defined(__BEOS__) || defined(__HAIKU__)
    m_lock.Lock();
#else
    int r = pthread_mutex_lock(&m_lock);
    ASSERT(r == 0);(void)r;
#endif
}

inline void Lock::unlock()
{
#ifdef USE_LIBTASK
    qunlock(&m_lock);
#elif defined(__BEOS__) || defined(__HAIKU__)
    m_lock.Unlock();
#else
    int r = pthread_mutex_unlock(&m_lock);
    ASSERT(r == 0);(void)r;
#endif
}
#else
class Lock {
public:
    void lock() {}
    bool lockWithTimeout(unsigned int) { return true; }
    void unlock() {}
};
#endif


#ifndef SINGLE_THREADED
/**
 *  Like clc::Lock, but constructor automatically locks and destructor unlocks.  Useful for
 *  protecting scopes in an exception-safe manner.
 */
class Locker
{
public:
    Locker(Lock& lock) : m_lock(lock) { m_lock.lock(); }
    ~Locker() { m_lock.unlock(); }
protected:
    Lock& m_lock;
private:
    // Unimplemented
    Locker(Locker const&);
    Locker& operator=(Locker const&);
};
#else
class Locker
{
public:
    Locker(Lock&) {}
};
#endif

}

#endif

