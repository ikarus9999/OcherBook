#ifndef OCHER_UNZIP_CACHE_H
#define OCHER_UNZIP_CACHE_H

#include <list>

#include "unzip.h"
#include "TreeMem.h"
#include "clc/data/Buffer.h"


/**
 * Unzips zip files to memory on-demand, and caches the results.
 */
class UnzipCache
{
public:
    UnzipCache(const char* zipFilename, const char *password=0);
    ~UnzipCache();

    TreeFile* getFile(const char *filename, const char *relative=0);
    TreeDirectory* getRoot() { return m_root; }

protected:
    void clearCache();
    void newCache();

    /**
     * @param matchedName  The actual name that matched pattern (even if pattern is NULL, and even
     *      if the extraction failed)
     * @return -1 error, 0 did not match, 1 matched and extracted, 2 matched uniquely, quit now
     */
    int unzipFile(const char *pattern, clc::Buffer *matchedName);

    /**
     * Unzips file(s) that match the pattern into the TreeDirectory.
     * @param pattern  If NULL, extract all, otherwise extract only the named file(s).
     * @param matchedNames  If not NULL, extracted pathnames are appended
     * @return -1 stopped due to error, else number matched
     */
    int unzip(const char *pattern, std::list<clc::Buffer> *matchedNames);

    unzFile m_uf;
    TreeDirectory* m_root;
    clc::Buffer m_filename;
    clc::Buffer m_password;
};


#endif

