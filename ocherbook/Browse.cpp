#include "Browse.h"
#include "Layout.h"
#include "Epub.h"


void openBook(const char *filename)
{
    Epub epub(filename);
    printf("title       : %s\n", epub.m_title.c_str());
    printf("epub version: %s\n", epub.m_epubVersion.c_str());
    printf("uid         : %s\n", epub.m_uid.c_str());

    clc::Buffer html;
    for (int i = 0; ; i++) {
        if (epub.getSpineItemByIndex(i, html) != 0)
            break;
        render(html, &epub);
    }
}

int browse(int argc, char **argv)
{
    openBook(argv[1]);
    return 0;
}
