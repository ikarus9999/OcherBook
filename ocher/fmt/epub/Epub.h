#ifndef OCHER_EPUB_PARSER_H
#define OCHER_EPUB_PARSER_H

#include <map>
#include <vector>

#include "clc/data/Buffer.h"

#include "ocher/fmt/epub/UnzipCache.h"


struct EpubItem
{
    clc::Buffer href;
    clc::Buffer mediaType;
};

class Epub
{
public:
    Epub(const char* epubFilename, const char *password=0);

    clc::Buffer getFormatName();

    clc::Buffer m_epubVersion;
    clc::Buffer m_uid;
    clc::Buffer m_title;

    TreeFile* getFile(const char *filename, const char *relative=0) {
        return m_zip.getFile(filename, relative);
    }

    int getSpineItemByIndex(unsigned int i, clc::Buffer &item);
    int getManifestItemById(unsigned int i, clc::Buffer &item);
    int getContentByHref(const char *href, clc::Buffer &item);

    /**
     * Parses XML. Caller must call mxml_delete.
     */
    mxml_node_t *parseXml(clc::Buffer &xml);

protected:
    TreeFile* findSpine();
    void parseSpine(TreeFile* spine);

    UnzipCache m_zip;
    std::map<clc::Buffer, EpubItem> m_items;
    std::vector<clc::Buffer> m_spine;
    clc::Buffer m_contentPath;  ///< directory of full-path attr
};


#endif

