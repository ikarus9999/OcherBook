#ifndef LIBCLC_DATA_SET_H
#define LIBCLC_DATA_SET_H

#include "clc/support/Debug.h"
#include "clc/data/Iterator.h"
#include "clc/data/List.h"


namespace clc
{

class SetIterator;


/**
 *  Loosely based on java.util.Set.
 */
class Set
{
public:
    friend class SetIterator;
    friend class SetMutatingIterator;

    Set() {}

    Set(unsigned int capacity_hint) : m_set(capacity_hint) {}

    bool contains(void* o) const;

    bool add(void* o);

    inline void clear() { m_set.clear(); }

    /**
     *  Removes an arbitrary element.  Implementation is free to select the element based on
     *  efficiency.
     */
    void* remove();

    /**
     *  Removes a particular element.
     *  @return True iff the element was found.
     */
    inline bool remove(void* o) { return m_set.RemoveItem(o); }

    bool removeAll(Set const& s);

    inline unsigned int size() const { return m_set.countItems(); }

protected:
    List m_set;
};


class SetIterator : public Iterator
{
public:
    SetIterator(Set const& set) :
        m_set(set),
        m_next(0)
    {
    }

    bool hasNext() const
    {
        return m_next < m_set.size();
    }

    /**
     *  Returns the next element in the iteration.
     *  @return Non-null iff there was a next element
     */
    virtual void* next()
    {
        if (hasNext()) {
            return m_set.m_set.ItemAtFast(m_next++);
        }
        return 0;
    }

    virtual void reset()
    {
        m_next = 0;
    }

protected:
    Set const& m_set;
    unsigned int m_next;
};


class SetMutatingIterator : public SetIterator
{
public:
    SetMutatingIterator(Set& set) :
        SetIterator(set),
        m_mutableSet(set)
    {
    }

    virtual void* remove()
    {
        if (! m_next) {
            ASSERT(0);  // should have called next() to get started
            return 0;
        }
        m_next--;
        return m_mutableSet.m_set.RemoveItem(m_next);
    }

private:
    Set& m_mutableSet;
};


}

#endif
