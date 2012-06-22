#ifndef LIBCLC_DATA_LIST_H
#define LIBCLC_DATA_LIST_H

/** @file
 *  Light-weight list of pointers.
 *
 *  Derived from the Haiku project.  Original copyright notice:
 *  Copyright 2001-2007, Haiku, Inc. All Rights Reserved.
 *  Distributed under the terms of the MIT License.
 */

#include <stdint.h>
#include <stdlib.h>

namespace clc
{

/**
 *  @todo  replace bool return with throw
 */
class List
{
public:
    List(size_t capacity = 16);
    List(const List& anotherList);
    virtual ~List();

    List& operator =(const List &);

    /* Adding and removing items. */
    bool addAt(void* item, size_t index);
    bool add(void* item);
    bool AddList(const List* list, size_t index);
    bool AddList(const List* list);
    List* split(size_t index);
    bool split(size_t index, List* tail);
    bool RemoveItem(void* item);
    void* RemoveItem(size_t index);
    bool RemoveItems(size_t index, size_t count);
    bool ReplaceItem(size_t index, void* newItem);
    void clear();

    inline void set(size_t index, void* newItem) { ReplaceItem(index, newItem); }
    inline void* remove(size_t index) { return RemoveItem(index); }
    void* remove();

    // Reorder items
    void sortItems(int (*compareFunc)(const void*, const void*));
    bool SwapItems(size_t indexA, size_t indexB);
    /**
     * This moves a list item from posititon a to position b, moving the appropriate block of
     *  list elements to make up for the move.  For example, in the array:
     *  A B C D E F G H I J
     *  Moving 1(B)->6(G) would result in this:
     * A C D E F G B H I J
     */
    bool MoveItem(size_t fromIndex, size_t toIndex);

    // Retrieve items
    /**
     * @param index  0..size-1, or -1..-size from end
     * @return Pointer at that index, or NULL if invalid index
     */
    void* itemAt(int index) const;
    void* get(size_t index) const { return itemAt(index); }
    void* firstItem() const;
    void* ItemAtFast(size_t index) const { return m_objectList[index]; }
    void* lastItem() const;
    void* items() const { return m_objectList; }

    // Query
    bool hasItem(void* item) const { return (indexOf(item) != NotFound); }
    size_t indexOf(void* item) const;
    size_t countItems() const { return m_itemCount; }
    size_t size() const { return m_itemCount; }
    size_t length() const { return m_itemCount; }
    bool isEmpty() const { return (m_itemCount == 0); }

    // Iteration
    /**
     *  iterate a function over the whole list.  If the function outputs a true
     * value, then the process is terminated.
     */
    void DoForEach(bool (*func)(void* item));
    void DoForEach(bool (*func)(void* item, void* arg), void *arg);

private:
    /**
     *  Resizes m_objectList to be large enough to contain count items.
     *  m_itemCount is adjusted accordingly.
     */
    bool _resizeArray(size_t count);

    void** m_objectList;
    size_t m_allocated;
    size_t m_itemCount;
    size_t m_blockSize;
    size_t m_resizeThreshold;

public:
    /**
     * Special error value.
     */
    static const size_t NotFound = ((size_t)-1);

};


}

#endif
