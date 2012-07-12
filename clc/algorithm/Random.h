#ifndef LIBCLC_RANDOM_H
#define LIBCLC_RANDOM_H

#include <stdint.h>

namespace clc
{

/**
 *  Random number generator, seeded by the best entropy available on the platform.
 */
class Random
{
public:
    /**
     *  Creates a new random number generator.
     */
    Random();

    bool nextBoolean();

    /**
     *  @param bytes  Buffer to fill with random bytes.
     *  @param n  Size of the buffer.
     */
    void nextBytes(uint8_t* bytes, unsigned int n);

    inline void nextBytes(char* bytes, unsigned int n) { nextBytes((uint8_t*)bytes, n); }

    /**
     *  @return random number uniformly distributed between 0.0 (inclusive) and 1.0 (exclusive)
     */
    double nextDouble();

    /**
     *  @return random number uniformly distributed between 0 (inclusive) and n (exclusive for n>0)
     */
    inline unsigned int nextInt(unsigned int n) { return (unsigned int)(nextDouble()*n); }
};


}

#endif

