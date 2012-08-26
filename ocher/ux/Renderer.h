#ifndef OCHER_UX_RENDERER_H
#define OCHER_UX_RENDERER_H

#include "clc/data/Buffer.h"

#include "ocher/ux/Pagination.h"


/**
 */
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
};


/**
 */
class Renderer
{
public:
    Renderer();
    virtual ~Renderer() {}

    virtual bool init() { return true; }

    virtual void set(clc::Buffer layout) { m_layout = layout; }

    /**
     * Render the page.
     * @return -1 if this is an unknown page (prior page not paginated),
     *  0 if reached the end of the page and it overflowed;
     *  1 if reached the end of the layout (no overflow)
     */
    virtual int render(unsigned int pageNum, bool doBlit) = 0;

protected:
    clc::Buffer m_layout;
    Pagination m_pagination;
};

#endif

