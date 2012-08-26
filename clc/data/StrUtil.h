#ifndef LIBCLC_STRING_UTILS_H
#define LIBCLC_STRING_UTILS_H

#include <ctype.h>
#include <iostream>
#include <stdarg.h>
#include <stdint.h>

#include "clc/data/Buffer.h"


namespace clc
{

/**
 */
std::ostream& operator<<(std::ostream& out, Buffer const& s);

/**
 *  Utility class for common string manipulation functions.
 *  @todo  document if/how these handle UTF-8
 */
namespace str
{
    /**
     *  Converts hex character (case insensitive) to a nibble.
     *  @param[in] hex  Hexadecimal character.  Results undefined if not a valid hex character.
     *  @return Integer value (4 bits) of the hexadecimal character.
     */
    inline unsigned char hexCharToByte(const char hex) {
        return (unsigned char)((hex > '9') ? (tolower(hex) + 10 - 'a') : (hex - '0'));
    }

    /**
     * @param byte  The byte to encode.
     * @param hex  2-byte buffer to receive the hex representation.
     */
    void hexByteToString(const char byte, char* hex);

    /**
     */
    void hexBytesToString(const unsigned char* buffer, size_t buflen, char* hex);

    /**
     */
    unsigned char hexStringToByte(const char hex[2]);

    /**
     */
    void hexStringToBytes(const char* hex, size_t bufferlen, unsigned char* buffer);

    /**
     */
    char hexByteToChar(const char byte);

    /**
     *  @param hex  Hex-encoded string, where each two characters map represent a byte.
     *  @return Binary string.
     */
    Buffer hexToBin(const Buffer& hex);

    /**
     *  @return Hex-encoded string.
     */
    Buffer binToHex(const Buffer& bin);

    /**
     */
    Buffer format(const char* fmt, ...);

    /**
     */
    Buffer format(const char* fmt, va_list ap);

    /**
     */
    void appendFormat(Buffer& s, const char* fmt, ...);

    /**
     */
    void appendFormat(Buffer& s, const char* fmt, va_list ap);

    /**
     *  Parses an unsigned integer.
     */
    uint64_t toUInt(Buffer const& s, bool *valid=0);

    /**
     *  Retrieves a line from a string.  Common usage is to loop, incrementing fromOffset each time
     *  by the length of the line retrieved, stopping when the length is zero.
     *  @param[in] s  The string containing lines.
     *  @param[in] fromOffset  The starting offset of the line.
     *  @return  The line, from fromOffset up to and including the line feed (if any).
     */
    Buffer getLine(Buffer const& s, size_t fromOffset);

    /**
     *  Parses a dotted numeric version with optional trailing junk, into an array of integers.
     *  Parsing stops when the array is full or junk is found.
     *  Example:  "2.6.27-11-generic" --> {2, 6, 27}
     *  Example:  "192.168.0.1" --> {192, 168, 0, 1}
     *  @param[in] src  The string with dotted decimal numbers.
     *  @param[out] dsts  The array to receive the parsed integers.
     *  @param[in] ndsts  The number of elements in dsts.
     *  @return  The number of integers populated in the destination array.
     */
    unsigned int parseDottedDecimal(const char* src, int* dsts, unsigned int ndsts);

    /**
     *  Trims the leading and trailing whitespace.  Binary safe.
     *  @param[in,out] s  The string to trim.
     */
    void trimWhitespace(Buffer& s);

    /**
     *  Trims the specified characters.  Binary safe.
     *  @param[in,out] s  The string to trim.
     *  @param c  Character to remove.
     */
    int trimLeading(Buffer& s, char c);

    /**
     *  Trims the specified characters.  Binary safe.
     *  @param[in,out] s  The string to trim.
     *  @param setOfChars  Null-terminated string, containing the character(s) to remove.
     */
    int trimLeadingOfSet(Buffer& s, const char* setOfChars);

    /**
     *  If haystack starts with needle, remove needle and return true, else false.
     *  @param haystack  The buffer to search.
     *  @param needle  What to search for.
     */
    bool trimLeading(Buffer& haystack, Buffer const& needle);

    /**
     *  Trims the trailing specified characters.  Binary safe.
     *  @param[in,out] s  The string to trim.
     *  @param c  Character to remove.
     */
    void trimTrailing(Buffer& s, char c);

    /**
     *  Trims the trailing specified characters.  Binary safe.
     *  @param[in,out] s  The string to trim.
     *  @param setOfChars  Null-terminated string, containing the character(s) to remove.
     */
    void trimTrailingOfSet(Buffer& s, const char* setOfChars);

    /**
     *  @param haystack  The buffer to search.
     *  @param needle  What to search for.
     */
    bool startsWith(Buffer& haystack, Buffer const& needle);

    void capitalizeEachWord(Buffer &s);
};

}

#endif

