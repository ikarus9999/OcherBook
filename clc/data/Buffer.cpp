#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <new>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/param.h>

#include "clc/data/Buffer.h"
#include "clc/support/Debug.h"
#ifndef SINGLE_THREADED
#include "clc/os/Atomic.h"
#endif

namespace clc
{

// define proper names for case-option of _DoReplace()
#define KEEP_CASE false
#define IGNORE_CASE true

// define proper names for count-option of _DoReplace()
#define REPLACE_ALL 0x7FFFFFFF

#undef max
#define max(a,b) ((a)>(b)?(a):(b))

const size_t kPrivateDataOffset = sizeof(int32_t) + sizeof(size_t);


// helper function, returns minimum of two given values (but clamps to 0):
static inline size_t
min_clamp0(size_t num1, size_t num2)
{
    if (num1 < num2)
        return num1 > 0 ? num1 : 0;

    return num2 > 0 ? num2 : 0;
}


// helper function, massages given pointer into a legal c-string:
static inline const char *
safestr(const char* str)
{
    return str ? str : "";
}

static int memncmp(const void* m1, const void* m2, size_t n1, size_t n2)
{
    size_t min = min_clamp0(n1, n2);
    int r = memcmp(m1, m2, min);
    if (r || n1 == n2)
        return r;
    if (n1 < n2)
        return -1;
    return 1;
}

static inline int32_t&
referenceCount(char* privateData)
{
    return *(int32_t*)(privateData - sizeof(size_t) - sizeof(int32_t));
}

inline int32_t&
Buffer::refCount()
{
    return referenceCount(m_data);
}


inline const int32_t&
Buffer::refCount() const
{
    return referenceCount(m_data);
}

inline int32_t Buffer::incRef()
{
#ifdef SINGLE_THREADED
    return ++refCount();
#else
    return atomicAdd(&refCount(), 1) + 1;
#endif
}

inline int32_t Buffer::decRef()
{
#ifdef SINGLE_THREADED
    return --refCount();
#else
    return atomicAdd(&refCount(), -1) - 1;
#endif
}

static inline void
setLength(size_t len, char* privateData)
{
    *(((size_t*)privateData) - 1) = len;
}

inline void
Buffer::_SetLength(size_t len)
{
    setLength(len, m_data);
}



inline bool
Buffer::_IsShareable() const
{
    return refCount() >= 0;
}


class Buffer::PosVect
{
public:
    PosVect() :
        fSize(0),
        fBufferSize(20),
        fBuffer(NULL) {}

    ~PosVect()
    {
        free(fBuffer);
    }

    void Add(size_t pos)
    {
        if (fBuffer == NULL || fSize == fBufferSize) {
            if (fBuffer != NULL)
                fBufferSize *= 2;

            size_t* newBuffer = NULL;
            newBuffer = (size_t *)realloc(fBuffer, fBufferSize * sizeof(size_t));
            if (newBuffer == NULL)
                throw std::bad_alloc();

            fBuffer = newBuffer;
        }

        fBuffer[fSize++] = pos;
    }

    inline size_t ItemAt(size_t index) const
    { return fBuffer[index]; }
    inline size_t CountItems() const
    { return fSize; }

private:
    size_t  fSize;
    size_t  fBufferSize;
    size_t* fBuffer;
};


Buffer::Buffer()
{
    _Init("", 0);
}


Buffer::Buffer(const char* string)
{
    _Init(string, strlen(safestr(string)));
}


Buffer::Buffer(const Buffer& string)
{
    // check if source is sharable - if so, share else clone
    if (string._IsShareable()) {
        m_data = string.m_data;
        incRef();
    } else {
        _Init(string.c_str(), string.length());
    }
}


Buffer::Buffer(const char* string, size_t len)
{
    _Init(string, len);
}


Buffer::~Buffer()
{
    if (!_IsShareable() || decRef() == 0)
        _FreePrivateData();
}


size_t
Buffer::countChars() const
{
    size_t count = 0;

    const char *start = m_data;
    const char *end = m_data + length();

    while (start++ != end) {
        count++;
        // Jump to next UTF8 character
        for (; (*start & 0xc0) == 0x80; start++)
            ;
    }

    return count;
}


Buffer&
Buffer::operator=(const Buffer& string)
{
    return setTo(string);
}


Buffer&
Buffer::operator=(const char* string)
{
    if (!string)
        clear();
    else if (string != c_str())
        setTo(string, strlen(string));
    return *this;
}


Buffer&
Buffer::operator=(char c)
{
    return setTo(c, 1);
}

Buffer&
Buffer::formatList(const char* fmt, va_list argList)
{
#ifdef __NetBSD__   // TODO:  HAVE_VASPRINTF
    char* buf;
    vasprintf(&buf, fmt, argList);
    size_t len = strlen(buf);
    _DetachWith(buf, len, len);
    free(buf);
#else
    va_list argList2;
#if defined(__va_copy) && !defined(va_copy)
#define va_copy __va_copy
#endif
    va_copy(argList2, argList);
    int len = vsnprintf(NULL, 0, fmt, argList2) + 1;  // measure,
    va_end(argList2);
    char* buf = (char*)alloca(len);
    int printed = vsnprintf(buf, len, fmt, argList);  // format,
    ASSERT(printed+1 == len); (void)printed;
    _DetachWith(buf, len-1, len-1);  // copy
#endif
    return *this;
}


Buffer&
Buffer::format(const char* fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);
    formatList(fmt, argList);
    va_end(argList);
    return *this;
}


