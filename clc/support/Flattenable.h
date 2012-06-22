#ifndef LIBCLC_FLATTENABLE_H
#define LIBCLC_FLATTENABLE_H

#include <stdint.h>
#include <unistd.h>
#include <vector>

namespace clc
{

typedef uint32_t type_code;

class Buffer;

/**
 *  Helper class, used in a Flattenable during flattening.
 */
class Flattener
{
public:
    /**
     *  Creates a Flattener, which by default has no buffer to fill.  Call setTo, or even without
     *  a buffer it can be used to measure the space required.
     */
    Flattener() : m_buf(0), m_len(0), m_filled(0) {}

    /**
     *  Creates a Flattener, which will fill buff, up to len bytes.
     *  @param buf  The buffer.
     *  @param len  The length of the buffer.
     */
    Flattener(char* buf, size_t len);

    /**
     *  Sets the buffer that the Flattener will fill.
     *  @param buf  The buffer.
     *  @param len  The length of the buffer.
     */
    void setTo(char* buf, size_t len);

    /**
     *  Packs a byte.
     */
    void u8(uint8_t i);

    /**
     *  Packs a uint16_t in network byte order.
     */
    void u16(uint16_t i);

    /**
     *  Packs a uint32_t in network byte order.
     */
    void u32(uint32_t i);

    /**
     *  Packs a Buffer, by prefixing a length.  The buffer must not be longer than 64k.
     */
    void pBuf(const Buffer& b);

    /**
     *  Packs a Buffer as a C string, by storing up to and including the terminating NULL.
     */
    void cBuf(const Buffer& b);

    /**
     *  Packs a C string, by storing up to and including the terminating NULL.
     */
    void cBuf(const char* b);

    /**
     *  Packs a vector of C Buffers.
     */
    void vecCBuf(const std::vector<Buffer>& v);

    /**
     * @return Number of bytes this Flattener currently wants to fill.
     */
    size_t wantedLen() const { return m_filled; }

protected:
    char* m_buf;
    size_t m_len;     ///< size of m_buf
    size_t m_filled;  ///< amount of buf filled; always <= m_len.
};

/**
 *  Helper class, used in a Flattenable during unflattening.
 */
class Unflattener
{
public:
    /**
     *  Creates a Unflattener.
     *  @param buf  The buffer.
     *  @param len  The length of the buffer.
     */
    Unflattener(const char*& buf, size_t& len) : m_buf(buf), m_len(len) {}

    uint8_t u8();

    uint16_t u16();

    uint32_t u32();

    void cBuf(const Buffer& b);

    /**
     *  Unpacks a Buffer, by first reading a prefixed length.  The buffer must not be longer than 64k.
     */
    void pBuf(Buffer& b);

    void cBuf(Buffer& b);

    void vecCBuf(std::vector<Buffer>& v);

    size_t length() const { return m_len; }

private:
    const char*& m_buf;
    size_t& m_len;
};

/**
 *  Mix-in for objects that can be flattened (serialized) and unflattened.
 */
class Flattenable
{
public:
    virtual ~Flattenable();

    /**
     *  @return true iff this object is always the same size.
     */
    virtual bool isFixedSize() const = 0;

    /**
     *  @return The size of the object if it were flattened right now.
     */
    virtual size_t flattenedSize() const;

    /**
     * Flattens the object (or as much as will fit) into the Flattener.
     * @return number of bytes that would have been produced if the Flattener were of unlimited
     * size.  If returned value is greater than Flattener's size, then Flattener was too small.
     */
    virtual size_t flatten(Flattener& f) const = 0;

    /**
     *  Flattens the object into the Buffer.  Buffer's contents, if any, are replaced.
     *  The Buffer will grow to accomodate the flattened object.
     *  Default implementation calls flatten() twice (once for flattenedSize and then the real
     *  flatten() call) so classes in performance critical code may want to override.
     */
    virtual void flatten(Buffer& s) const;

    /**
     * Unflattens state into the current object.
     * @param buffer  Buffer to unflatten from.  Updated to reflect the consumed bytes.
     * @param size  Size of buffer to unflatten from.  Updated to reflect the consumed bytes.
     * @throw clc::BufferUnderflow if not enough bytes are available.
     */
    virtual void unflatten(const char*& buffer, size_t& size) = 0;
};


}

#endif
