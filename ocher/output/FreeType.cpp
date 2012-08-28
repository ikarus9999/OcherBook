#include <ctype.h>

#include "ocher/device/Device.h"
#include "ocher/output/FreeType.h"
#include "ocher/output/FrameBuffer.h"

#include "clc/support/Logger.h"


FreeType::FreeType(FrameBuffer *fb) :
    m_fb(fb)
{
}

bool FreeType::init()
{
    int r;
    r = FT_Init_FreeType(&m_lib);
    if (r) {
        clc::Log::error("ocher.freetype", "FT_Init_FreeType failed: %d", r);
        return false;
    }

    r = FT_New_Face(m_lib, "FreeSans.otf", 0, &m_face);
    if (r || !m_face) {
        clc::Log::error("ocher.freetype", "FT_New_Face failed: %d", r);
        return false;
    }
    return true;
}

void FreeType::setSize(unsigned int points)
{
    FT_Set_Char_Size(m_face, 0, points*64, m_fb->dpi(), m_fb->dpi());
}

// TODO:  how slow is this?  Might be worth caching rendered words (per
// face/size)
bool FreeType::renderGlyph(int c, bool doBlit, int penX, int penY, int *dx, int *dy, int *height)
{
    FT_GlyphSlot slot = m_face->glyph;
    unsigned int glyphIndex = FT_Get_Char_Index(m_face, c);
    int r = FT_Load_Glyph(m_face, glyphIndex, FT_LOAD_DEFAULT);
    if (r) {
        clc::Log::error("ocher.freetype", "FT_Load_Glyph failed: %d", r);
        return false;
    }
    if (m_face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        r = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
        if (r) {
            clc::Log::error("ocher.freetype", "FT_Render_Glyph failed: %d", r);
            return false;
        }
    }

    if (doBlit) {
        //printf("%u(%c) %d %d %d %d\n", c, isprint(c) ? c : '?',
        //        m_face->glyph->bitmap_left, m_face->glyph->bitmap_top,
        //        m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
        m_fb->blit(m_face->glyph->bitmap.buffer, penX + m_face->glyph->bitmap_left,
                penY - m_face->glyph->bitmap_top, m_face->glyph->bitmap.width,
                m_face->glyph->bitmap.rows);
    }

    *dx = slot->advance.x >> 6;
    *dy = slot->advance.y >> 6;
    *height = m_face->size->metrics.height >> 6;
    return true;
}

