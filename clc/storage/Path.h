#ifndef LIBCLC_PATH_H
#define LIBCLC_PATH_H

#include "clc/support/Flattenable.h"
#include "clc/data/Buffer.h"
#include "clc/data/List.h"

namespace clc
{

class Directory;
class Entry;
struct entry_ref;

class Path
{
public:
    /**
     *  @return  True if the path is an absolute path on this platform.
     */
    static bool isAbsolute(const char* path);

    /**
     *  @param base  The base pathname.
     *  @param leaf  The portion to be appended.  Note that absolute paths override
     *      existing data in base.
     *  @return The combined pathname.
     */
    static Buffer join(const char* base, const char* leaf);

    /**
     *  @param base  The base pathname.  Modified by appending leaf.
     *  @param leaf  The portion to be appended.  Note that absolute paths override
     *      existing data in base.
     *  @return Reference to the modified base pathname.
     */
    static Buffer& join(Buffer& base, const char* leaf);

    static void split(const char* path, Buffer& base, Buffer& file);

    /**
     *  Returns a list of files that match a pattern in a directory.  Note that all matching names
     *  are returned regardless of type (directory, regular file, link, etc).
     *  @param directory  The directory in which to list files
     *  @param glob  The filename globbing pattern, using literal characters, ?, and *.
     *      NULL implies * (all files).
     *  @param files  A list to receive Buffer*.  Caller must delete each.
     *  @return 0 on success; nonzero for an error (files list may be partial).  Even on error,
     *      caller is responsbile for cleaning up files list.
     */
    static int list(const char* directory, const char* glob, List& files);

    static Buffer getDirectory(Buffer &path);

#if 0
    Path();
    Path(const Path& path);
    Path(const entry_ref* ref);
    Path(const Entry* entry);
    Path(const char* dir, const char* leaf = NULL, bool normalize = false);
    Path(const Directory* dir, const char* leaf = NULL, bool normalize = false);

    ~Path();

    int  InitCheck() const;

    int  SetTo(const entry_ref* ref);
    int  SetTo(const Entry* entry);
    int  SetTo(const char* path, const char* leaf = NULL, bool normalize = false);
    int  SetTo(const Directory* dir, const char* leaf = NULL, bool normalize = false);
    void   Unset();

    int  Append(const char* path, bool normalize = false);

    const char*  Leaf() const;
    int  GetParent(Path* path) const;

    bool   operator==(const Path& item) const;
    bool   operator==(const char* path) const;
    bool   operator!=(const Path& item) const;
    bool   operator!=(const char* path) const;
    Path&   operator=(const Path& item);
    Path&   operator=(const char* path);

#if 0
    // Flattenable protocol
    bool   IsFixedSize() const;
    type_code  TypeCode() const;
    ssize_t   FlattenedSize() const;
    int  Flatten(void* buffer, ssize_t size) const;
    bool   AllowsTypeCode(type_code code) const;
    int  Unflatten(type_code code, const void* buffer, ssize_t size);
#endif

private:
    int  _SetPath(const char* path);
    static bool   _MustNormalize(const char* path, int* _error);

    Buffer m_path;
#endif
};

}

#endif // _PATH_H
