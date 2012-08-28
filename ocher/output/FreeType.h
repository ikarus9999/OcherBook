#ifndef OCHER_FREETYPE_H
#define OCHER_FREETYPE_H

#include <ft2build.h>
#include FT_FREETYPE_H

class FrameBuffer;

class FreeType
{
public:
    FreeType(FrameBuffer *fb);

    bool init();
    void setSize(unsigned int points);
    bool renderGlyph(int c, bool doBlit, int penX, int penY, int *dx, int *dy, int *height);

protected:
    FT_Library m_lib;
    FT_Face m_face;

    FrameBuffer *m_fb;
};

#endif

