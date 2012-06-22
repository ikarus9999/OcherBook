#ifndef OCHER_EPUB_PARSER_H
#define OCHER_EPUB_PARSER_H

#include <map>
#include <vector>

#include "clc/data/Buffer.h"

#include "UnzipCache.h"


struct EpubItem
{
    clc::Buffer href;
    clc::Buffer mediaType;
};

class Epub : public UnzipCache
{
public:
    Epub(const char* epubFilename, const char *password=0);

    clc::Buffer m_epubVersion;
    clc::Buffer m_uid;
    clc::Buffer m_title;

    int getSpineItemByIndex(unsigned int i, clc::Buffer &item);
    int getManifestItemById(unsigned int i, clc::Buffer &item);
    int getContentByHref(const char *href, clc::Buffer &item);

protected:
    TreeFile* findSpine();
    void parseSpine(TreeFile* spine);

    std::map<clc::Buffer, EpubItem> m_items;
    std::vector<clc::Buffer> m_spine;
    clc::Buffer m_contentPath;  ///< directory of full-path attr
};


#endif

