#include "clc/support/ByteOrder.h"
#include "clc/support/Flattenable.h"
#include "clc/data/Buffer.h"
#include "clc/support/Exception.h"


namespace clc
{

Flattenable::~Flattenable()
{
}

size_t Flattenable::flattenedSize() const
{
    Flattener f;
    return flatten(f);
}

void Flattenable::flatten(Buffer& s) const
{
    size_t n = flattenedSize();
    char* p = s.lockBuffer(n);
    Flattener f(p, n);
    n = flatten(f);
    s.unlockBuffer(n);
}


Flattener::Flattener(char* buf, size_t len) :
    m_buf(buf),
    m_len(len),
    m_filled(0)
{
}

void Flattener::setTo(char* buf, size_t len)
{
    m_buf = buf;
    m_len = len;
    m_filled = 0;
}


void Flattener::u8(uint8_t i)
{
    if (m_filled < m_len)
        *(m_buf + m_filled) = i;
    ++m_filled;
}

void Flattener::u16(uint16_t i)
{
    if (m_filled < m_len)
        *(m_buf + m_filled) = (uint8_t)(i>>8);;
    ++m_filled;
    if (m_filled < m_len)
        *(m_buf + m_filled) = (uint8_t)i;
    ++m_filled;
}

void Flattener::u32(uint32_t i)
{
    i = ByteOrder::hostToBE32(i);
    if (m_filled < m_len) {
        size_t stored = m_len - m_filled;
        if (stored > 4)
            stored = 4;
        memcpy(m_buf + m_filled, &i, stored);
    }
    m_filled += 4;
}

void Flattener::pBuf(const Buffer& b)
{
    size_t len = b.length();
    u16(len);
    if (m_filled < m_len) {
        len = (len < (m_len-m_filled)) ? len : (m_len-m_filled);
        memcpy(m_buf + m_filled, b.data(), len);
    }
    m_filled += len;
}

void Flattener::cBuf(const Buffer& b)
{
    cBuf(b.c_str());
}

void Flattener::cBuf(const char* s)
{
    size_t len = strlen(s) + 1;
    u16(len);
    if (m_filled < m_len) {
        len = (len < (m_len-m_filled)) ? len : (m_len-m_filled);
        memcpy(m_buf + m_filled, s, len);
    }
    m_filled += len;
}

void Flattener::vecCBuf(const std::vector<Buffer>& v)
{
    u16(v.size());
    for (std::vector<Buffer>::const_iterator it = v.begin(); it != v.end(); ++it) {
        cBuf(*it);
    }
}

void Unflattener::cBuf(Buffer& b)
{
    uint16_t len = u16();
    Buffer t;  // Below could throw, so lock a local
    char *c = t.lockBuffer(len);
    while (len > 0) {  // TODO: rewrite with memcpy
        *c = u8();
        ++c;
        --len;
    }
    t.unlockBuffer(len);
    b = t;
}

void Unflattener::vecCBuf(std::vector<Buffer>& v)
{
    v.clear();
    uint16_t i = u16();
    while (i > 0) {
        Buffer b;
        cBuf(b);
        v.push_back(b);
        --i;
    }
}

uint8_t Unflattener::u8()
{
    if (m_len < 1)
        throw BufferUnderflowException();
    uint8_t i = *m_buf;
    m_buf ++;
    m_len --;
    return i;
}

uint16_t Unflattener::u16()
{
    if (m_len < 2)
        throw BufferUnderflowException();
    uint16_t i = ByteOrder::unflatten16(m_buf);
    m_buf += 2;
    m_len -= 2;
    return i;
}

uint32_t Unflattener::u32()
{
    if (m_len < 4)
        throw BufferUnderflowException();
    uint32_t i = ByteOrder::unflatten32(m_buf);
    m_buf += 4;
    m_len -= 4;
    return i;
}

void Unflattener::pBuf(Buffer& b)
{
    unsigned int len = u16();
    if (m_len < len)
        throw BufferUnderflowException();
    char* p = b.lockBuffer(len);
    memcpy(p, m_buf, len);
    m_buf += len;
    m_len -= len;
    b.unlockBuffer(len);
}

}

