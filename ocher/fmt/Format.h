#ifndef OCHER_FMT_H
#define OCHER_FMT_H

#include "clc/data/Buffer.h"

/**
 *  Base class for file-format readers.
 */
class Format
{
public:
    virtual clc::Buffer getFormatName() = 0;
};


#endif

