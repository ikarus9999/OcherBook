#include <new>

#include "clc/data/Set.h"

namespace clc
{


bool Set::contains(void* o) const
{
    char** items = (char**)m_set.items();
    unsigned int n = m_set.countItems();
    for (unsigned int i = 0; i < n; ++i) {
        if (items[i] == o)
            return true;
    }
    return false;
}


bool Set::add(void* o)
{
    if (!contains(o)) {
        if (m_set.add(o))
            return true;
        throw std::bad_alloc();
    }
    return false;
}


void* Set::remove()
{
    ASSERT(m_set.countItems());
    return m_set.RemoveItem(m_set.countItems()-1);
}


bool Set::removeAll(Set const& s)
{
    bool changed = false;
    char** items = (char**)s.m_set.items();
    unsigned int n = s.m_set.countItems();
    for (unsigned int i = 0; i < n; ++i) {
        changed |= m_set.RemoveItem((void*) items[i]);
    }
    return changed;
}


}

