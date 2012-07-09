#include <ctype.h>

#include "ocher/layout/Layout.h"


Layout::Layout() :
    m_dataLen(0),
    nl(0),
    ws(0),
    pre(0),
    m_text(new clc::Buffer)
{
    m_data.lockBuffer(chunk);
}

Layout::~Layout()
{
}

clc::Buffer Layout::unlock()
{
    m_data.unlockBuffer(m_dataLen);
    return m_data;
}

char *Layout::checkAlloc(unsigned int n)
{
    if (m_dataLen + n > m_data.size()) {
        m_data.unlockBuffer(m_data.size());
        m_data.lockBuffer(m_dataLen + n + chunk);
    }
    char *p = m_data.c_str() + m_dataLen;
    m_dataLen += n;
    return p;
}

void Layout::push(unsigned int opType, unsigned int op, unsigned int arg)
{
    char *p = checkAlloc(2);
    uint16_t i = (opType<<12) | (op<<8) | arg;
    p[0] = (i>>8)&0xff;
    p[1] = (i   )&0xff;
}

void Layout::pushPtr(void *ptr)
{
    int n = sizeof(ptr);
    char *p = checkAlloc(n);
    *((char**)p) = (char*)ptr;
}

void Layout::pushTextAttr(TextAttr attr, uint8_t arg)
{
    push(OpPushTextAttr, attr, arg);
}

void Layout::popTextAttr(unsigned int n)
{
    push(OpCmd, CmdPopAttr, n);
}

void Layout::pushLineAttr(LineAttr attr, uint8_t arg)
{
    push(OpPushLineAttr, attr, arg);
}

void Layout::popLineAttr(unsigned int n)
{
    push(OpCmd, CmdPopAttr, n);
}

void Layout::outputChar(char c)
{
    nl = 0;
    if (isspace(c)) {
        if (! ws) {
            ws = 1;
            m_text->append(' ', 1);
        }
    } else {
        ws = 0;
        m_text->append(c, 1);
    }
}

void Layout::outputNl()
{
    if (! nl) {
        nl = 1;
        m_text->append('\n', 1);
    }
}

void Layout::outputBr()
{
    m_text->append('\n', 1);
}

void Layout::flushText()
{
    push(OpCmd, CmdOutputStr, 0);
    pushPtr(m_text);
}