Buffer&
Buffer::appendFormatList(const char* fmt, va_list argList)
{
#ifdef __NetBSD__   // TODO:  HAVE_VASPRINTF
    char* buf;
    vasprintf(&buf, fmt, argList);
    _DoAppend(buf, strlen(buf) - 1);
    free(buf);
#else
    va_list argList2;
    va_copy(argList2, argList);
    int len = vsnprintf(NULL, 0, fmt, argList2) + 1;  // measure,
    va_end(argList2);
    char* buf = (char*)alloca(len);
    int printed = vsnprintf(buf, len, fmt, argList);  // format,
    ASSERT(printed+1 == len); (void)printed;
    _DoAppend(buf, len - 1);  // copy
#endif
    return *this;
}

Buffer&
Buffer::appendFormat(const char* fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);
    appendFormatList(fmt, argList);
    va_end(argList);
    return *this;
}


Buffer&
Buffer::setTo(const char* string, size_t len)
{
    _DetachWith(string, len, len);
    return *this;
}


Buffer&
Buffer::setTo(const Buffer& string)
{
    // we share the information already
    if (m_data == string.m_data)
        return *this;

    bool freeData = true;
    if (_IsShareable() && decRef() > 0) {
        // there is still someone who shares our data
        freeData = false;
    }

    if (freeData)
        _FreePrivateData();

    // if source is sharable share, otherwise clone
    if (string._IsShareable()) {
        m_data = string.m_data;
        incRef();
    } else
        _Init(string.c_str(), string.length());
    return *this;
}


Buffer&
Buffer::adopt(Buffer& from)
{
    setTo(from);
    from.setTo("");

    return *this;
}


Buffer&
Buffer::setTo(const Buffer& string, size_t len)
{
    size_t dataLen = min_clamp0(len, string.length());
    if (m_data != string.m_data) {
        _DetachWith(string.c_str(), dataLen, len);
    } else if (length() > len) {
        // need to truncate
        _DetachWith("", 0, len);
        memmove(m_data, string.c_str(), dataLen);
    }
    return *this;
}


Buffer&
Buffer::adopt(Buffer& from, size_t len)
{
    setTo(from, len);
    from.setTo("");
    return *this;
}


Buffer&
Buffer::setTo(char c, size_t count)
{
    _DetachWith("", 0, count);
    memset(m_data, c, count);
    return *this;
}


Buffer&
Buffer::CopyInto(Buffer& into, size_t fromOffset, size_t len) const
{
    if (this != &into)
        into.setTo(m_data + fromOffset, len);
    return into;
}


void
Buffer::CopyInto(char* into, size_t fromOffset, size_t len) const
{
    if (into) {
        len = min_clamp0(len, length() - fromOffset);
        memcpy(into, m_data + fromOffset, len);
    }
}


Buffer&
Buffer::operator+=(const char* string)
{
    size_t len = strlen(string);
    _DoAppend(string, len);
    return *this;
}


Buffer&
Buffer::operator+=(char c)
{
    _DoAppend(&c, 1);
    return *this;
}


Buffer&
Buffer::append(const Buffer& string, size_t len)
{
    if (&string != this) {
        len = min_clamp0(len, string.length());
        _DoAppend(string.m_data, len);
    }
    return *this;
}


Buffer&
Buffer::append(const char* string, size_t len)
{
    _DoAppend(string, len);
    return *this;
}


Buffer&
Buffer::append(char c, size_t count)
{
    if (count > 0) {
        size_t oldLength = length();
        _DetachWith(m_data, oldLength, oldLength + count);
        memset(m_data + oldLength, c, count);
    }
    return *this;
}


Buffer&
Buffer::prepend(const char* string)
{
    _DoPrepend(string, strlen(string));
    return *this;
}


Buffer&
Buffer::prepend(const Buffer& string)
{
    if (&string != this)
        _DoPrepend(string.c_str(), string.length());
    return *this;
}


Buffer&
Buffer::prepend(const char* string, size_t len)
{
    _DoPrepend(string, len);
    return *this;
}


Buffer&
Buffer::prepend(const Buffer& string, size_t len)
{
    if (&string != this)
        _DoPrepend(string.m_data, min_clamp0(len, string.length()));
    return *this;
}


Buffer&
Buffer::prepend(char c, size_t count)
{
    if (count > 0) {
        size_t oldLength = length();
        _DetachWith(m_data, oldLength, oldLength + count);
        memmove(m_data + count, m_data, oldLength);
        memset(m_data, c, count);
    }
    return *this;
}


Buffer&
Buffer::insert(const char* string, ssize_t position)
{
    if (position < 0 || (size_t)position <= length()) {
        size_t len = strlen(string);
        if (position < 0) {
            size_t skipLen = min_clamp0(-1 * position, len);
            string += skipLen;
            len -= skipLen;
            position = 0;
        } else {
            position = min_clamp0(position, length());
        }
        _DoInsert(string, position, len);
    }
    return *this;
}


