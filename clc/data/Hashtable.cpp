#include <string.h>
#include <stddef.h>

#include "clc/crypto/Hash.h"
#include "clc/data/Hashtable.h"


namespace clc
{

Hashtable::Hashtable(unsigned int capacity) :
    m_size(capacity),
    m_buckets()
{
    ASSERT(capacity);
    // TODO: optimize
    while(capacity--)
        m_buckets.add(0);
}

Hashtable::~Hashtable()
{
    // Not possible to call derived class's deleteValue at this point.
    clear(false);
}

void Hashtable::clear()
{
    clear(true);
}

void Hashtable::dump()
{
    const int buckets = m_size;
    for (int i = 0; i < buckets; ++i) {
        printf("%p[%d] = ", this, i);
        List* list = (List*)m_buckets.ItemAtFast(i);
        if (list) {
            const int items = list->countItems();
            printf("List %p with %d items\n", list, items);
        } else
            printf("null\n");
    }
}

void Hashtable::clear(bool deleteValues)
{
    const int buckets = m_size;
    for (int i = 0; i < buckets; ++i) {
        List* list = (List*)m_buckets.ItemAtFast(i);
        if (list) {
            const int items = list->countItems();
            for (int j = 0; j < items; ++j) {
                Item* item = (Item*)list->ItemAtFast(j);
                if (deleteValues)
                    deleteValue(item->value);
                deleteItem(item);
            }
            delete list;
            m_buckets.set(i, 0);
        }
    }
}

unsigned int Hashtable::bucketNum(const void* key, size_t len) const
{
    uint32_t h = hash(key, len);
    return h % m_size;
}

List* Hashtable::bucket(const void* key, size_t len) const
{
    return (List*)m_buckets.ItemAtFast(bucketNum(key, len));
}

int Hashtable::findIndex(const List* bucket, const void* key, size_t len) const
{
    // Fastest to append new items on end, but locality suggests that recently appended items
    // are more likely to be referenced again soon, so search from back.
    const unsigned int size = bucket->countItems();
    for (int i = size-1; i >= 0; --i) {
        Item *item = (Item*)bucket->ItemAtFast(i);
        if (item->keyLen == len && memcmp(item->key, key, len) == 0)
            return i;
    }
    return -1;
}

Hashtable::Item* Hashtable::find(const List* bucket, const void* key, size_t len) const
{
    int index = findIndex(bucket, key, len);
    if (index < 0) {
        return (Item*)0;
    }
    return (Item*)bucket->ItemAtFast(index);
}

Hashtable::Item* Hashtable::remove(List* bucket, const void* key, size_t len)
{
    int index = findIndex(bucket, key, len);
    if (index < 0)
        return (Item*)0;
    return (Item*)bucket->remove(index);
}

Hashtable::Item* Hashtable::newItem(const void* key, size_t len, void* value)
{
    Item* i = (Item*)(new char[sizeof(Item)-1 + len]);
    i->value = value;
    i->keyLen = len;
    memcpy(i->key, key, len);
    return i;
}

void Hashtable::deleteItem(Item* i)
{
    delete[] (char*)i;
}

unsigned int Hashtable::size()
{
    unsigned int n = 0;
    const int buckets = m_size;
    for (int i = 0; i < buckets; ++i)
    {
        List* list = (List*)m_buckets.ItemAtFast(i);
        if (list)
            n += list->countItems();
    }
    return n;
}

bool Hashtable::containsKey(const void* key) const
{
    return containsKey(&key, sizeof(void*));
}

bool Hashtable::containsKey(const char *key) const
{
    return containsKey(key, strlen(key)+1);
}

bool Hashtable::containsKey(uint32_t key) const
{
    return containsKey(&key, sizeof(uint32_t));
}

bool Hashtable::containsKey(const void* key, size_t len) const
{
    List* l = bucket(key, len);
    return (l && findIndex(l, key, len) >= 0);
}

void* Hashtable::get(const void* key) const
{
    return get(&key, sizeof(void*));
}

void* Hashtable::get(const char *key) const
{
    return get(key, strlen(key)+1);
}

void* Hashtable::get(uint32_t key) const
{
    return get(&key, sizeof(uint32_t));
}

void* Hashtable::get(const void* key, size_t len) const
{
    List* l = bucket(key, len);
    if (l)
    {
        Item* i = find(l, key, len);
        if (i)
            return i->value;
    }
    return 0;
}

void Hashtable::put(const void* key, void* value)
{
    put(&key, sizeof(void*), value);
}

void Hashtable::put(const char* key, void *value)
{
    put(key, strlen(key)+1, value);
}

void Hashtable::put(uint32_t key, void *value)
{
    put(&key, sizeof(uint32_t), value);
}

void Hashtable::put(const void* key, size_t len, void* value)
{
    unsigned int n = bucketNum(key, len);
    List* l = (List*)m_buckets.ItemAtFast(n);
    if (! l) {
        bool added = m_buckets.ReplaceItem(n, l = new List);
        ASSERT(added); (void)added;
    }
    ASSERT(m_buckets.ItemAtFast(n));
    Item* i = find(l, key, len);
    if (i)
    {
        deleteValue(i->value);
        i->value = value;
    }
    else
    {
        i = newItem(key, len, value);
        l->add(i);
    }
}

void* Hashtable::remove(const void* key)
{
    return remove(&key, sizeof(void*));
}

void* Hashtable::remove(const char *key)
{
    return remove(key, strlen(key)+1);
}

void* Hashtable::remove(uint32_t key)
{
    return remove(&key, sizeof(uint32_t));
}

void* Hashtable::remove(const void* key, size_t len)
{
    void* value = (void*)0;
    List* l = bucket(key, len);
    if (l)
    {
        Item* i = remove(l, key, len);
        if (i)
        {
            value = i->value;
            deleteItem(i);
        }
    }
    return value;
}

HashtableIter::HashtableIter(Hashtable& ht) :
    m_ht(ht)
{
    scan(m_next);
}

bool HashtableIter::hasNext() const
{
    return m_next.item != (Hashtable::Item*)0;
}

void HashtableIter::begin()
{
    m_cur.begin();
    m_next.begin();
    scan(m_next);
    next();
}

Hashtable::ConstItem* HashtableIter::operator*()
{
    return (Hashtable::ConstItem*)m_cur.item;
}

HashtableIter& HashtableIter::operator++()
{
    next();
    return *this;
}

void* HashtableIter::next()
{
    m_cur = m_next;
    if (m_cur.item) {
        scan(m_next);
        return m_cur.item->value;
    }
    return 0;
}

void* HashtableIter::remove()
{
    void* value = 0;
    if (! m_cur.item && ! m_next.item) {
        // Past the end
    } else if (m_cur.item) {
        List* bucket = (List*)m_ht.m_buckets.ItemAtFast(m_cur.bucket);
        Hashtable::Item* item = (Hashtable::Item*)bucket->remove(m_cur.index);
        ASSERT(item == m_cur.item);
        m_cur.index--;
        // list shifted; m_next may now be wrong so rescan
        m_next = m_cur;
        scan(m_next);
        next();
        value = item->value;
        m_ht.deleteItem(item);
    } else {
        // Was prior to beginning.  Nothing to return, but do advance.
        next();
    }
    return value;
}

void HashtableIter::scan(struct Bookmark& bookmark)
{
    const int buckets = m_ht.m_size;
    for ( ; bookmark.bucket < buckets; ++bookmark.bucket) {
        List* list = (List*)m_ht.m_buckets.ItemAtFast(bookmark.bucket);
        if (! list)
            continue;
        bookmark.index++;
        if (bookmark.index < (int)list->countItems()) {
            bookmark.item = (Hashtable::Item*)list->ItemAtFast(bookmark.index);
            return;
        }
        bookmark.index = -1;
    }
    bookmark.item = (Hashtable::Item*)0;
}

}
