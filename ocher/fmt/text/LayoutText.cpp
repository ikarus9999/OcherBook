#include "ocher/fmt/text/Text.h"
#include "ocher/fmt/text/LayoutText.h"


LayoutText::LayoutText(Text *text) : m_text(text)
{
    // TODO:  \n\n means real line break

    unsigned int n = m_text->m_text.size();
    const char *raw = m_text->m_text.data();
    for (unsigned int i = 0; i < n; ++i) {
        outputChar(raw[i]);
    }
    flushText();
}


