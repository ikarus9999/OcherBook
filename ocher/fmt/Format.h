#ifndef OCHER_FMT_H
#define OCHER_FMT_H

#include "clc/data/Buffer.h"

class Format
{
public:
    Format(const char *epubFilename);

    virtual clc::Buffer getFormatName() = 0;
};


#endif

