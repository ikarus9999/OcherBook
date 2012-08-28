#ifndef LIBCLC_DIRECTORY_H
#define LIBCLC_DIRECTORY_H

#include <stdint.h>
#ifndef _WIN32
#include <sys/stat.h>
#include <dirent.h>
#endif
#include "clc/data/Buffer.h"


namespace clc
{

/**
 *  Minimal portable version of the POSIX stat struct.
 */
struct Stat
{
    uint64_t size;
    int mode;
    // TODO: creation time
    // TODO: access time
    // TODO: modify time
    bool isReg() const;
    bool isDir() const;
    bool isDev() const;
    bool isReadable() const;
    bool isWritable() const;

#ifdef _WIN32
    void setTo(WIN32_FIND_DATA* src);
#else
    void setTo(struct ::stat* s);
    int setTo(const Buffer& dirName, struct dirent* de);
#endif
    int setTo(const Buffer& name);
};

/**
 *  Iterable directory.
 */
class Directory
{
public:
    /**
     */
    static void mkdir(const char* dir);

    /**
     * Creates a directory at the specified path.  Does not create intermediate directories.
     * @param path the path of the directory to create
     * @return 0 or errno
     */
    static int mkdirs(const char *path);

    enum {
        IMPLICIT = 1,  // Include the implicit '.' and '..' entries
        FOLLOW_LINKS = 2,
    };

    Directory();

    Directory(const char* dir);

    ~Directory();

    void setTo(const char* dir);

    /**
     * @param[out] entryName  The name of the next entry, or empty string if no more entries.
     * @param[out] s  Pointer to a Stat struct to fill out for this entry.  May be NULL if not
     *     desired.  May be more efficient than separately filling out the Stat struct
     *     based on the entryName.
     * @param[in] flags  IMPLICIT: include the . and .. entries
     * @return Zero success (including end-of-iter), or nonzero on error.  More specifically:
     *     Zero, entryName not empty: entry was sucessfully retrieved and (optionally) stat'd.
     *     Zero, entryName empty: end of iteration.
     *     Nonzero, entryName not empty: entry was sucessfully retrieved, but stat failed.  Valid to
     *     continue.
     *     Nonzero, entryName empty: iteration failed.
     */
    int getNext(Buffer& entryName, struct Stat* s=NULL, unsigned int flags=0);

    void close();

    const Buffer getName() const { return m_name; }

    /**
     *  Convenience function to calculate the total number of bytes used in a directory.
     *  Count may not be exact for multiple reasons:  Errors (such as permissions preventing
     *  recursing into a subdir) are silently ignored.  512-byte blocks (with no tail packing)
     *  is assumed.
     *  @return Bytes
     */
    uint64_t countBytes();

protected:
    Buffer m_name;
#ifdef _WIN32
    HANDLE m_dp;
#else
    DIR* m_dp;
#endif
};

}

#endif

