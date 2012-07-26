#include "ocher/device/Device.h"
#include "ocher/output/FreeType.h"
#include "ocher/output/FrameBuffer.h"

#include "clc/support/Logger.h"


FreeType::FreeType(FrameBuffer *fb) :
    m_fb(fb)
{
    int r;
    r = FT_Init_FreeType(&m_lib);
    if (r) {
    }

    r = FT_New_Face(m_lib, "build/freefont-20120503/FreeSans.otf", 0, &m_face);
    if (r) {
    }
}

void FreeType::setSize(unsigned int points)
{
    FT_Set_Char_Size(m_face, 0, points*64, m_fb->dpi(), m_fb->dpi());
}

