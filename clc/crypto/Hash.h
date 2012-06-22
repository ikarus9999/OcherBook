#ifndef LIBCLC_HASH_H
#define LIBCLC_HASH_H

#include <stdint.h>


namespace clc
{

/**
 *  Hashes a chunk of data.
 *  @param key
 *  @param len
 *  @return A 32 bit hash value.
 *  @note This hash is not necessarily consistent across different endiannesses.
 */
uint32_t hash(const uint8_t* key, unsigned int len);

inline uint32_t hash(const char* key, unsigned int len) { return hash((const uint8_t*)key, len); }
inline uint32_t hash(const void* key, unsigned int len) { return hash((const uint8_t*)key, len); }

}

#endif
