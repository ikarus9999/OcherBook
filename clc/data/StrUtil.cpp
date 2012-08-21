#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "clc/support/Exception.h"
#include "clc/data/StrUtil.h"


namespace clc
{
namespace str
{

Buffer hexToBin(const Buffer& hex)
{
    Buffer bin;
    size_t bytes = hex.length();
    bytes >>= 1;
    const char* _hex = hex.c_str();
    char* p = bin.lockBuffer(bytes);
    for (size_t i = 0; i < bytes; ++i)
        p[i] = hexStringToByte(_hex+i*2);
    bin.unlockBuffer(bytes);
    return bin;
}

Buffer binToHex(const Buffer& bin)
{
    Buffer hex;
    size_t bytes = bin.length();
    char* p = hex.lockBuffer(bytes*2);
    for (size_t i = 0; i < bytes; ++i)
        hexByteToString(bin.ByteAt(i), p+i*2);
    hex.unlockBuffer(bytes*2);
    return hex;
}

char hexByteToChar(const char byte)
{
    return ((byte & 0x0f) > 9) ? ((byte & 0x0f) - 10 + 'a') : ((byte & 0x0f) + '0');
}

void hexByteToString(const char byte, char* hex)
{
    hex[0] = hexByteToChar(byte >> 4);
    hex[1] = hexByteToChar(byte);
}

void hexBytesToString(const unsigned char* buffer, size_t buflen, char* hex)
{
    while (buflen--) {
        hexByteToString(*buffer++, hex);
        hex += 2;
    }
    *hex = '\0';
}

unsigned char hexStringToByte(const char hex[2])
{
    char rval = hexCharToByte(hex[0]);
    rval <<= 4;
    rval += hexCharToByte(hex[1]);
    return rval;
}

void hexStringToBytes(const char* hex, size_t bufferlen, unsigned char* buffer) {
    while (bufferlen--) {
        *buffer++ = hexStringToByte(hex);
        hex += 2;
    }
}

std::ostream& operator<<(std::ostream& out, Buffer const& s)
{
    for (size_t i = 0; i < s.length(); ++i)
        out << s[i];
    return out;
}

Buffer format(const char* fmt, ...)
{
    char* buf;
    va_list argList;
    va_start(argList, fmt);
    int len = vasprintf(&buf, fmt, argList);
    va_end(argList);
    Buffer s;
    if (len >= 0) {
        s.setTo(buf, len);
        free(buf);
    }
    return s;
}

Buffer format(const char* fmt, va_list argList)
{
    char* buf;
    int len = vasprintf(&buf, fmt, argList);
    Buffer s;
    if (len >= 0) {
        s.setTo(buf, len);
        free(buf);
    }
    return s;
}

void appendFormat(Buffer& s, const char* fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);
    appendFormat(s, fmt, argList);
    va_end(argList);
}

void appendFormat(Buffer& s, const char* fmt, va_list argList)
{
    char* buf;
    int len = vasprintf(&buf, fmt, argList);
    if (len >= 0) {
        s.append(buf, len);
        free(buf);
    }
}

uint64_t toUInt(Buffer const& s)
{
    uint64_t v = 0;
    size_t i = 0;
    for ( ; i < s.length(); ++i) {
        char c = s[i];
        if (c >= '0' && c <= '9')
            v = (v*10) + (c - '0');
        else
            break;
    }
    if (i == 0)
        throw BufferUnderflowException();
    return v;
}

Buffer getLine(Buffer const& s, size_t fromOffset)
{
    size_t offset = s.FindFirst('\n', fromOffset);
    const char* start = s.c_str() + fromOffset;
    if (offset == Buffer::NotFound)
        return Buffer(start);
    return Buffer(start, offset - fromOffset + 1);
}

unsigned int parseDottedDecimal(const char* src, int* dsts, unsigned int ndsts)
{
    unsigned int i = 0;
    bool reset = true;
    while (i < ndsts) {
        if (*src >= '0' && *src <= '9') {
            if (reset) {
                dsts[i] = 0;
                reset = false;
            }
            dsts[i] = (dsts[i] * 10) + (*src - '0');
        }
        else if (*src == '.') {
            reset = true;
            ++i;
        } else {
            if (!reset)
                ++i;
            break;
        }
        ++src;
    }
    return i;
}

int trimLeading(Buffer& s, char c)
{
    char setOfChars[] = {c, 0};
    return trimLeadingOfSet(s, setOfChars);
}

int trimLeadingOfSet(Buffer& s, const char* setOfChars)
{
    const char* data = s.c_str();
    size_t len = s.length();
    size_t toTrim = 0;
    while (toTrim < len && strchr(setOfChars, data[toTrim]))
        toTrim++;
    s.remove(0, toTrim);
    return toTrim;
}

bool trimLeading(Buffer& haystack, Buffer const& needle)
{
    if (startsWith(haystack, needle)) {
        haystack.remove(0, needle.length());
        return true;
    }
    return false;
}

void trimTrailing(Buffer& s, char c)
{
    char setOfChars[] = {c, 0};
    trimTrailingOfSet(s, setOfChars);
}

void trimTrailingOfSet(Buffer& s, const char* setOfChars)
{
    const char* data = s.c_str();
    size_t len = s.length();
    size_t newLen = len;
    while (newLen > 0 && strchr(setOfChars, data[newLen-1]))
        newLen--;
    s.truncate(newLen);
}

void trimWhitespace(Buffer& s)
{
    trimTrailingOfSet(s, " \t\v\r\n");
    trimLeadingOfSet(s, " \t\v\r\n");
}

bool startsWith(Buffer& haystack, Buffer const& needle)
{
    return haystack.length() >= needle.length() &&
        memcmp(haystack.c_str(), needle.c_str(), needle.length()) == 0;
}

void capitalizeEachWord(Buffer& s)
{
#if 0
    size_t len = s.length();

    // TODO:  Lock to avoid repeated refcount checks?
    //		should there be a simplified lock (lock existing length)?
    if (len > 0) {
        s.fork();
        size_t count = 0;
        do {
            // Find the first alphabetical character...
            for (; count < len; count++) {
                if (isalpha(m_data[count])) {
                    // ...found! Convert it to uppercase.
                    m_data[count] = toupper(s.c_str()+count);
                    count++;
                    break;
                }
            }

            // Now find the first non-alphabetical character,
            // and meanwhile, turn to lowercase all the alphabetical ones
            for (; count < len; count++) {
                if (isalpha(s.c_str()+count))
                    m_data[count] = tolower(m_data[count]);
                else
                    break;
            }
        } while (count < len);
    }
#endif
}

}
}

