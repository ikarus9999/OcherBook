#ifndef OCHER_FB_RENDER_H
#define OCHER_FB_RENDER_H

#include "ocher/ux/Renderer.h"

class FreeType;
class FrameBuffer;


class RenderFb : public Renderer
{
public:
    RenderFb(FreeType *ft, FrameBuffer *fb);

    bool init();
    int outputWrapped(clc::Buffer *b, unsigned int strOffset, bool doBlit);
    int render(unsigned int pageNum, bool doBlit);

protected:
    FreeType *m_ft;
    FrameBuffer *m_fb;
    int m_col;
    int m_penX;
    int m_penY;
    int m_lineHeight;
    int m_page;

    void pushAttrs();
    void applyAttrs(int i);
    void popAttrs();

    Attrs a[10];
    int ai;
};

#endif


