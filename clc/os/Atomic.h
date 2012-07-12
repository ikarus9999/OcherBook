#ifndef LIBCLC_ATOMIC_H
#define LIBCLC_ATOMIC_H

/** @file
 *  Atomic operations.  Unless specified otherwise by "NoBarrier" in the name, these operations
 *  act as full memory barriers.
 *
 *  In general, these functions return the value prior to the modification.  TODO:  Consider
 *  changing to return the value POST modification.  Easier to be portable.
 *
 *  Where possible, these are implemented using the OS-supplied atomics, since they may be patched
 *  at runtime to be optimal for the particular platform.
 */

#if defined(__BEOS__) || defined(__HAIKU__)
#include <support/SupportDefs.h>
#else

#include <stdint.h>

inline int32_t atomicGet(int32_t *p)
{
    return *p;
}

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) // (HAVE_GCC_SYNC_FETCH_AND_ADD)

/*
 * @note http://gcc.gnu.org/onlinedocs/gcc-4.3.3/gcc/Atomic-Builtins.html#Atomic-Builtins
 */
inline int32_t atomicAdd(int32_t *p, int32_t v)
{
    return __sync_fetch_and_add(p, v);
}
inline bool atomicExchange(int32_t* p, int32_t oldVal, int32_t newVal)
{
    return __sync_bool_compare_and_swap(p, oldVal, newVal);
}

#elif defined(__NetBSD__)

extern "C" {
#include <atomic.h>
}
inline int32_t atomicAdd(int32_t *p, int32_t v)
{
    return atomic_add_32_nv((uint32_t*)p, v) - v;
}

#elif defined(__FreeBSD__)

#include <machine/atomic.h>

inline int32_t atomicAdd(int32_t *p, int32_t v)
{
    // TODO: racy
    int32_t prev = *p;
    atomic_add_32((uint32_t*)p, v);
    return prev - v;
}

#elif defined(__MACH__)

#include <libkern/OSAtomic.h>
inline int32_t atomicAdd(int32_t *p, int32_t v)
{
    return OSAtomicAdd32Barrier(v, p) - v;
}
inline bool atomicExchange(int32_t* p, int32_t oldVal, int32_t newVal)
{
    return OSAtomicCompareAndSwap32Barrier(oldVal, newVal, p);
}

#elif defined(__i386__)

inline int32_t atomicAdd(int32_t *p, int32_t v)
{
    __asm__ (
    "lock xaddl %0, %1 ;"
    : "+r" (v),                     /* 0 (result) */
      "=m" (*p)                     /* 1 */
    : "m" (*p));                    /* 2 */
    return (v);
}
inline bool atomicExchange(int32_t* p, int32_t oldVal, int32_t newVal)
{
    // TODO
    __asm__ volatile("xchgl %0,%1" : "+m" (*p), "+r" (newVal));
    return true;
}

#elif defined(__mips__)

inline int32_t atomicAdd(int32_t *p, int32_t v)
{
    int32_t temp = 0;
    __asm__ __volatile (
        "    .set mips3\n"
        "1:  ll %0, %1   # loads value of p into memory, and sets the link bit\n"
        "    addu %0, %2 # add the content of op 2 to op 0\n"
        "    sc %0, %1  # as long as things haven't changed, will store the result back into p\n"
        "    beqz %0, 1b\n"
        "    .set mips0\n"
        : "=r" (temp), "=m" (*p)
        : "Ir" (v), "m" (*p));
    return (temp-v);
}

inline bool atomicExchange(int32_t *p, int32_t oldVal, int32_t newVal)
{
    int32_t temp1 = 0; // will hold our result
    __asm__ __volatile (
        "    .set mips3\n"
        "1:  ll   %0, %1     # loads the value p into temp1, and sets the link bit\n"
        "    bne  %0, %2, 2f # if p != oldVal, then don't swap the values\n"
        "    addu %0, %3, $0 # add newVal with 0 and store in temp1\n"
        "    sc   %0, %1     # as long as things haven't changed, store newVal in p\n"
        "    beqz %0, 1b     # if the store didn't succeed, try again\n"
        "    j    3f         # jump out\n"
        "2:  li   %0, 0      # put a 0 indicating we didn't swap\n"
        "3:  .set mips0\n"
        : "=r" (temp1), "=m" (*p)
        : "r" (oldVal), "r" (newVal), "m" (*p)
        : "memory"
        );
    return temp1 == 1;
}

#else

#error Define atomics for your platform

#endif
#endif

#define atomicIncrement(p) atomicAdd(p, 1)
#define atomicDecrement(p) atomicAdd(p, -1)

#endif
