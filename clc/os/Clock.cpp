#if defined(__BEOS__) || defined(__HAIKU__)
#include <kernel/OS.h>
#else
#if defined(__MACH__)
#include <mach/mach_time.h>
#else
#define __USE_POSIX199309
#include <time.h>
#endif
#include <sys/time.h>
#endif

#include "clc/os/Clock.h"
#include "clc/support/Debug.h"
#include "clc/support/Intrinsics.h"
#ifndef SINGLE_THREADED
#include "clc/os/Lock.h"
#endif


namespace clc
{

#if !defined(__BEOS__) && !defined(__HAIKU__)

#ifndef SINGLE_THREADED
Lock clockLock;
#endif

#if defined(__MACH__)
uint64_t Clock::s_monStart;
mach_timebase_info_data_t Clock::s_monFreq;
#else
#ifndef HAVE_NO_RT
bool Clock::s_usingMon;
#endif
#endif

#if !defined(HAVE_NO_RT) || defined(__MACH__)
/**
 *  Initializes the static parts of Clock.
 */
class ClockStarter : public Clock
{
public:
    ClockStarter()
    {
#if defined(__MACH__)
        s_monStart = mach_absolute_time();
        mach_timebase_info(&s_monFreq);
#else
        s_usingMon = true;  // Until discover otherwise
#endif
    }
} g_clockStarter;
#endif

#endif


uint64_t Clock::nowUSec()
{
#if defined(__BEOS__) || defined(__HAIKU__)
    return real_time_clock_usecs();
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    uint64_t sec = tv.tv_sec;
    uint64_t usec = tv.tv_usec;
    return usec + (sec*1000000);
#endif
}

uint64_t Clock::monotonicUSec()
{
#if defined(__MACH__)
    uint64_t now = mach_absolute_time();
    uint64_t elapsedNano = (now-s_monStart) * s_monFreq.numer / s_monFreq.denom;
    return elapsedNano / 1000;
#elif defined(__BEOS__) || defined(__HAIKU__)
    return system_time();
#else
#ifndef HAVE_NO_RT
    if (s_usingMon)
    {
        struct timespec ts;
        int r = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (LIKELY(r == 0))
        {
            uint64_t sec = ts.tv_sec;
            uint64_t nsec = ts.tv_nsec;
            return nsec/1000 + sec*1000000;
        }
        // Should never fail after first use, so don't have to worry about mismatched epochs.
        s_usingMon = false;
    }
#endif

    // Monotonic time is not natively available.  Use nowUSec, but guard against the clock going
    // backwards.
    {
        static uint64_t previousTime;
        static uint64_t diff;
        uint64_t now = nowUSec();
#ifndef SINGLE_THREADED
        clockLock.lock();
#endif
        if (UNLIKELY(now < previousTime))
            diff += previousTime - now;
        previousTime = now;
        now += diff;
#ifndef SINGLE_THREADED
        clockLock.unlock();
#endif
        return now;
    }
#endif
}

#if !defined(__BEOS__) && !defined(__HAIKU__)
void Clock::futureUsec(unsigned int usec, struct timespec* ts)
{
    while (usec > 1000000U) {
        usec -= 1000000U;
        ts->tv_sec ++;
    }
    ts->tv_nsec += usec * 1000;
    if (ts->tv_nsec > 1000000000) {
        ts->tv_nsec -= 1000000000;
        ts->tv_sec ++;
    }
}

struct timespec Clock::futureUsec(unsigned int usec, const struct timeval* tv)
{
    struct timespec ts;
    ts.tv_sec = tv->tv_sec;
    ts.tv_nsec = tv->tv_usec * 1000;
    futureUsec(usec, &ts);
    return ts;
}

struct timespec Clock::futureUsec(unsigned int usec)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return futureUsec(usec, &tv);
}
#endif

}
