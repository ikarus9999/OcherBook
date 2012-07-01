#include "clc/crypto/MurmurHash2.h"

namespace clc
{

/**
 *  MurmurHash2, public domain by Austin Appleby
 *  @url http://murmurhash.googlepages.com
 *
 *  Requirements when selecting a hash:
 *   - Fast
 *     (murmur is 10% faster than jenkins lookup3; 5x faster than SHA1)
 *   - Small code size; prefer no lookup tables
 *     (murmur can be as small as 52 bytes)
 *   - Excellent distribution, since may be "hashing" a single 4-byte pointer
 *     (http://murmurhash.googlepages.com/avalanche)
 *  Non-requirements:
 *   - Deterministic results across different platforms
 */
uint32_t hash(const uint8_t* data, unsigned int len)
{
    // Could let the caller choose a seed, but not currently, for API simplicity.
    uint32_t seed = ~0;

    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value.
    uint32_t h = seed ^ len;

    // Mix 4 bytes at a time into the hash.
    while (len >= 4) {
        uint32_t k = *(uint32_t *)data;
        data += 4;
        len -= 4;

        h *= m;
        k *= m;

        k ^= k >> r;
        k *= m;

        h ^= k;
    }

    // Handle the last few bytes of the input array.
    switch(len) {
    case 3:
        h ^= data[2] << 16;
    case 2:
        h ^= data[1] << 8;
    case 1:
        h ^= data[0];
        h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

}
