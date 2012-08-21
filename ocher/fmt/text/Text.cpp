#include "clc/support/Logger.h"
#include "clc/storage/File.h"

#include "ocher/fmt/text/Text.h"


Text::Text(const char *filename)
{
    clc::File f(filename);
    f.readRest(m_text);
    clc::Log::debug("ocher.fmt.text", "Loaded %u bytes", m_text.size());
}

clc::Buffer Text::getFormatName()
{
    static clc::Buffer name("TEXT");
    return name;
}

