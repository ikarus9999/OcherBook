#ifndef OCHER_FMT_TEXT_LAYOUT_H
#define OCHER_FMT_TEXT_LAYOUT_H

#include "clc/data/Buffer.h"
#include "ocher/layout/Layout.h"


class Text;

class LayoutText : public Layout
{
public:
    LayoutText(Text *text) : m_text(text) {}

    void append(clc::Buffer text);

protected:
    Text *m_text;
};

#endif

