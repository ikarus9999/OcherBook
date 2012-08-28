#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <new>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <shlobj.h>
#else
#include <dirent.h>
#include <fnmatch.h>
#if defined(__MACH__)
#include <sys/param.h>
#include <sys/mount.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/statfs.h>
#else
#include <sys/statvfs.h>
#endif
#endif

#include "clc/data/Buffer.h"
#include "clc/data/List.h"
#include "clc/storage/Directory.h"
#include "clc/storage/Path.h"


namespace clc
{

int Directory::mkdirs(const char *path, mode_t mode)
{
#if 0
    strchr(path, '/');
    r = clc_mkdir(path, mode);
    if (r != 0) {
        r = errno;
    }
    umask(oldmask);
    return r;
#endif
    return -1;
}


Directory::Directory() : m_dp(0)
{
}

Directory::Directory(const char* dir) : m_name(dir), m_dp(0)
{
}

Directory::~Directory()
{
    close();
}

void Directory::close()
{
#ifdef _WIN32
    if (m_dp) {
        FindClose(m_dp);
        m_dp = 0;
    }
#else
    if (m_dp) {
        closedir(m_dp);
        m_dp = 0;
    }
#endif
}

void Directory::setTo(const char* dir)
{
    close();
    m_name = dir;
}

int Directory::getNext(Buffer& entryName, struct Stat* s, unsigned int flags)
{
    entryName.clear();
    if (!m_dp && !m_name.length())
        return ENOTDIR;
#ifdef _WIN32
#define SHOULD_SKIP(name) ((flags&IMPLICIT)==0 && (strcmp((name), ".")==0 || strcmp((name), "..")==0))
    WIN32_FIND_DATA fd;
    char* name = &fd.cFileName[0];
    if (!m_dp) {
        Buffer glob = Path::join(m_name.c_str(), "*");
        m_dp = FindFirstFile(glob, &fd);
        if (m_dp) {
            if (! SHOULD_SKIP(name))
                goto done;
            // else fall through to FindNextFile
        } else
            goto err;
    }
    do {
        if (FindNextFile(m_dp, &fd) == 0)
            goto err;
    }  while (SHOULD_SKIP(name));
done:
    entryName = name;
    if (s)
        s->setTo(&fd);
    return 0;
err:
    int error = GetLastError();
    if (error == ERROR_NO_MORE_FILES)
        error = 0;
    return error;
#undef SHOULD_SKIP
#else
    if (! m_dp) {
        m_dp = opendir(m_name);
        if  (! m_dp)
            return errno;
    }
    errno = 0;
    struct dirent* de;
    do {
        de = readdir(m_dp);
    } while (de && !(flags & IMPLICIT) && (strcmp(de->d_name, ".")==0 || strcmp(de->d_name, "..")==0));
    if (de) {
        entryName = de->d_name;
        if (s)
            return s->setTo(m_name, de);
        else
            return 0;
    } else {
        return errno;
    }
#endif
}

uint64_t Directory::countBytes()
{
    uint64_t bytes = 0;
    List dirs;
    Directory* dir;
    Buffer dirName(m_name);
    Buffer name;
    struct Stat statBuf;

scanNewDir:
    dir = new Directory(dirName);
    do {
        while (dir->getNext(name) == 0 && name.length()) {
            Buffer entryName = Path::join(dir->getName().c_str(), name);
            statBuf.setTo(entryName);
            if (statBuf.isDir()) {
                bytes += ((statBuf.size + 511) & ~511);
                dirs.add(dir);
                dirName = entryName;
                goto scanNewDir;
            } else if (statBuf.isReg()) {
                bytes += ((statBuf.size + 511) & ~511);
            }
        }
        delete dir;
        if (dirs.size()) {
            dir = (Directory*)dirs.remove(dirs.size()-1);
        } else
            dir = 0;
    } while (dir);
    return bytes;
}

#ifdef _WIN32
void Stat::setTo(WIN32_FIND_DATA* src)
{
    size = (((uint64_t)src->nFileSizeHigh)<<32)+src->nFileSizeLow;
    mode = src->dwFileAttributes;
}
int Stat::setTo(const Buffer& name)
{
    HANDLE h;
    WIN32_FIND_DATA dp;
    if ((h = FindFirstFile(name.c_str(), &dp)) != INVALID_HANDLE_VALUE) {
        setTo(&dp);
        return 0;
    } else {
        return GetLastError();
    }
}
bool Stat::isDir() const { return (mode&FILE_ATTRIBUTE_DIRECTORY) != 0; }
bool Stat::isReg() const { return (mode&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_DEVICE)) == 0; }
bool Stat::isDev() const { return (mode&FILE_ATTRIBUTE_DEVICE) != 0; }
bool Stat::isReadable() const { return true; }
bool Stat::isWritable() const { return (mode&FILE_ATTRIBUTE_READONLY) == 0; }
#else
void Stat::setTo(struct ::stat* s)
{
    size = s->st_size;
    mode = s->st_mode;
}
int Stat::setTo(const Buffer& dirName, struct dirent* de)
{
    Buffer fileName = Path::join(dirName.c_str(), de->d_name);
    return setTo(fileName);
}
int Stat::setTo(const Buffer& name)
{
    struct stat buf;
    if (lstat(name.c_str(), &buf) != 0)
        return errno;
    setTo(&buf);
    return 0;
}
bool Stat::isDir() const { return S_ISDIR(mode); }
bool Stat::isReg() const { return S_ISREG(mode); }
bool Stat::isDev() const { return S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode) || S_ISSOCK(mode); }
bool Stat::isReadable() const { return (mode&S_IRUSR); }
bool Stat::isWritable() const { return (mode&S_IWUSR); }
#endif


}

