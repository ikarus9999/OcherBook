#include <fnmatch.h>

#include "clc/support/Logger.h"
#include "clc/storage/Path.h"
#include "UnzipCache.h"


UnzipCache::UnzipCache(const char *filename, const char *password) :
    m_uf(0), m_root(0), m_filename(filename), m_password(password ? password : "")
{
    newCache();
}

UnzipCache::~UnzipCache()
{
    if (m_uf)
        unzClose(m_uf);
    clearCache();
}

void UnzipCache::newCache()
{
    clc::Buffer rootName(".");
    m_root = new TreeDirectory(rootName);
}

void UnzipCache::clearCache()
{
    if (m_root) {
        delete m_root;
        m_root = 0;
    }
}

TreeFile *UnzipCache::getFile(const char* filename, const char* relative)
{
    clc::Buffer fullPath;
    if (relative) {
        fullPath = clc::Path::join(relative, filename);
        filename = fullPath.c_str();
    }
    TreeFile *f = m_root->findFile(filename);
    if (f)
        return f;
    if (unzip(filename, NULL)) {
        // Even if error, may have extracted.
        f = m_root->findFile(filename);
    }
    return f;
}

int UnzipCache::unzipFile(const char *pattern, clc::Buffer *matchedName)
{
    char pathname[256];
    int err;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(m_uf, &file_info, pathname, sizeof(pathname), NULL, 0, NULL, 0);
    if (err != UNZ_OK) {
        clc::Log::error("ocher.epub.unzip", "unzGetCurrentFileInfo: %d", err);
        return -1;
    }

    int match = 1;
    if (pattern) {
        // TODO:  allow match to be looser:  leading ./, \, etc.  Anything but wildcards.
        if (strcmp(pattern, pathname) == 0)
            match = 2;
        int r = fnmatch(pattern, pathname, FNM_NOESCAPE /*| FNM_CASEFOLD*/);
        if (r == 0) {
            clc::Log::trace("ocher.epub.unzip", "matched %s to %s", pathname, pattern);
            if (matchedName)
                matchedName->setTo(pathname);
        } else if (r == FNM_NOMATCH) {
            clc::Log::trace("ocher.epub.unzip", "did not match %s to %s", pathname, pattern);
            return 0;
        } else {
            clc::Log::error("ocher.epub.unzip", "fnmatch: %s: error", pattern);
            return -1;
        }
    }

    clc::Buffer buffer;
    clc::Buffer filename;
    TreeFile *tfile = 0;

    char *start = pathname;
    char *p = start;
    TreeDirectory *root = m_root;
    while(1) {
        if (*p == '/' || *p == '\\' || *p == 0) {
            if (p-start) {
                clc::Buffer name(start, p-start);
                if (*p) {
                    root = root->createDirectory(name);
                }
                else {
                    tfile = root->createFile(name, buffer);
                    filename = name;
                    clc::Log::debug("ocher.epub.unzip", "Creating file %s", filename.c_str());
                }
            }
            if (! *p)
                break;
            start = p + 1;
        }
        p++;
    };

    if (tfile) {
        char * buf = buffer.lockBuffer(file_info.uncompressed_size);

        err = unzOpenCurrentFilePassword(m_uf, m_password.empty() ? NULL : m_password.c_str());
        if (err != UNZ_OK) {
            clc::Log::error("ocher.epub.unzip", "unzOpenCurrentFilePassword: %d", err);
        } else {
            clc::Log::info("ocher.epub.unzip", "extracting: %s", pathname);

            do {
                err = unzReadCurrentFile(m_uf, buf, buffer.size());
                if (err < 0) {
                    clc::Log::error("ocher.epub.unzip", "unzReadCurrentFile: %d", err);
                }
            } while (err > 0);
        }
        buffer.unlockBuffer();
        tfile->data = buffer;

        if (err == UNZ_OK) {
            err = unzCloseCurrentFile(m_uf);
            if (err != UNZ_OK) {
                clc::Log::error("ocher.epub.unzip", "unzCloseCurrentFile: %d", err);
            }
        } else
            unzCloseCurrentFile(m_uf);    /* don't lose the error */
    }

    return err == UNZ_OK ? match : -1;
}

int UnzipCache::unzip(const char *pattern, std::list<clc::Buffer> *matchedNames)
{
    uLong i;
    unz_global_info64 gi;
    int numMatched = 0;

    if (m_uf)
        unzClose(m_uf);
    m_uf = unzOpen64(m_filename.c_str());
    int err = unzGetGlobalInfo64(m_uf, &gi);
    if (err != UNZ_OK) {
        clc::Log::error("ocher.epub.unzip", "unzGetGlobalInfo: %d", err);
        return -1;
    }

    for (i = 0; i < gi.number_entry; i++) {
        int r;
        if (matchedNames) {
            clc::Buffer matchedName;
            r = unzipFile(pattern, &matchedName);
            if (!matchedName.empty())
                matchedNames->push_back(matchedName);
        } else {
            r = unzipFile(pattern, NULL);
        }
        if (r > 0) {
            ++numMatched;
            if (r > 1)
                break;
        }

        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(m_uf);
            if (err != UNZ_OK) {
                clc::Log::error("ocher.epub.unzip", "unzGoToNextFile: %d", err);
                return -1;
            }
        }
    }

    return numMatched;
}


