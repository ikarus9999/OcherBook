#ifndef LIBCLC_OS_CLOCK_H
#define LIBCLC_OS_CLOCK_H

/** @file
 *  Cross-platform clock class.
 */

#include <stdint.h>
#ifdef __MACH__
#include <mach/mach_time.h>
#endif


namespace clc
{

/**
 *  Cross-platform clock class.
 */
class Clock
{
public:
    /**
     *  Returns number of microseconds since the UNIX epoch of Jan 1 1970 00:00:00 UTC.
     *  @note The time returned can be ambiguous (consider leap seconds) and discontinuous
     *      (because of leap seconds and/or resetting of the system clock).
     *  @return  Microseconds since UNIX epoch
     */
    static uint64_t nowUSec();

    static unsigned long now() { return nowUSec() / 1000000U; }

    /**
     *  Returns the number of microseconds that have elapsed since some fixed point in the past.
     *  @return Monotonically increasing count of microseconds
     */
    static uint64_t monotonicUSec();

    /**
     *  Calculates a time in the future.
     *  @param usec  Number of microseconds in the future
     *  @param ts  Modified
     */
    static void futureUsec(unsigned int usec, struct timespec* ts);

#if !defined(__BEOS__) && !defined(__HAIKU__)
    /**
     *  Adds time to a timeval, returning a timespec.
     *  @param usec  Number of microseconds in the future
     *  @param tv  The base time, as a timeval
     *  @return The future time, as a timespec
     */
    static struct timespec futureUsec(unsigned int usec, const struct timeval* tv);

    /**
     *  Calculates a time in the future, relative to the current realtime (ie, wall) clock.
     *  @param usec  Number of microseconds in the future
     *  @return The future time
     */
    static struct timespec futureUsec(unsigned int usec);
#endif

protected:
#if defined(__MACH__)
    static uint64_t s_monStart;
    static mach_timebase_info_data_t s_monFreq;
#elif !defined(__BEOS__) && !defined(__HAIKU__)
    static bool s_usingMon;
#endif
};

}

#endif
