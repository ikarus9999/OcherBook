#ifndef OCHER_FRAMEBUFFER_H
#define OCHER_FRAMEBUFFER_H

#include "ocher/output/FreeType.h"

class FrameBuffer
{
public:
    virtual unsigned int height() = 0;
    virtual unsigned int width() = 0;
    virtual unsigned int dpi() = 0;

protected:
    FreeType m_ft;


};

#endif

