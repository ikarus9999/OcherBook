#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clc/support/Debug.h"
#include "clc/data/List.h"


namespace clc
{

static inline void moveItems(void** items, size_t offset, size_t count)
{
    if (count > 0 && offset != 0)
        memmove(items + offset, items, count * sizeof(void*));
}


List::List(size_t capacity) :
    m_objectList(0),
    m_allocated(0),
    m_itemCount(0),
    m_blockSize(capacity),
    m_resizeThreshold(0)
{
    if (!m_blockSize)
        m_blockSize = 1;
    _resizeArray(m_itemCount);
}


List::List(const List& anotherList) :
    m_objectList(0),
    m_allocated(0),
    m_itemCount(0),
    m_blockSize(anotherList.m_blockSize)
{
    *this = anotherList;
}


List::~List()
{
    free(m_objectList);
}


List& List::operator =(const List &list)
{
    m_blockSize = list.m_blockSize;
    _resizeArray(list.m_itemCount);
    m_itemCount = list.m_itemCount;
    memcpy(m_objectList, list.m_objectList, m_itemCount * sizeof(void*));
    return *this;
}


void* List::remove()
{
    if (m_itemCount)
        return remove(m_itemCount-1);
    return 0;
}

bool List::addAt(void *item, size_t index)
{
    if (index > m_itemCount) {
        ASSERT(0);
        return false;
    }

    bool result = true;

    if (m_itemCount + 1 > m_allocated)
        result = _resizeArray(m_itemCount + 1);
    if (result) {
        ++m_itemCount;
        moveItems(m_objectList + index, 1, m_itemCount - index - 1);
        m_objectList[index] = item;
    }
    return result;
}


bool List::add(void *item)
{
    bool result = true;
    if (m_allocated > m_itemCount) {
        m_objectList[m_itemCount] = item;
        ++m_itemCount;
    } else {
        if ((result = _resizeArray(m_itemCount + 1))) {
            m_objectList[m_itemCount] = item;
            ++m_itemCount;
        }
    }
    return result;
}


bool List::AddList(const List *list, size_t index)
{
    bool result = (list && index <= m_itemCount);
    if (result && list->m_itemCount > 0) {
        size_t count = list->m_itemCount;
        if (m_itemCount + count > m_allocated)
            result = _resizeArray(m_itemCount + count);
        if (result) {
            m_itemCount += count;
            moveItems(m_objectList + index, count, m_itemCount - index - count);
            memcpy(m_objectList + index, list->m_objectList,
                   list->m_itemCount * sizeof(void *));
        }
    }
    return result;
}


bool List::AddList(const List *list)
{
    bool result = (list != 0);
    if (result && list->m_itemCount > 0) {
        size_t index = m_itemCount;
        size_t count = list->m_itemCount;
        if (m_itemCount + count > m_allocated)
            result = _resizeArray(m_itemCount + count);
        if (result) {
            m_itemCount += count;
            memcpy(m_objectList + index, list->m_objectList,
                   list->m_itemCount * sizeof(void *));
        }
    }
    return result;
}


List* List::split(size_t index)
{
    List* l = new List;
    split(index, l);
    return l;
}


bool List::split(size_t index, List* tail)
{
    bool result = true;
    tail->m_itemCount = 0;
    if (index < m_itemCount) {
        size_t n = m_itemCount - index;
        if (tail->m_allocated < n) {
            result = tail->_resizeArray(n);
        }
        if (result) {
            tail->m_itemCount = n;
            memcpy(tail->m_objectList, m_objectList+n, n*sizeof(void*));
            m_itemCount -= n;
        }
    }
    return result;
}


bool List::RemoveItem(void *item)
{
    size_t index = indexOf(item);
    if (index != NotFound)
        RemoveItem(index);
    return (index != NotFound);
}


void* List::RemoveItem(size_t index)
{
    void *item = 0;
    if (index < m_itemCount) {
        item = m_objectList[index];
        moveItems(m_objectList + index + 1, -1, m_itemCount - index - 1);
        --m_itemCount;
        if (m_itemCount <= m_resizeThreshold)
            _resizeArray(m_itemCount);
    }
    return item;
}


bool List::RemoveItems(size_t index, size_t count)
{
    bool result = index <= m_itemCount;
    if (result) {
        if (index + count > m_itemCount)
            count = m_itemCount - index;
        moveItems(m_objectList + index + count, -count,
                   m_itemCount - index - count);
        m_itemCount -= count;
        if (m_itemCount <= m_resizeThreshold)
            _resizeArray(m_itemCount);
    }
    return result;
}


bool List::ReplaceItem(size_t index, void *newItem)
{
    bool result = false;

    if (index < m_itemCount) {
        m_objectList[index] = newItem;
        result = true;
    }
    return result;
}


void List::clear()
{
    m_itemCount = 0;
    _resizeArray(0);
}


void List::sortItems(int (*compareFunc)(const void *, const void *))
{
    if (compareFunc)
        qsort(m_objectList, m_itemCount, sizeof(void *), compareFunc);
}


bool List::SwapItems(size_t indexA, size_t indexB)
{
    bool result = false;

    if (indexA < m_itemCount && indexB < m_itemCount) {
        void *tmpItem = m_objectList[indexA];
        m_objectList[indexA] = m_objectList[indexB];
        m_objectList[indexB] = tmpItem;

        result = true;
    }

    return result;
}


bool List::MoveItem(size_t fromIndex, size_t toIndex)
{
    if ((fromIndex >= m_itemCount) || (toIndex >= m_itemCount))
        return false;

    if (fromIndex < toIndex)
    {
        void * tmp_mover = m_objectList[fromIndex];
        memmove(m_objectList + fromIndex + 1, m_objectList + fromIndex, (toIndex - fromIndex) * sizeof(void *));
        m_objectList[toIndex] = tmp_mover;
    }
    else if (fromIndex > toIndex)
    {
        void * tmp_mover = m_objectList[fromIndex];
        memmove(m_objectList + toIndex + 1, m_objectList + toIndex, (fromIndex - toIndex) * sizeof(void *));
        m_objectList[toIndex] = tmp_mover;
    };
    return true;
}


void* List::itemAt(int index) const
{
    void *item;
    if (index < 0)
        index = (int)m_itemCount + index;
    if (index >= 0 && (size_t)index < m_itemCount)
        item = m_objectList[index];
    else
        item = 0;
    return item;
}


void* List::firstItem() const
{
    void *item = 0;
    if (m_itemCount > 0)
        item = m_objectList[0];
    return item;
}


void* List::lastItem() const
{
    void *item = 0;
    if (m_itemCount > 0)
        item = m_objectList[m_itemCount - 1];
    return item;
}


size_t List::indexOf(void *item) const
{
    for (size_t i = 0; i < m_itemCount; i++) {
        if (m_objectList[i] == item)
            return i;
    }
    return NotFound;
}


void List::DoForEach(bool (*func)(void* item))
{
    if (func != 0) {
        bool terminate = false;
        size_t index = 0;
        while ((!terminate) && (index < m_itemCount)) {
            terminate = func(m_objectList[index]);
            index++;
        };
    }

}


void List::DoForEach(bool (*func)(void* item, void* arg), void * arg)
{
    if (func != 0) {
        bool terminate = false;
        size_t index = 0;
        while ((!terminate) && (index < m_itemCount)) {
            terminate = func(m_objectList[index], arg);
            index++;
        };
    }
}

bool List::_resizeArray(size_t count)
{
    bool result = true;
    // calculate the new physical size
    // by doubling the existing size
    // until we can hold at least count items
    size_t newSize = m_allocated > 0 ? m_allocated : m_blockSize;
    size_t targetSize = count;
    if (targetSize > m_allocated) {
        while (newSize < targetSize)
            newSize <<= 1;
    } else if (targetSize <= m_resizeThreshold) {
        newSize = m_resizeThreshold;
    }
    // Never go down to 0, because realloc of 0 frees.
    if (newSize == 0)
        newSize = 1;

    // resize if necessary
    if (newSize != m_allocated) {
        void** newObjectList = (void**)realloc(m_objectList, newSize * sizeof(void*));
        if (newObjectList) {
            m_objectList = newObjectList;
            m_allocated = newSize;
            // set our lower bound to either 1/4
            //of the current physical size, or 0
            m_resizeThreshold = m_allocated >> 2 >= m_blockSize ? m_allocated >> 2 : 0;
        } else {
            result = false;
        }
    }
    return result;
}

}

