#ifndef LIBCLC_FILE_H
#define LIBCLC_FILE_H

#include <string>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include "clc/data/Buffer.h"


namespace clc
{

/**
 *  Portable blocking file IO.  Similar to the POSIX model but with some conveniences
 *  and some portability.
 *
 *  When you construct (or otherwise initialize) a clc::File, the file is automatically opened.
 *  The file is closed when you re-initialize or destroy the object.
 *
 *  At each initialization, you're asked to supply an "open mode" value. This is a combination of
 *  flags that tells the object whether you want to read and/or write the file, create it if it
 *  doesn't exist, truncate it, and so on.
 *
 *  Since there are portability issues with using off_t or assuming 2G or 4G max files, this code
 *  explicitly uses uint64_t for all sizes and offsets.
 *
 *  Files are always treated as binary.  No implicit conversion is performed.
 *
 *  @todo  Deal with UTF8 on Windows
 *  @todo  Rationalize exceptions vs return codes
 */
class File
{
public:
    enum SeekMethod {
        FromBeg,
        FromCur,
        FromEnd
    };

    /**
     *  Constructs an empty File object, which is not usable for IO.  Call setTo before using.
     */
    File();

    /**
     *  Constructor.
     *  @param mode  Similar to the mode string expected by fopen ("r", "r+", "w", etc).  For
     *      compatibility, binary may be specified with "b", but is always assumed anyway.
     *      Optional (non-C-like) suffixes include "x" (fail if file already exists), "u"
     *      (create unique filename based off specified filename), and "t" for a temporary
     *      file (automatically unlink in destructor).  In summary, mode must
     *      match [rwa]b?+?b?[xut]?
     *  @throw  IOException if open failed
     */
    File(const Buffer& filename, const char *mode="r");
    File(const char* filename, const char *mode="r");

    /**
     *  See Constructor.
     */
    void setTo(const Buffer& filename, const char *mode="r");
    void setTo(const char* filename, const char *mode="r");

    /**
     *  Closes the file, and returns the object to an uninitialized state.
     */
    void unset();

    /**
     *  Destructor; closes the file.
     */
    ~File();

    /**
     *  Returns the name of the file (as specified in the constructor, but possibly modified for
     *  unique files).
     *  @return The name of the file.
     */
    Buffer getName() const { return m_filename; }

    /**
     *  @return The current position in the file.
     */
    uint64_t position() const;

    /**
     *  @param offset
     *  @param  how  What offset is relative to
     *  @throw  IOException if seek failed
     *  @return  The new absolute offset
     */
    uint64_t seek(int64_t offset, SeekMethod how);

    /**
     *  Attempts to read numBytes into buffer, from the current position of the file.
     *  @param buffer
     *  @param numBytes
     *  @return  The number of bytes read.  Less than requested indicates EOF.
     *  @throw  IOException on error (EOF is not an error)
     */
    uint32_t read(char* buffer, uint32_t numBytes);

    /**
     * Reads a line, up to any and including any line terminator.
     * @param buf the string to read into
     * @param keepEol Whether to keep or strip the end of line character, if any.
     * @param maxLen Maximum reasonable length (or 0 for unlimited); greater causes an exception
     * @throw IOException on error (EOF is not an error)
     * @throw BufferOverflowException if maxLen exceeded
     */
    Buffer readLine(bool keepEol=false, size_t maxLen=0);

    /**
     *  Reads numBytes into buffer at offset.
     *  @throw  IOException on error (EOF is not an error)
     */
    uint32_t read(Buffer& buffer, uint32_t numBytes, uint32_t offset=0);

    /**
     *  Reads and returns the remainder of the file, appending it to the Buffer.
     *  @param[out] b  Rest of file is appended to this buffer.
     *  @throw  IOException on error (EOF is not an error)
     */
    void readRest(Buffer& b);

    /**
     *  @throw  IOException
     */
    void write(const char* buf, uint32_t numBytes);
    void write(const Buffer& buf) { write(buf.c_str(), buf.length()); }

    /**
     *  @return The current size, in bytes, of the file.
     */
    uint64_t size();

    /**
     *
     */
    void getTimes(time_t& accessTime, time_t& modifyTime, time_t& changeTime);

    /**
     * Flush any buffered writes to disk.
     */
    void flush();

    /**
     * Closes the file.
     */
    void close();

    /**
     * Indicates if the end-of-file condition is set.
     * @return bool true if it EOF has been reached, false otherwise
     */
    bool isEof() const;

    /**
     * Checks for the existence of a file or directory at the given path.
     * @param path the path to check
     * @return bool true if it exists, false otherwise
     */
    static bool exists(const char *path);

    /**
     * Checks if the entry in the filesystem at the given path is a directory.
     * @param path the path to check
     * @return bool true if the entry is a directory, false otherwise.
     */
    static bool isDirectory(const char *path);

    /**
     * Remove the entry at the given path.  Path may be either a file or a directory.
     * @param path the path to remove
     * @return 0 or errno
     */
    static int removePath(const char *path);

    /**
     * Creates a directory at the specified path.  Does not create intermediate directories.
     * @param path the path of the directory to create
     * @return 0 or errno
     */
    static int mkdir(const char *path);

    /**
     * Renames a file or directory at the given path.
     * @param oldPath the path to rename from
     * @param newPath the path to rename to
     * @return 0 or errno
     */
    static int rename(const char *oldPath, const char *newPath);

protected:
    void init(const char *mode);

    Buffer m_filename;
    FILE *m_fd;
    bool m_temp;
};


}

#endif
