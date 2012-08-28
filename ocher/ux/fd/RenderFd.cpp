#define __USE_GNU  // for memrchr
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

#include "ocher/settings/Options.h"
#include "ocher/ux/fd/RenderFd.h"
#include "ocher/fmt/Layout.h"


// TODO:  margins (not the same as margins for fb?)

RendererFd::RendererFd() :
    m_fd(-1),
    m_x(0),
    m_y(0),
    m_page(1),
    ai(1)
{
    struct winsize win;
    if (ioctl(0, TIOCGWINSZ, &win) != 0) {
        m_width = 80;
        m_height = 0;
    } else {
        m_width = win.ws_col;
        m_height = win.ws_row;
    }
}

bool RendererFd::init()
{
    m_fd = opt.inFd;
    return true;
}

void RendererFd::setWidth(int width)
{
    m_width = width;
}

void RendererFd::clearScreen()
{
    write(m_fd, "\033E", 2);
}

void RendererFd::enableUl()
{
    write(m_fd, "\x1b[4m", 5);
}

void RendererFd::disableUl()
{
    write(m_fd, "\x1b[24m", 6);
}

void RendererFd::enableEm()
{
    write(m_fd, "\x1b[1m", 5);
}

void RendererFd::disableEm()
{
    write(m_fd, "\x1b[22m", 6);
}

void RendererFd::pushAttrs()
{
    a[ai+1] = a[ai];
    ai++;
}

void RendererFd::applyAttrs(int i)
{
    if (a[ai].ul && !a[ai-i].ul) {
        enableUl();
    } else if (!a[ai].ul && a[ai-i].ul) {
        disableUl();
    }

    if (a[ai].em && !a[ai-i].em) {
        enableEm();
    } else if (!a[ai].em && a[ai-i].em) {
        disableEm();
    }
}

void RendererFd::popAttrs()
{
    ai--;
    applyAttrs(-1);
}

int RendererFd::outputWrapped(clc::Buffer *b, unsigned int strOffset, bool doBlit)
{
    int len = b->size();
    const unsigned char *start = (const unsigned char*)b->data();
    const unsigned char *p = start;

    ASSERT(strOffset <= len);
    len -= strOffset;
    p += strOffset;

    do {
        int w = m_width - m_x;

        // If at start of line, eat spaces
        if (m_x == 0) {
            while (*p != '\n' && isspace(*p)) {
                ++p;
                --len;
            }
        }

        // How many chars should go out on this line?
        const unsigned char *nl = 0;
        int n = w;
        if (w >= len) {
            n = len;
            nl = (const unsigned char *)memchr(p, '\n', n);
        } else {
            nl = (const unsigned char *)memchr(p, '\n', n);
            if (!nl) {
                // don't break words
                if (!isspace(*(p+n-1)) && !isspace(*(p+n))) {
                    unsigned char *space = (unsigned char*)memrchr(p, ' ', n);
                    if (space) {
                        nl = space;
                    }
                }
            }
        }
        if (nl)
            n = nl - p;

        if (doBlit)
            write(m_fd, p, n);
        p += n;
        len -= n;
        m_x += n;
        if (nl || m_x >= m_width-1) {
            if (doBlit)
                write(m_fd, "\n", 1);
            m_x = 0;
            m_y++;
            if (nl) {
                p++;
                len--;
            }
            if (m_height > 0 && m_y >= m_height) {
                return p - start;
            }
        }
    } while (len > 0);
    return -1;
}


int RendererFd::render(unsigned int pageNum, bool doBlit)
{
    m_x = 0;
    m_y = 0;
    if (m_height) {
        clearScreen();
    }

    unsigned int layoutOffset;
    unsigned int strOffset;
    if (!pageNum) {
        layoutOffset = 0;
        strOffset = 0;
    } else if (! m_pagination.get(pageNum-1, &layoutOffset, &strOffset)) {
        // Previous page not already paginated?
        // Perhaps at end of book?
        return -1;
    }

    const unsigned int N = m_layout.size();
    const char *raw = m_layout.data();
    ASSERT(layoutOffset < N);
    for (unsigned int i = layoutOffset; i < N; ) {
        ASSERT(i+2 <= N);
        uint16_t code = *(uint16_t*)(raw+i);
        i += 2;

        unsigned int opType = (code>>12)&0xf;
        unsigned int op = (code>>8)&0xf;
        unsigned int arg = code & 0xff;
        switch (opType) {
            case Layout::OpPushTextAttr:
                clc::Log::debug("ocher.renderer.fd", "OpPushTextAttr");
                switch (op) {
                    case Layout::AttrBold:
                        pushAttrs();
                        a[ai].b = 1;
                        if (doBlit)
                            applyAttrs(1);
                        break;
                    case Layout::AttrUnderline:
                        pushAttrs();
                        a[ai].ul = 1;
                        if (doBlit)
                            applyAttrs(1);
                        break;
                    case Layout::AttrItalics:
                        pushAttrs();
                        a[ai].em = 1;
                        if (doBlit)
                            applyAttrs(1);
                        break;
                    case Layout::AttrSizeRel:
                        pushAttrs();
                        break;
                    case Layout::AttrSizeAbs:
                        pushAttrs();
                        break;
                    default:
                        clc::Log::error("ocher.renderer.fd", "unknown OpPushTextAttr");
                        ASSERT(0);
                        break;
                }
                break;
            case Layout::OpPushLineAttr:
                clc::Log::debug("ocher.renderer.fd", "OpPushLineAttr");
                switch (op) {
                    case Layout::LineJustifyLeft:
                        break;
                    case Layout::LineJustifyCenter:
                        break;
                    case Layout::LineJustifyFull:
                        break;
                    case Layout::LineJustifyRight:
                        break;
                    default:
                        clc::Log::error("ocher.renderer.fd", "unknown OpPushLineAttr");
                        ASSERT(0);
                        break;
                }
                break;
            case Layout::OpCmd:
                switch (op) {
                    case Layout::CmdPopAttr:
                        clc::Log::debug("ocher.renderer.fd", "OpCmd CmdPopAttr");
                        if (arg == 0)
                            arg = 1;
                        while (arg--)
                            popAttrs();
                        break;
                    case Layout::CmdOutputStr: {
                        clc::Log::debug("ocher.renderer.fd", "OpCmd CmdOutputStr");
                        ASSERT(i + sizeof(clc::Buffer*) <= N);
                        clc::Buffer *str = *(clc::Buffer**)(raw+i);
                        ASSERT(strOffset <= str->size());
                        int breakOffset = outputWrapped(str, strOffset, doBlit);
                        strOffset = 0;
                        if (breakOffset >= 0) {
                            if (!doBlit) {
                                m_pagination.set(pageNum, i-2, breakOffset);
                            }
                            return 0;
                        }
                        i += sizeof(clc::Buffer*);
                        break;
                    }
                    case Layout::CmdForcePage:
                        clc::Log::debug("ocher.renderer.fd", "OpCmd CmdForcePage");
                        break;
                    default:
                        clc::Log::error("ocher.renderer.fd", "unknown OpCmd");
                        ASSERT(0);
                        break;
                }
                break;
            case Layout::OpSpacing:
                break;
            case Layout::OpImage:
                break;
            default:
                clc::Log::error("ocher.renderer.fd", "unknown op type");
                ASSERT(0);
                break;

        };
    }
    return 1;
}
