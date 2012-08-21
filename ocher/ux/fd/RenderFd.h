#ifndef OCHER_FD_RENDERER_H
#define OCHER_FD_RENDERER_H

#include "ocher/ux/Renderer.h"

class RendererFd : public Renderer
{
public:
    RendererFd();

    bool init();
    void render(unsigned int pageNum);

    void setWidth(int width);
    void outputWrapped(clc::Buffer *b);

protected:
    int m_fd;
    int m_width;
    int m_x;
    int m_y;
    int m_page;

    void enableUl();
    void disableUl();
    void enableEm();
    void disableEm();

    void pushAttrs();
    void applyAttrs(int i);
    void popAttrs();

    Attrs a[10];
    int ai;
};

#endif

