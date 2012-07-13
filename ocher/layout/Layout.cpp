#include <ctype.h>

#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

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
    // TODO:  walk m_text and release embedded strings?
    delete m_text;
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
    *(uint16_t*)p = i;
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
    // TODO:  lock large chunks of m_text for efficiency; limit size and flush
    if (isspace(c)) {
        if (! ws) {
            ws = 1;
            m_text->append(' ', 1);
        }
    } else {
        nl = 0;
        ws = 0;
        m_text->append(c, 1);
    }
}

void Layout::outputNl()
{
    if (! nl) {
        m_text->append('\n', 1);
        nl = 1;
    }
}

void Layout::outputBr()
{
    m_text->append('\n', 1);
    nl = 1;
}

void Layout::flushText()
{
    push(OpCmd, CmdOutputStr, 0);
    pushPtr(m_text);
    // m_text pointer is now owned by the layout bytecode.
    m_text = new clc::Buffer;
}

