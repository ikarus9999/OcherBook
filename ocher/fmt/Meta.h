#ifndef OCHER_META_H
#define OCHER_META_H

#include "clc/data/Buffer.h"

class Meta
{
public:
    clc::Buffer author;
    clc::Buffer title;
    clc::Buffer language;

    clc::Buffer icon;

    unsigned int pages;

    unsigned int pageNum;
    clc::Buffer format;
};

#endif

