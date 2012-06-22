#ifndef LIBCLC_DATA_HASHTABLE_H
#define LIBCLC_DATA_HASHTABLE_H

#include <stdint.h>

#include "clc/data/Buffer.h"
#include "clc/data/Iterator.h"
#include "clc/data/List.h"


namespace clc
{

class HashtableIter;

/**
 *  A hashtable.
 */
class Hashtable
{
public:
    friend class HashtableIter;

    void dump();

    struct ConstItem {
        char* value;
        const size_t keyLen;
        const unsigned char key[1];
    };
    struct Item {
        void* value;
        size_t keyLen;
        unsigned char key[1];
    };

    Hashtable(unsigned int capacity=100);
    virtual ~Hashtable();
    unsigned int size();
    virtual bool containsKey(const void* key) const;
    virtual bool containsKey(const void* key, size_t len) const;
    virtual bool containsKey(const char *key) const;
    virtual bool containsKey(uint32_t key) const;
    /**
     *  @throw bad_alloc
     */
    virtual void put(const void* key, void* value);
    /**
     *  @throw bad_alloc
     */
    virtual void put(const void* key, size_t len, void* value);
    /**
     *  @throw bad_alloc
     */
    virtual void put(const char* key, void* value);
    /**
     *  @throw bad_alloc
     */
    virtual void put(uint32_t key, void *value);
    virtual void* get(const void* key) const;
    virtual void* get(const void* key, size_t len) const;
    virtual void* get(const char* key) const;
    virtual void* get(uint32_t key) const;
    virtual void* remove(const void* key);
    virtual void* remove(const void* key, size_t len);
    virtual void* remove(const char* key);
    virtual void* remove(uint32_t key);

    /**
     *  Same as calling remove(key) for every key, which causes deleteValue to be called for
     *  every value.
     */
    void clear();

protected:
    void clear(bool deleteValues);

    /**
     *  Override in derived class to delete values stored in the Hashtable, in the following
     *  situations:
     *  <ul>
     *    <li>put() which replaces an old object</li>
     *    <li>remaining objects when Hashtable is destroyed<li>
     *  </ul>
     */
    virtual void deleteValue(void* value) const { (void)value; };

private:
    /**
     *  @return Index of bucket for this key.
     */
    unsigned int bucketNum(const void* key, size_t len) const;

    /**
     *  @return Bucket for this key, or 0 if bucket has not yet been created.
     */
    List* bucket(const void* key, size_t len) const;

    /**
     *  @return Index of the Item, or <0 if not found.
     */
    int findIndex(const List* bucket, const void* key, size_t len) const;

    /**
     *  Finds the specified Item in the bucket.
     *  @return The Item, or NULL if not found.
     */
    Item* find(const List* bucket, const void* key, size_t len) const;

    /**
     *  Removes the specified Item from the bucket.
     *  @return The Item, or NULL if not found.
     */
    Item* remove(List* bucket, const void* key, size_t len);

    static Item* newItem(const void* key, size_t len, void* value);
    static void deleteItem(Item* i);

    unsigned int m_size;
    clc::List m_buckets;  ///< Of List of Item*

    // Unimplemented
    Hashtable(const Hashtable&);
    Hashtable& operator=(const Hashtable&);
};


/**
 *  Specialization of Hashtable to store Buffers.
 */
class BufferHashtable : public Hashtable
{
public:
    BufferHashtable(unsigned int capacity=100) : Hashtable(capacity) { }
    ~BufferHashtable() { clear(); }

    /**
     *  @throw bad_alloc
     */
    void put(const void* key, const Buffer& value) { Hashtable::put(&key, sizeof(void*), (void*)new Buffer(value)); }
    /**
     *  @throw bad_alloc
     */
    void put(const void* key, size_t len, const Buffer& value) { Hashtable::put(key, len, (void*)new Buffer(value)); }
    /**
     *  @throw bad_alloc
     */
    void put(const char* key, const Buffer& value) { Hashtable::put(key, strlen(key)+1, (void*)new Buffer(value)); }
    /**
     *  @throw bad_alloc
     */
    void put(uint32_t key, const Buffer& value) { Hashtable::put(&key, sizeof(uint32_t), (void*)new Buffer(value)); }
    /**
     *  @throw bad_alloc
     */
    void put(const Buffer& key, const Buffer& value) { Hashtable::put(key, (void*)new Buffer(value)); }
    void put(const Buffer& key, Buffer* value) { Hashtable::put(key, (void*)value); }
    Buffer* getBuffer(const void* key) const { return (Buffer*)Hashtable::get(key); };
    Buffer* getBuffer(const void* key, size_t len) const { return (Buffer*)Hashtable::get(key, len); };
    Buffer* getBuffer(const char* key) const { return (Buffer*)Hashtable::get(key); };
    Buffer* getBuffer(uint32_t key) const { return (Buffer*)Hashtable::get(key); };
    Buffer* getBuffer(const Buffer& key) const { return (Buffer*)Hashtable::get(key); };
    Buffer* removeBuffer(const void* key) { return (Buffer*)Hashtable::remove(key); }
    Buffer* removeBuffer(const void* key, size_t len) { return (Buffer*)Hashtable::remove(key, len); }
    Buffer* removeBuffer(const char* key) { return (Buffer*)Hashtable::remove(key); }
    Buffer* removeBuffer(uint32_t key) { return (Buffer*)Hashtable::remove(key); }

protected:
    void deleteValue(void* s) const { delete (Buffer*)s; }

private:
    /**
     *  Override merely to disallow.  This class handles object lifecycle; bare pointer with no
     *  length makes no sense.
     */
    void put(const void*, void*) { ASSERT(0); }
    void put(const void*, size_t, void*) { ASSERT(0); }
    void put(const char*, void*) { ASSERT(0); }
    void put(uint32_t, void*) { ASSERT(0); }
};


class HashtableIter : public Iterator
{
public:
    HashtableIter(Hashtable& h);

    /**
     * Returns true if there are more items in the iterator.
     *
     * @return bool
     */
    bool hasNext() const;

    /**
     * Returns the next value in the hashtable.
     *
     * @return The next value in the hashtable, or 0 if at the end.
     */
    void* next();

    /**
     *  Removes the current item, and advances to the next item.
     *  @return The value associated with the current item, or 0 if no current item.
     */
    void* remove();

    /**
     *  Resets the iterator to the beginning.
     */
    void begin();

    /**
     *  @return pointer to the current item, or 0 if past end.
     */
    Hashtable::ConstItem* operator*();

    /**
     *  Moves to next item in Hashtable.
     */
    HashtableIter& operator++();

protected:
    struct Bookmark {
        Bookmark() : bucket(0), index(-1), item(0) {}
        void begin() { bucket = 0; index = -1; item = 0; }
        int bucket;
        int index;
        Hashtable::Item* item;
    };

    /**
     *  Scans forward from the bookmark to find the next item, and sets the bookmark there.
     */
    void scan(struct Bookmark& bookmark);

    struct Bookmark m_cur;
    struct Bookmark m_next;
    Hashtable& m_ht;
};

}

#endif

