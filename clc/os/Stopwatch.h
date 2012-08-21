#ifndef LIBCLC_STOPWATCH_H
#define LIBCLC_STOPWATCH_H

#include <stdint.h>

#include "clc/os/Clock.h"


namespace clc
{

class Stopwatch
{
public:
    Stopwatch(bool autoStart=true) : m_startUSec(0), m_elapsedUSec(0)
    {
        if (autoStart)
            start();
    }

    /**
     *  If stopwatch is curently running, elapsed time is since start.
     *  If stopwatch is curently stopped, elapsed time is the previous run (0 if none).
     *  @return Elapsed time, in microseconds.
     */
    uint64_t elapsedUSec()
    {
        if (m_startUSec)
            return Clock::monotonicUSec() - m_startUSec;
        else
            return m_elapsedUSec;
    }

    uint64_t lap()
    {
        if (m_startUSec) {
            uint64_t now = Clock::monotonicUSec();
            uint64_t elapsed = now - m_startUSec;
            m_startUSec = now;
            return elapsed;
        } else {
            start();
            return 0;
        }
    }

    void start()
    {
        m_startUSec = Clock::monotonicUSec();
        m_elapsedUSec = 0;
    }

    /**
     *  Stops the stopwatch, if it is running.
     *  @return Elapsed time, in microseconds, of the most recent run.
     */
    uint64_t stop()
    {
        if (m_startUSec) {
            m_elapsedUSec = Clock::monotonicUSec() - m_startUSec;
            m_startUSec = 0;
        }
        return m_elapsedUSec;
    }

    bool running()
    {
        return m_startUSec != 0;
    }

protected:
    uint64_t m_startUSec;
    uint64_t m_elapsedUSec;
};

}

#endif
