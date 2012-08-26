#ifndef OCHER_CURSES_RENDER_H
#define OCHER_CURSES_RENDER_H

#include "clc/tui/Tui.h"

#include "ocher/ux/Renderer.h"


class RenderCurses : public Renderer
{
public:
    RenderCurses();

    bool init(clc::Tui* tui);
    int render(unsigned int pageNum, bool doBlit);

    int outputWrapped(clc::Buffer *b, unsigned int strOffset, bool doBlit);

protected:
    clc::Window* m_window;
    int m_width;
    int m_height;
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

