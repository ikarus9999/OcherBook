#ifndef OCHER_FMT_TEXT_H
#define OCHER_FMT_TEXT_H

#include "ocher/fmt/Format.h"


class Text : public Format
{
public:
    Text(const char *filename);
    virtual ~Text() {}

    clc::Buffer getFormatName();

    clc::Buffer m_text;
};

#endif

