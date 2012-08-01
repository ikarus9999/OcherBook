#include <ctype.h>

#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

#include "ocher/fmt/Layout.h"
#include "ocher/output/FreeType.h"
#include "ocher/output/FrameBuffer.h"
#include "ocher/settings/Options.h"
#include "ocher/ux/fb/RenderFb.h"


RenderFb::RenderFb(FreeType *ft, FrameBuffer *fb) :
    m_ft(ft),
    m_fb(fb),
    m_col(0),
    m_penX(0),
    m_penY(20),  //TODO
    m_page(1),
    ai(1)
{
}

bool RenderFb::init()
{
    m_ft->setSize(12);
    return true;
}

void RenderFb::pushAttrs()
{
    a[ai+1] = a[ai];
    ai++;
}

void RenderFb::popAttrs()
{
    ai--;
}

void RenderFb::outputWrapped(clc::Buffer *b)
{
    int len = b->size();
    const char *p = b->data();

    int width = m_fb->width();
    // TODO: a complete hack of an approximation of # of char on a line
    int w = width / 8;
    do {
        // If at start of line, eat spaces
        if (m_col == 0) {
            while (*p != '\n' && isspace(*p)) {
                ++p;
                --len;
            }
        }

#if 1
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
#endif

        for (int i = 0; i < n; ++i) {
            // TODO: UTF32
            m_ft->renderGlyph(p[i], &m_penX, &m_penY);
            m_col ++;
        }

        p += n;
        len -= n;
        m_penX += n;
        if (nl || m_penX >= width-1) {
            //write(m_fd, "\n", 1);
            m_penX = 0;
            m_penY += 20;  // TODO
            if (nl) {
                p++;
                len--;
            }
        }
    } while (len > 0);
}

void RenderFb::render(unsigned int pageNum)
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
                clc::Log::debug("ocher.render.fb", "OpPushTextAttr");
                switch (op) {
                    case Layout::AttrBold:
                        pushAttrs();
                        a[ai].b = 1;
                        break;
                    case Layout::AttrUnderline:
                        pushAttrs();
                        a[ai].ul = 1;
                        break;
                    case Layout::AttrItalics:
                        pushAttrs();
                        a[ai].em = 1;
                        break;
                    case Layout::AttrSizeRel:
                        pushAttrs();
                        break;
                    case Layout::AttrSizeAbs:
                        pushAttrs();
                        break;
                    default:
                        clc::Log::error("ocher.render.fb", "unknown OpPushTextAttr");
                        ASSERT(0);
                        break;
                }
                break;
            case Layout::OpPushLineAttr:
                clc::Log::debug("ocher.render.fb", "OpPushLineAttr");
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
                        clc::Log::error("ocher.render.fb", "unknown OpPushLineAttr");
                        ASSERT(0);
                        break;
                }
                break;
            case Layout::OpCmd:
                switch (op) {
                    case Layout::CmdPopAttr:
                        clc::Log::debug("ocher.render.fb", "OpCmd CmdPopAttr");
                        if (arg == 0)
                            arg = 1;
                        while (arg--)
                            popAttrs();
                        break;
                    case Layout::CmdOutputStr: {
                        clc::Log::debug("ocher.render.fb", "OpCmd CmdOutputStr");
                        ASSERT(i + sizeof(clc::Buffer*) <= N);
                        clc::Buffer *str = *(clc::Buffer**)(raw+i);
                        i += sizeof(clc::Buffer*);
                        outputWrapped(str);
                        m_fb->update(0, 0, m_fb->width(), m_fb->height(), false); // DDD
                        break;
                    }
                    case Layout::CmdForcePage:
                        clc::Log::debug("ocher.render.fb", "OpCmd CmdForcePage");
                        break;
                    default:
                        clc::Log::error("ocher.render.fb", "unknown OpCmd");
                        ASSERT(0);
                        break;
                }
                break;
            case Layout::OpSpacing:
                break;
            case Layout::OpImage:
                break;
            default:
                clc::Log::error("ocher.render.fb", "unknown op type");
                ASSERT(0);
                break;

        };
    }
}