Buffer&
Buffer::insert(const char* string, size_t len, ssize_t position)
{
    if (position < 0 || (size_t)position <= length()) {
        if (position < 0) {
            size_t skipLen = min_clamp0(-1 * position, len);
            string += skipLen;
            len -= skipLen;
            position = 0;
        } else {
            position = min_clamp0(position, length());
        }
        _DoInsert(string, position, len);
    }
    return *this;
}


Buffer&
Buffer::insert(const char* string, size_t fromOffset, size_t len, ssize_t position)
{
    insert(string + fromOffset, len, position);
    return *this;
}


Buffer&
Buffer::insert(const Buffer& string, ssize_t position)
{
    if (&string != this && string.length() > 0)
        insert(string.m_data, position);
    return *this;
}


Buffer&
Buffer::insert(const Buffer& string, size_t len, ssize_t position)
{
    if (&string != this && string.length() > 0)
        insert(string.c_str(), len, position);
    return *this;
}


Buffer&
Buffer::insert(const Buffer& string, size_t fromOffset, size_t len, ssize_t position)
{
    if (&string != this && string.length() > 0)
        insert(string.c_str() + fromOffset, len, position);
    return *this;
}


Buffer&
Buffer::insert(char c, size_t count, ssize_t position)
{
    if (position < 0) {
        count = max(count + position, 0);
        position = 0;
    } else
        position = min_clamp0(position, length());

    if (count > 0) {
        size_t oldLength = length();
        _DetachWith(m_data, oldLength, oldLength + count);
        memmove(m_data + position + count, m_data + position, oldLength - position);
        memset(m_data + position, c, count);
    }

    return *this;
}


Buffer&
Buffer::truncate(size_t newLength)
{
    if (newLength < length())
        _DetachWith(m_data, newLength, newLength);

    return *this;
}


Buffer&
Buffer::remove(size_t from, size_t len)
{
    if (len > 0 && from < length())
        _ShrinkAtBy(from, min_clamp0(len, (length() - from)));
    return *this;
}


Buffer&
Buffer::RemoveFirst(const Buffer& string)
{
    if (string.length() > 0) {
        size_t pos = _ShortFindAfter(string.c_str(), string.length());
        if (pos != NotFound)
            _ShrinkAtBy(pos, string.length());
    }
    return *this;
}


Buffer&
Buffer::RemoveLast(const Buffer& string)
{
    size_t pos = _FindBefore(string.c_str(), length(), string.length());
    if (pos != NotFound)
        _ShrinkAtBy(pos, string.length());

    return *this;
}


Buffer&
Buffer::RemoveAll(const Buffer& string)
{
    if (string.length() == 0 || length() == 0 || FindFirst(string) == NotFound)
        return *this;

    fork();

    return _DoReplace(string.c_str(), "", REPLACE_ALL, 0, KEEP_CASE);
}


Buffer&
Buffer::RemoveFirst(const char* string)
{
    size_t len = string ? strlen(string) : 0;
    if (len > 0) {
        size_t pos = _ShortFindAfter(string, len);
        if (pos != NotFound)
            _ShrinkAtBy(pos, len);
    }
    return *this;
}


Buffer&
Buffer::RemoveLast(const char* string)
{
    size_t len = string ? strlen(string) : 0;
    if (len > 0) {
        size_t pos = _FindBefore(string, length(), len);
        if (pos != NotFound)
            _ShrinkAtBy(pos, len);
    }
    return *this;
}


Buffer&
Buffer::RemoveAll(const char* string)
{
    if (!string || *string == 0 || length() == 0 || FindFirst(string) == NotFound)
        return *this;

    fork();

    return _DoReplace(string, "", REPLACE_ALL, 0, KEEP_CASE);
}


Buffer&
Buffer::RemoveSet(const char* setOfCharsToRemove)
{
    return ReplaceSet(setOfCharsToRemove, "");
}


Buffer&
Buffer::MoveInto(Buffer& into, size_t from, size_t len)
{
    if (len) {
        CopyInto(into, from, len);
        remove(from, len);
    }
    return into;
}


void
Buffer::MoveInto(char* into, size_t from, size_t len)
{
    if (into) {
        CopyInto(into, from, len);
        remove(from, len);
    }
}


bool
Buffer::operator<(const char* string) const
{
    return strcmp(c_str(), safestr(string)) < 0;
}


bool
Buffer::operator<=(const char* string) const
{
    return strcmp(c_str(), safestr(string)) <= 0;
}


bool
Buffer::operator==(const char* string) const
{
    return strcmp(c_str(), safestr(string)) == 0;
}


bool
Buffer::operator>=(const char* string) const
{
    return strcmp(c_str(), safestr(string)) >= 0;
}


bool
Buffer::operator>(const char* string) const
{
    return strcmp(c_str(), safestr(string)) > 0;
}


int
Buffer::Compare(const Buffer& string) const
{
    return memncmp(c_str(), string.c_str(), length(), string.length());
}


int
Buffer::Compare(const char* string) const
{
    string = safestr(string);
    return memncmp(c_str(), string, length(), strlen(string));
}


int
Buffer::Compare(const Buffer& string, size_t len) const
{
    return memncmp(c_str(), string.c_str(), min_clamp0(length(), len), min_clamp0(string.length(), len));
}


