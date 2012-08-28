#include <ctype.h>

#include "clc/support/Debug.h"
#include "clc/support/Logger.h"

#include "ocher/fmt/Layout.h"
#include "ocher/output/FreeType.h"
#include "ocher/output/FrameBuffer.h"
#include "ocher/settings/Settings.h"
#include "ocher/ux/fb/RenderFb.h"


RenderFb::RenderFb(FreeType *ft, FrameBuffer *fb) :
    m_ft(ft),
    m_fb(fb),
    m_col(0),
    m_penX(settings.marginLeft),
    m_penY(settings.marginTop),
    m_lineHeight(10),
    m_page(1),
    ai(1)
{
}

bool RenderFb::init()
{
    if (! m_ft->init())
        return false;
    m_ft->setSize(settings.fontPoints);
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

int RenderFb::outputWrapped(clc::Buffer *b, unsigned int strOffset, bool doBlit)
{
    int dx, dy;
    int len = b->size();
    const unsigned char *start = (const unsigned char*)b->data();
    const unsigned char *p = start;

    ASSERT(strOffset <= len);
    len -= strOffset;
    p += strOffset;

    // TODO:  first time on a page, must penY+= bearingY of current face
    // TODO:  proper word wrap
    // TODO:  paginate

    bool wordWrapped = false;
    int width = m_fb->width();
    do {
        // If at start of line, eat spaces
        if (m_col == 0) {
            while (*p != '\n' && isspace(*p)) {
                ++p;
                --len;
            }
        }

        if (*p != '\n') {
            // Where is the next word break?
            const unsigned char *end = p;
            while (*end && !isspace(*end)) {
                ++end;
            }
            if (*end)
                ++end;

            // Output until EOL (\n or wrap)
            for ( ; p < end && *p != '\n'; ++p, --len) {
                uint32_t c = *p;
                if (c > 0x7f) {
                    // Convert UTF8 to UTF32, as required by FreeType
                    if ((c & 0xe0) == 0xc0) {
                        c = ((c & 0x1f) << 6) | (p[1] & 0x3f);
                        p += 1;
                        len -= 1;
                    } else if ((c & 0xf0) == 0xe0) {
                        c = ((c & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
                        p += 2;
                        len -= 2;
                    } else if ((c & 0xf8) == 0xf0) {
                        c = ((c & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
                        p += 3;
                        len -= 3;
                    } else if ((c & 0xfc) == 0xf8) {
                        c = ((c & 0x03) << 24) | ((p[1] & 0x3f) << 18) | ((p[2] & 0x3f) << 12) | ((p[3] & 0x3f) << 6) | (p[4] & 0x3f);
                        p += 4;
                        len -= 4;
                    } else if ((c & 0xfe) == 0xfc) {
                        c = ((c & 0x01) << 30) | ((p[1] & 0x3f) << 24) | ((p[2] & 0x3f) << 18) | ((p[3] & 0x3f) << 12) | ((p[4] & 0x3f) << 6) | (p[5] & 0x3f);
                        p += 5;
                        len -= 5;
                    } else {
                        // out of sync?
                        continue;
                    }
                }

                // TODO:  probably have to move the freetype stuff inline here
                // for proper wordwrap
                if (m_ft->renderGlyph(c, doBlit, m_penX, m_penY, &dx, &dy, &m_lineHeight)) {
                    m_penX += dx;
                    m_penY += dy;
                }
                if (m_penX >= width-1 - settings.marginRight) {
                    ++p;
                    --len;
                    break;
                }
                m_col ++;
            }
        }

        // Word-wrap or hard linefeed, but avoid the two back-to-back.
        if ((*p == '\n' && !wordWrapped) || m_penX >= width-1 - settings.marginRight) {
            m_col = 0;
            m_penX = settings.marginLeft;
            m_penY += m_lineHeight;
            if (*p == '\n') {
                p++;
                len--;
            } else {
                wordWrapped = true;
            }
            if (m_penY > (int)m_fb->height() - settings.marginBottom) {
                return p - start;
            }
        }
    } while (len > 0);
    return -1;  // think of this as "failed to cross page boundary"
}

int RenderFb::render(unsigned int pageNum, bool doBlit)
{
    m_penX = settings.marginLeft;
    m_penY = settings.marginTop;
    m_fb->clear();

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
                        ASSERT(strOffset <= str->size());
                        int breakOffset = outputWrapped(str, strOffset, doBlit);
                        strOffset = 0;
                        if (breakOffset >= 0) {
                            if (!doBlit) {
                                m_pagination.set(pageNum, i-2, breakOffset);
                            }
                            m_fb->update(0, 0, m_fb->width(), m_fb->height(), false); // DDD
                            return 0;
                        }
                        i += sizeof(clc::Buffer*);
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
    m_fb->update(0, 0, m_fb->width(), m_fb->height(), false); // DDD
    return 1;
}
