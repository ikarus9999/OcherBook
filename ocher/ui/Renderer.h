#ifndef OCHER_RENDERER_H
#define OCHER_RENDERER_H

#include "clc/data/Buffer.h"


class Attrs {
public:
    Attrs() : ul(0), b(0), em(0), pre(0), ws(0), nl(0), pts(12) {}
    int ul;
    int b;
    int em;
    int pre;
    int ws;
    int nl;
    int pts;   ///< text points
    // TODO text size, ...
};


class Renderer
{
public:
    Renderer();

    virtual void init(clc::Buffer layout) { m_layout = layout; }
    virtual void render(unsigned int pageNum) = 0;

protected:
    clc::Buffer m_layout;



};

#endif