int
Buffer::Compare(const char* string, size_t len) const
{
    string = safestr(string);
    size_t stringLen = strlen(string);
    return memncmp(c_str(), string, min_clamp0(length(), len), min_clamp0(stringLen, len));
}


int
Buffer::ICompare(const Buffer& string) const
{
    return strcasecmp(c_str(), string.c_str());
}


int
Buffer::ICompare(const char* string) const
{
    return strcasecmp(c_str(), safestr(string));
}


int
Buffer::ICompare(const Buffer& string, size_t len) const
{
    return strncasecmp(c_str(), string.c_str(), len);
}


int
Buffer::ICompare(const char* string, size_t len) const
{
    return strncasecmp(c_str(), safestr(string), len);
}


size_t
Buffer::FindFirst(const Buffer& string) const
{
    return _ShortFindAfter(string.c_str(), string.length());
}


size_t
Buffer::FindFirst(const char* string) const
{
    ASSERT(string);
    return _ShortFindAfter(string, strlen(string));
}


size_t
Buffer::FindFirst(const Buffer& string, size_t fromOffset) const
{
    return _FindAfter(string.c_str(), min_clamp0(fromOffset, length()), string.length());
}


size_t
Buffer::FindFirst(const char* string, size_t fromOffset) const
{
    ASSERT(string);
    return _FindAfter(string, min_clamp0(fromOffset, length()), strlen(safestr(string)));
}


size_t
Buffer::FindFirst(char c) const
{
    const char *start = c_str();
    const char *end = c_str() + length();

    // Scans the string until we found the
    // character, or we reach the string's start
    while (start != end && *start != c) {
        start++;
    }

    if (start == end)
        return NotFound;

    return start - c_str();
}


size_t
Buffer::FindFirst(char c, size_t fromOffset) const
{
    const char *start = c_str() + min_clamp0(fromOffset, length());
    const char *end = c_str() + length();

    // Scans the string until we found the
    // character, or we reach the string's start
    while (start < end && *start != c) {
        start++;
    }

    if (start >= end)
        return NotFound;

    return start - c_str();
}


size_t
Buffer::FindLast(const Buffer& string) const
{
    return _FindBefore(string.c_str(), length(), string.length());
}


size_t
Buffer::FindLast(const char* string) const
{
    ASSERT(string);
    return _FindBefore(string, length(), strlen(safestr(string)));
}


size_t
Buffer::FindLast(const Buffer& string, size_t beforeOffset) const
{
    return _FindBefore(string.c_str(), min_clamp0(beforeOffset, length()), string.length());
}


size_t
Buffer::FindLast(const char* string, size_t beforeOffset) const
{
    ASSERT(string);
    return _FindBefore(string, min_clamp0(beforeOffset, length()), strlen(safestr(string)));
}


size_t
Buffer::FindLast(char c) const
{
    const char *start = c_str();
    const char *end = c_str() + length();

    // Scans the string backwards until we found
    // the character, or we reach the string's start
    while (end != start && *end != c) {
        end--;
    }

    if (end == start && *end != c)
        return NotFound;

    return end - c_str();
}


size_t
Buffer::FindLast(char c, size_t beforeOffset) const
{
    const char *start = c_str();
    const char *end = c_str() + min_clamp0(beforeOffset, length());

    // Scans the string backwards until we found
    // the character, or we reach the string's start
    while (end > start && *end != c) {
        end--;
    }

    if (end <= start && *end != c)
        return NotFound;

    return end - c_str();
}


size_t
Buffer::IFindFirst(const Buffer& string) const
{
    return _IFindAfter(string.c_str(), 0, string.length());
}


size_t
Buffer::IFindFirst(const char* string) const
{
    ASSERT(string);
    return _IFindAfter(string, 0, strlen(safestr(string)));
}


size_t
Buffer::IFindFirst(const Buffer& string, size_t fromOffset) const
{
    return _IFindAfter(string.c_str(), min_clamp0(fromOffset, length()), string.length());
}


size_t
Buffer::IFindFirst(const char* string, size_t fromOffset) const
{
    ASSERT(string);
    return _IFindAfter(string, min_clamp0(fromOffset,length()), strlen(safestr(string)));
}


size_t
Buffer::IFindLast(const Buffer& string) const
{
    return _IFindBefore(string.c_str(), length(), string.length());
}


size_t
Buffer::IFindLast(const char* string) const
{
    ASSERT(string);
    return _IFindBefore(string, length(), strlen(safestr(string)));
}


size_t
Buffer::IFindLast(const Buffer& string, size_t beforeOffset) const
{
    return _IFindBefore(string.c_str(), min_clamp0(beforeOffset, length()), string.length());
}


size_t
Buffer::IFindLast(const char* string, size_t beforeOffset) const
{
    ASSERT(string);
    return _IFindBefore(string, min_clamp0(beforeOffset, length()), strlen(safestr(string)));
}


Buffer&
Buffer::ReplaceFirst(char replaceThis, char withThis)
{
    size_t pos = FindFirst(replaceThis);
    if (pos != NotFound) {
        fork();
        m_data[pos] = withThis;
    }
    return *this;
}


Buffer&
Buffer::ReplaceLast(char replaceThis, char withThis)
{
    size_t pos = FindLast(replaceThis);
    if (pos != NotFound) {
        fork();
        m_data[pos] = withThis;
    }
    return *this;
}


