#include "ocher/fmt/text/Text.h"


Text::Text(const char *filename)
{
    // TODO
}

clc::Buffer Text::getFormatName()
{
    static clc::Buffer name("TEXT");
    return name;
}

