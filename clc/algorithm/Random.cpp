#include <stdlib.h>
#include <unistd.h>

#include "clc/algorithm/Random.h"
#include "clc/os/Clock.h"
#include "clc/os/Lock.h"
#include "clc/support/Debug.h"
#include "clc/support/Intrinsics.h"

/*
 *  Random number generation has plenty of pitfalls.  Watch out for these:
 *  - Seeding based on time() is portable, but is too predictable.  Use something with higher
 *    resolution and/or more randomness.
 *  - Avoid constraining a random number to a range using modulus, for two reasons:  The resulting
 *    distribution may no longer be uniform, and some RNG implementations have less randomness in the
 *    lower bits.
 *  - Avoid rand() if possible, since some implementations of rand() are very bad -- little state,
 *    short periods, predictable lower bits.
 */

namespace clc
{

Random::Random()
{
    static bool initialized = false;
    static clc::Lock randomLock;
    if (UNLIKELY(! initialized)) {
        randomLock.lock();
        if (! initialized) {
#if defined(__MACH__)
            srandomdev();  // Uses /dev/random; more secure than using time
#else
            unsigned long seed = (unsigned long)clc::Clock::monotonicUSec();
            seed += getpid();
            srandom(seed);
#endif
            initialized = true;
        }
        randomLock.unlock();
    }
}


/**
 *  @return Random number between 0 and RAND_MAX, inclusive.
 */
static inline long getRandom(void)
{
    return random();
}

/**
 *  Returns a uniformly distributed number between 0.0 and 1.0, inclusive.
 */
static double uniform_deviate()
{
    return getRandom() * ( 1.0 / ( RAND_MAX + 1.0 ) );
}

bool Random::nextBoolean()
{
    return uniform_deviate() < 0.5;
}

double Random::nextDouble()
{
    double d;
    while (UNLIKELY((d = uniform_deviate()) == 1.0))
        ;
    return d;
}

void Random::nextBytes(uint8_t* bytes, unsigned int len)
{
    unsigned long r;
    unsigned long b = 0;
    while (len) {
        if ((b & 0xff) != 0xff) {
            r = getRandom();
            b = RAND_MAX;
        }
        *bytes++ = r & 0xff;
        if (--len == 0)
            break;
        r >>= 8;
        b >>= 8;
    }
}

}