Buffer&
Buffer::ReplaceAll(char replaceThis, char withThis, size_t fromOffset)
{
    fromOffset = min_clamp0(fromOffset, length());
    size_t pos = FindFirst(replaceThis, fromOffset);

    // detach and set first match
    if (pos != NotFound) {
        fork();
        m_data[pos] = withThis;
        while (1) {
            pos = FindFirst(replaceThis, pos);
            if (pos == NotFound)
                break;
            m_data[pos] = withThis;
        }
    }
    return *this;
}


Buffer&
Buffer::Replace(char replaceThis, char withThis, unsigned int maxReplaceCount,
    size_t fromOffset)
{
    fromOffset = min_clamp0(fromOffset, length());
    size_t pos = FindFirst(replaceThis, fromOffset);

    if (maxReplaceCount > 0 && pos != NotFound) {
        fork();
        maxReplaceCount--;
        m_data[pos] = withThis;
        for ( ;  maxReplaceCount > 0; maxReplaceCount--) {
            pos = FindFirst(replaceThis, pos);
            if (pos == NotFound)
                break;
            m_data[pos] = withThis;
        }
    }
    return *this;
}


Buffer&
Buffer::ReplaceFirst(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis || FindFirst(replaceThis) == NotFound)
        return *this;

    fork();

    return _DoReplace(replaceThis, withThis, 1, 0, KEEP_CASE);
}


Buffer&
Buffer::ReplaceLast(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis)
        return *this;

    size_t replaceThisLength = strlen(replaceThis);
    size_t pos = _FindBefore(replaceThis, length(), replaceThisLength);

    if (pos != NotFound) {
        size_t withThisLength =  strlen(withThis);
        ssize_t difference = withThisLength - replaceThisLength;

        if (difference > 0) {
            if (!_OpenAtBy(pos, difference))
                return *this;
        } else if (difference < 0) {
            if (!_ShrinkAtBy(pos, -difference))
                return *this;
        } else {
            fork();
        }
        memcpy(m_data + pos, withThis, withThisLength);
    }

    return *this;
}


Buffer&
Buffer::ReplaceAll(const char* replaceThis, const char* withThis,
    size_t fromOffset)
{
    if (!replaceThis || !withThis || FindFirst(replaceThis) == NotFound)
        return *this;

    fork();

    return _DoReplace(replaceThis, withThis, REPLACE_ALL,
        min_clamp0(fromOffset, length()), KEEP_CASE);
}


Buffer&
Buffer::Replace(const char* replaceThis, const char* withThis, unsigned int maxReplaceCount,
        size_t fromOffset)
{
    if (!replaceThis || !withThis || maxReplaceCount == 0 || FindFirst(replaceThis) == NotFound)
        return *this;

    fork();

    return _DoReplace(replaceThis, withThis, maxReplaceCount,
        min_clamp0(fromOffset, length()), KEEP_CASE);
}


Buffer&
Buffer::IReplaceFirst(char replaceThis, char withThis)
{
    char tmp[2] = { replaceThis, '\0' };

    size_t pos = _IFindAfter(tmp, 0, 1);
    if (pos != NotFound) {
        fork();
        m_data[pos] = withThis;
    }
    return *this;
}


Buffer&
Buffer::IReplaceLast(char replaceThis, char withThis)
{
    char tmp[2] = { replaceThis, '\0' };

    size_t pos = _IFindBefore(tmp, length(), 1);
    if (pos != NotFound) {
        fork();
        m_data[pos] = withThis;
    }
    return *this;
}


Buffer&
Buffer::IReplaceAll(char replaceThis, char withThis, size_t fromOffset)
{
    char tmp[2] = { replaceThis, '\0' };
    fromOffset = min_clamp0(fromOffset, length());
    size_t pos = _IFindAfter(tmp, fromOffset, 1);

    if (pos != NotFound) {
        fork();
        m_data[pos] = withThis;
        while (1) {
            pos = _IFindAfter(tmp, pos, 1);
            if (pos == NotFound)
                break;
            m_data[pos] = withThis;
        }
    }
    return *this;
}


Buffer&
Buffer::IReplace(char replaceThis, char withThis, unsigned int maxReplaceCount,
    size_t fromOffset)
{
    char tmp[2] = { replaceThis, '\0' };
    fromOffset = min_clamp0(fromOffset, length());
    size_t pos = _IFindAfter(tmp, fromOffset, 1);

    if (maxReplaceCount > 0 && pos != NotFound) {
        fork();
        m_data[pos] = withThis;
        maxReplaceCount--;
        for ( ; maxReplaceCount > 0; maxReplaceCount--) {
            pos = _IFindAfter(tmp, pos, 1);
            if (pos == NotFound)
                break;
            m_data[pos] = withThis;
        }
    }

    return *this;
}


Buffer&
Buffer::IReplaceFirst(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis || IFindFirst(replaceThis) == NotFound)
        return *this;

    fork();
    return _DoReplace(replaceThis, withThis, 1, 0, IGNORE_CASE);
}


