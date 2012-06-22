#ifndef LIBCLC_DEBUG_H
#define LIBCLC_DEBUG_H

/** @file
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


namespace clc
{

class Buffer;

class Debugger
{
public:
    /**
     *  @return  Arbitrary value set by debugger; normally 0
     */
    static int enter();

    static void print(char const* str);

    static void printf(const Buffer& b);

    static void printf(const char* fmt, va_list ap);

    static void printf(char const* fmt, ...);

    static int asserted(char const* file, int line, char const* expr);

    /**
     * Names the thread, so that they name is visible in the debugger.
     * @note only in debug builds
     */
    static void nameThread(const char* name);
};


}


#if DEBUG
    #define ASSERT(E)  do{if(!(E)) clc::Debugger::asserted(__FILE__, __LINE__, #E);}while(0)

    #define ASSERT_WITH_MESSAGE(E, msg) \
                       do{if(!(E)) clc::Debugger::asserted(__FILE__, __LINE__, msg);}while(0)

    #define DEBUG_ONLY(arg)  arg

#else
    #define ASSERT(E)                    (void)0
    #define ASSERT_WITH_MESSAGE(E, msg)  (void)0
    #define DEBUG_ONLY(x)
#endif

/**
 *  STATIC_ASSERT is a compile-time check that can be used to
 *  verify static expressions such as: STATIC_ASSERT(sizeof(int64) == 8);
 */
#define STATIC_ASSERT(x)                                \
    do {                                                \
        struct __staticAssertStruct__ {                 \
            char __static_assert_failed__[2*(x) - 1];   \
        };                                              \
    } while (0)


#endif  /* _DEBUG_H */
