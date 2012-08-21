#ifndef LIBCLC_ITERATOR_H
#define LIBCLC_ITERATOR_H

#include "clc/support/Debug.h"


namespace clc
{

/**
 *  An iterator (aka generator).
 */
class Iterator
{
public:
    virtual ~Iterator() {}

    /**
     *  @return true if the iteration has more elements.
     */
    virtual bool hasNext() const
    {
        return false;
    }

    /**
     *  Returns the next element in the iteration (potentially yielding
     *  the Task to get it).
     */
    virtual void* next()
    {
        ASSERT(0);  // hasNext was false, so don't call
        return 0;
    }

    virtual void* remove()
    {
        ASSERT(0);  // Derived class should implement, or don't call
        return 0;
    }
};

}

#endif
