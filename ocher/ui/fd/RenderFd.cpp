#include <unistd.h>
#include <stdint.h>

#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

#include "ocher/ui/fd/RenderFd.h"
#include "ocher/layout/Layout.h"


// TODO:  page size

RendererFd::RendererFd(clc::Buffer layout, int fd) :
    Renderer(layout),
    m_fd(fd),
    m_page(1),
    ai(1)
{
    clc::Log::info("ocher", "RendererFd %d bytes", layout.size());
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

void RendererFd::render(unsigned int offset, unsigned int pageNum)
{
    unsigned int page = m_page;
    unsigned int x = 0;
    unsigned int y = 0;

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
                        break;
                    case Layout::AttrSizeAbs:
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
                        write(m_fd, str->data(), str->size());
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
