#ifndef OCHER_FD_RENDERER_H
#define OCHER_FD_RENDERER_H

#include "ocher/ui/Renderer.h"

class RendererFd : public Renderer
{
public:
    RendererFd(clc::Buffer layout, int fd);

    void render(unsigned int offset, unsigned int pageNum);

protected:
    int m_fd;
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