Buffer&
Buffer::IReplaceLast(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis)
        return *this;

    size_t replaceThisLength = strlen(replaceThis);
    size_t pos = _IFindBefore(replaceThis, length(), replaceThisLength);

    if (pos != NotFound) {
        size_t withThisLength = strlen(withThis);
        ssize_t difference = withThisLength - replaceThisLength;

        if (difference > 0) {
            if (!_OpenAtBy(pos, difference))
                return *this;
        } else if (difference < 0) {
            if (!_ShrinkAtBy(pos, -difference))
                return *this;
        } else {
            fork();
        }
        memcpy(m_data + pos, withThis, withThisLength);
    }

    return *this;
}


Buffer&
Buffer::IReplaceAll(const char* replaceThis, const char* withThis,
    size_t fromOffset)
{
    if (!replaceThis || !withThis || IFindFirst(replaceThis) == NotFound)
        return *this;

    fork();

    return _DoReplace(replaceThis, withThis, REPLACE_ALL,
        min_clamp0(fromOffset, length()), IGNORE_CASE);
}


Buffer&
Buffer::IReplace(const char* replaceThis, const char* withThis,
    unsigned int maxReplaceCount, size_t fromOffset)
{
    if (!replaceThis || !withThis || maxReplaceCount == 0 || FindFirst(replaceThis) == NotFound)
        return *this;

    fork();

    return _DoReplace(replaceThis, withThis, maxReplaceCount,
        min_clamp0(fromOffset, length()), IGNORE_CASE);
}


Buffer&
Buffer::ReplaceSet(const char* setOfChars, char with)
{
    if (!setOfChars || strcspn(m_data, setOfChars) >= length())
        return *this;

    fork();

    size_t offset = 0;
    size_t len = length();
    for (size_t pos;;) {
        pos = strcspn(m_data + offset, setOfChars);

        offset += pos;
        if (offset >= len)
            break;

        m_data[offset] = with;
        offset++;
    }

    return *this;
}


Buffer&
Buffer::ReplaceSet(const char* setOfChars, const char* with)
{
    if (!setOfChars || !with
        || strcspn(m_data, setOfChars) >= length())
        return *this;

    // delegate simple case
    size_t withLen = strlen(with);
    if (withLen == 1)
        return ReplaceSet(setOfChars, *with);

    fork();

    size_t pos = 0;
    size_t searchLen = 1;
    size_t len = length();

    PosVect positions;
    for (size_t offset = 0; offset < len; offset += (pos + searchLen)) {
        pos = strcspn(m_data + offset, setOfChars);
        if (pos + offset >= len)
            break;
        positions.Add(offset + pos);
    }

    _ReplaceAtPositions(&positions, searchLen, with, withLen);
    return *this;
}


char*
Buffer::lockBuffer(size_t maxLength)
{
    _DetachWith(m_data, length(), maxLength);
    refCount() = -1;  // unsharable
    return m_data;
}


Buffer&
Buffer::unlockBuffer(size_t len)
{
    if (len > 0) {
        len = min_clamp0(len, length());
    } else {
        len = strlen(m_data);
    }

    // is unsharable, so was and remains only mine
    _Realloc(len);
    m_data[len] = '\0';
    refCount() = 1;  // mark shareable again

    return *this;
}


Buffer&
Buffer::ToLower()
{
    size_t len = length();
    if (len > 0) {
        fork();
        for (size_t count = 0; count < len; count++)
            m_data[count] = tolower(m_data[count]);
    }
    return *this;
}


Buffer&
Buffer::ToUpper()
{
    size_t len = length();
    if (len > 0) {
        fork();
        for (size_t count = 0; count < len; count++)
            m_data[count] = toupper(m_data[count]);
    }
    return *this;
}


Buffer&
Buffer::Capitalize()
{
    size_t len = length();

    if (len > 0) {
        fork();
        m_data[0] = toupper(m_data[0]);
        for (size_t count = 1; count < len; count++)
            m_data[count] = tolower(m_data[count]);
    }
    return *this;
}


Buffer&
Buffer::CharacterEscape(const char* original,
                         const char* setOfCharsToEscape, char escapeWith)
{
    if (setOfCharsToEscape)
        _DoCharacterEscape(original, setOfCharsToEscape, escapeWith);
    return *this;
}


Buffer&
Buffer::CharacterEscape(const char* setOfCharsToEscape, char escapeWith)
{
    if (setOfCharsToEscape && length() > 0)
        _DoCharacterEscape(m_data, setOfCharsToEscape, escapeWith);
    return *this;
}


Buffer&
Buffer::CharacterDeescape(const char* original, char escapeChar)
{
    return _DoCharacterDeescape(original, escapeChar);
}


Buffer&
Buffer::CharacterDeescape(char escapeChar)
{
    if (length() > 0)
        _DoCharacterDeescape(m_data, escapeChar);
    return *this;
}


Buffer&
Buffer::operator<<(const char* string)
{
    size_t len = strlen(string);
    _DoAppend(string, len);
    return *this;
}


Buffer&
Buffer::operator<<(const Buffer& string)
{
    _DoAppend(string.c_str(), string.length());
    return *this;
}


Buffer&
Buffer::operator<<(char c)
{
    _DoAppend(&c, 1);
    return *this;
}


Buffer&
Buffer::operator<<(unsigned int i)
{
    char num[32];
    size_t len = snprintf(num, sizeof(num), "%u", i);
    _DoAppend(num, len);
    return *this;
}


Buffer&
Buffer::operator<<(int32_t i)
{
    char num[32];
    size_t len = snprintf(num, sizeof(num), "%d", i);
    _DoAppend(num, len);
    return *this;
}


