#define __USE_GNU  // for memrchr
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

#include "ocher/ui/fd/RenderFd.h"
#include "ocher/layout/Layout.h"


// TODO:  page size

RendererFd::RendererFd(int fd) :
    m_fd(fd),
    m_width(72),
    m_x(0),
    m_y(0),
    m_page(1),
    ai(1)
{
}

void RendererFd::setWidth(int width)
{
    m_width = width;
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

void RendererFd::outputWrapped(clc::Buffer *b)
{
    if (m_width <= 0) {
        write(m_fd, b->data(), b->size());
        return;
    }
    
    int len = b->size();
    const char *p = b->data();

    do {
        int w = m_width - m_x;

        // If at start of line, eat spaces
        if (m_x == 0) {
            while (*p != '\n' && isspace(*p)) {
                ++p;
                --len;
            }
            //write(m_fd, "----------------------------------------------------------------------------", m_width);
            //write(m_fd, "\n", 1);
        }

        // How many chars should go out on this line?
        const char *nl = 0;
        int n = w;
        if (w >= len) {
            n = len;
            nl = (const char *)memchr(p, '\n', n);
        } else {
            nl = (const char *)memchr(p, '\n', n);
            if (!nl) {
                // don't break words
                if (!isspace(*(p+n-1)) && !isspace(*(p+n))) {
                    char *space = (char*)memrchr(p, ' ', n);
                    if (space) {
                        nl = space;
                    }
                }
            }
        }
        if (nl)
            n = nl - p;

        write(m_fd, p, n);
        p += n;
        len -= n;
        m_x += n;
        if (nl || m_x >= m_width-1) {
            write(m_fd, "\n", 1);
            m_x = 0;
            m_y++;
            if (nl) {
                p++;
                len--;
            }
        }
    } while (len > 0);
}


void RendererFd::render(unsigned int pageNum)
{
    (void)pageNum;  // TODO:  render the requested page, not everything

    const unsigned int N = m_layout.size();
    const char *raw = m_layout.data();
    for (unsigned int i = 0; i < N; ) {
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
                        applyAttrs(1);
                        break;
                    case Layout::AttrUnderline:
                        pushAttrs();
                        a[ai].ul = 1;
                        applyAttrs(1);
                        break;
                    case Layout::AttrItalics:
                        pushAttrs();
                        a[ai].em = 1;
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
                        i += sizeof(clc::Buffer*);
                        outputWrapped(str);
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
}
