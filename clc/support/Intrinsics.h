#ifndef LIBCLC_INTRINSICS_H
#define LIBCLC_INTRINSICS_H

#if __GNUC__ >= 3
#define LIKELY(cond)    __builtin_expect(!!(cond),1)
#define UNLIKELY(cond)  __builtin_expect(!!(cond),0)
#else
#define LIKELY(cond)   (cond)
#define UNLIKELY(cond) (cond)
#endif

#if __GNUC__ >= 4
#define CLC_PUBLIC    __attribute__((visibility("default")))
#define CLC_PRIVATE   __attribute__((visibility("hidden")))
#else
#define CLC_PUBLIC
#define CLC_PRIVATE
#endif
#ifdef UNIT_TESTING
#define CLC_PROTECTED CLC_PUBLIC
#else
#define CLC_PROTECTED CLC_PRIVATE
#endif

#endif