Buffer&
Buffer::operator<<(uint64_t i)
{
    char num[64];
    size_t len = snprintf(num, sizeof(num), "%"PRIu64, i);
    _DoAppend(num, len);
    return *this;
}


Buffer&
Buffer::operator<<(int64_t i)
{
    char num[64];
    size_t len = snprintf(num, sizeof(num), "%"PRId64, i);
    _DoAppend(num, len);
    return *this;
}


Buffer&
Buffer::operator<<(float f)
{
    char num[64];
    size_t len = snprintf(num, sizeof(num), "%.2f", f);

    _DoAppend(num, len);
    return *this;
}


void
Buffer::fork()
{
    if (refCount() > 1) {
        // Note:  Ordering is significant due to possible exceptions
        char* newData = _Clone(m_data, length());
        if (decRef() == 0)
            _FreePrivateData();
        m_data = newData;
    }
}


char*
Buffer::_Alloc(size_t len, bool adoptReferenceCount)
{
    char* newData = (char *)malloc(len + kPrivateDataOffset + 1);
    if (! newData)
        throw std::bad_alloc();

    newData += kPrivateDataOffset;
    newData[len] = '\0';

    int32_t n;
    if (adoptReferenceCount)
        n = refCount();
    else
        n = 1;
    referenceCount(newData) = n;
    setLength(len, newData);

    return newData;
}


char*
Buffer::_Realloc(size_t len)
{
    if (len == length())
        return m_data;

    int32_t oldReferenceCount = refCount();
    ASSERT(oldReferenceCount == -1 || oldReferenceCount == 1);  // Must be unsharable or unshared
    char *dataPtr = m_data - kPrivateDataOffset;

    dataPtr = (char*)realloc(dataPtr, len + kPrivateDataOffset + 1);
    if (! dataPtr)
        throw std::bad_alloc();
    dataPtr += kPrivateDataOffset;

    m_data = dataPtr;
    m_data[len] = '\0';

    _SetLength(len);
    refCount() = oldReferenceCount;
    return dataPtr;
}


void
Buffer::_Init(const char* src, size_t len)
{
    m_data = _Clone(src, len);
}


char*
Buffer::_Clone(const char* data, size_t len)
{
    char* newData = _Alloc(len, false);
    if (data)
        memcpy(newData, data, len);
    return newData;
}


char*
Buffer::_OpenAtBy(size_t offset, size_t len)
{
    size_t oldLength = length();
    fork();
    memmove(m_data + offset + len, m_data + offset, oldLength - offset);
    return _Realloc(oldLength + len);
}


char*
Buffer::_ShrinkAtBy(size_t offset, size_t len)
{
    size_t oldLength = length();
    fork();
    memmove(m_data + offset, m_data + offset + len, oldLength - offset - len);
    return _Realloc(oldLength - len);
}


void
Buffer::_DetachWith(const char* data, size_t dataLen, size_t totalLen)
{
    if (refCount() > 1) {
        // Note:  Ordering is significant due to possible exceptions
        char* newData = _Alloc(totalLen, false);
        if (data) {
            dataLen = min_clamp0(dataLen, totalLen);
            memcpy(newData, data, dataLen);
        }
        if (decRef() == 0)
            _FreePrivateData();
        m_data = newData;
    } else {
        bool self = (data == m_data);
        _Realloc(totalLen);
        if (!self && data) {
            dataLen = min_clamp0(dataLen, totalLen);
            memcpy(m_data, data, dataLen);
        }
    }
}


void
Buffer::_FreePrivateData()
{
    free(m_data - kPrivateDataOffset);
    m_data = 0;
}


void
Buffer::_DoAppend(const char* string, size_t len)
{
    if (len > 0) {
        size_t oldLength = length();
        _DetachWith(m_data, oldLength, oldLength + len);
        memcpy(m_data + oldLength, string, len);
    }
}


void
Buffer::_DoPrepend(const char* string, size_t len)
{
    if (len > 0) {
        size_t oldLength = length();
        _DetachWith(m_data, oldLength, oldLength + len);
        memmove(m_data + len, m_data, oldLength);
        memcpy(m_data, string, len);
    }
}


void
Buffer::_DoInsert(const char* string, size_t offset, size_t len)
{
    if (len > 0) {
        size_t oldLength = length();
        _DetachWith(m_data, oldLength, oldLength + len);
        memmove(m_data + offset + len, m_data + offset, oldLength - offset);
        memcpy(m_data + offset, string, len);
    }
}


size_t
Buffer::_ShortFindAfter(const char* string, size_t /*len*/) const
{
    // TODO  honor len
    const char *ptr = strstr(c_str(), string);

    if (ptr != NULL)
        return ptr - c_str();

    return NotFound;
}


size_t
Buffer::_FindAfter(const char* string, size_t offset, size_t /*strlen*/) const
{
    // TODO  honor len
    const char *ptr = strstr(c_str() + offset, string);

    if (ptr != NULL)
        return ptr - c_str();

    return NotFound;
}


size_t
Buffer::_IFindAfter(const char* string, size_t offset, size_t /*strlen*/) const
{
    // TODO  honor len
    const char *ptr = strcasestr(c_str() + offset, string);

    if (ptr != NULL)
        return ptr - c_str();

    return NotFound;
}


size_t
Buffer::_FindBefore(const char* string, size_t offset, size_t strlen) const
{
    const char *ptr = m_data + offset - strlen;

    while (ptr >= m_data) {
        if (!memcmp(ptr, string, strlen))
            return ptr - m_data;
        ptr--;
    }
    return NotFound;
}


size_t
Buffer::_IFindBefore(const char* string, size_t offset, size_t strlen) const
{
    char *ptr1 = m_data + offset - strlen;

    while (ptr1 >= m_data) {
        if (!strncasecmp(ptr1, string, strlen))
            return ptr1 - m_data;
        ptr1--;
    }
    return NotFound;
}


Buffer&
Buffer::_DoCharacterEscape(const char* string, const char* setOfCharsToEscape,
    char escapeChar)
{
    size_t len = strlen(safestr(string));
    _DetachWith(string, len, len);

    len = length();
    memcpy(m_data, string, len);

    PosVect positions;
    size_t pos = 0;
    for (size_t offset = 0; offset < len; offset += pos + 1) {
        if ((pos = strcspn(m_data + offset, setOfCharsToEscape))
                < len - offset) {
            positions.Add(offset + pos);
        }
    }

    size_t count = positions.CountItems();
    size_t newLength = len + count;
    if (!newLength) {
        _Realloc(0);
        return *this;
    }

    char* newData = _Alloc(newLength);
    char* oldString = m_data;
    char* newString = newData;
    size_t lastPos = 0;

    for (size_t i = 0; i < count; ++i) {
        pos = positions.ItemAt(i);
        len = pos - lastPos;
        if (len > 0) {
            memcpy(newString, oldString, len);
            oldString += len;
            newString += len;
        }
        *newString++ = escapeChar;
        *newString++ = *oldString++;
        lastPos = pos + 1;
    }

    len = length() + 1 - lastPos;
    if (len > 0)
        memcpy(newString, oldString, len);

    _FreePrivateData();
    m_data = newData;
    return *this;
}


Buffer&
Buffer::_DoCharacterDeescape(const char* string, char escapeChar)
{
    size_t len = strlen(safestr(string));
    _DetachWith(string, len, len);

    memcpy(m_data, string, length());
    const char escape[2] = { escapeChar, '\0' };
    return _DoReplace(escape, "", REPLACE_ALL, 0, KEEP_CASE);
}


Buffer&
Buffer::_DoReplace(const char* findThis, const char* replaceWith,
    unsigned int maxReplaceCount, size_t fromOffset, bool ignoreCase)
{
    if (findThis == NULL || maxReplaceCount == 0 || fromOffset >= length())
        return *this;

    typedef size_t (Buffer::*TFindMethod)(const char*, size_t, size_t) const;
    TFindMethod findMethod = ignoreCase ? &Buffer::_IFindAfter : &Buffer::_FindAfter;
    size_t findLen = strlen(findThis);

    if (!replaceWith)
        replaceWith = "";

    size_t replaceLen = strlen(replaceWith);
    size_t lastSrcPos = fromOffset;
    PosVect positions;
    for (size_t srcPos = 0; maxReplaceCount > 0
        && (srcPos = (this->*findMethod)(findThis, lastSrcPos, findLen)) != NotFound;
            maxReplaceCount--) {
        positions.Add(srcPos);
        lastSrcPos = srcPos + findLen;
    }
    _ReplaceAtPositions(&positions, findLen, replaceWith, replaceLen);
    return *this;
}


void
Buffer::_ReplaceAtPositions(const PosVect* positions, size_t searchLength,
    const char* with, size_t withLength)
{
    size_t len = length();
    size_t count = positions->CountItems();
    size_t newLength = len + count * (withLength - searchLength);
    if (!newLength) {
        _Realloc(0);
        return;
    }

    char *newData = _Alloc(newLength);
    char *oldString = m_data;
    char *newString = newData;
    size_t lastPos = 0;

    for (size_t i = 0; i < count; ++i) {
        size_t pos = positions->ItemAt(i);
        len = pos - lastPos;
        if (len > 0) {
            memcpy(newString, oldString, len);
            oldString += len;
            newString += len;
        }
        memcpy(newString, with, withLength);
        oldString += searchLength;
        newString += withLength;
        lastPos = pos + searchLength;
    }

    len = length() + 1 - lastPos;
    if (len > 0)
        memcpy(newString, oldString, len);

    _FreePrivateData();
    m_data = newData;
}


#if 0
/*
    Translates to (missing const):
    Buffer& Buffer::operator<<(Buffer& string)
*/
extern "C" Buffer&
__ls__7BufferR7Buffer(Buffer* self, Buffer& string)
{
    return self->operator<<(string);
}
#endif


int
Compare(const Buffer &string1, const Buffer &string2)
{
    return strcmp(string1.c_str(), string2.c_str());
}


int
ICompare(const Buffer &string1, const Buffer &string2)
{
    return strcasecmp(string1.c_str(), string2.c_str());
}


int
Compare(const Buffer *string1, const Buffer *string2)
{
    return strcmp(string1->c_str(), string2->c_str());
}


int
ICompare(const Buffer *string1, const Buffer *string2)
{
    return strcasecmp(string1->c_str(), string2->c_str());
}

}

