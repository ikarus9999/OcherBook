#ifndef LIBCLC_EXCEPTION_H
#define LIBCLC_EXCEPTION_H

/**
 *  @file Common exceptions.
 *
 *  Note that all exceptions derive from std::exception, not std::runtime_error.  The latter pulls
 *  in std::string.
 *
 *  @todo  better state than a string.  e.g., include file
 */

#include <exception>

namespace clc
{

class Exception : public std::exception
{
public:
    Exception(char const* what="") throw() : m_what(what) {}
    virtual char const* what() const throw() { return m_what; }
private:
    char const* m_what;
//        Exception(Exception const&);  ///< Unimplemented; disallowed
//        Exception& operator=(Exception const&);  ///< Unimplemented; disallowed
};

class IOException : public Exception
{
public:
    IOException(char const* what="") throw() : Exception(what), fn(0), err(0) {}
    IOException(char const* fn, int e) throw() : Exception("IOException"), fn(fn), err(e) {}
    // TODO:  istream or ostream pointer
    const char* fn;
    int err;
};

class BufferOverflowException : public Exception
{
public:
    BufferOverflowException(char const* what="") throw() : Exception(what) {}
};

#if 0
class BufferUnderflowException : public Exception
{
public:
    BufferUnderflowException(char const* what="") throw() : Exception(what) {}
};

class IndexOutOfBoundsException : public Exception
{
public:
    IndexOutOfBoundsException(char const* what="") throw() : Exception(what) {}
};

class UnsupportedOperationException : public Exception
{
public:
    UnsupportedOperationException(char const* what="") throw() : Exception(what) {}
};

class IllegalArgumentException : public Exception
{
public:
    IllegalArgumentException(char const* what="") throw() : Exception(what) {}
};

class IllegalFormatException : public Exception
{
public:
    IllegalFormatException(char const* what="") throw() : Exception(what) {}
};

class IllegalStateException : public Exception
{
public:
    IllegalStateException(char const* what="") throw() : Exception(what) {}
};
#endif

}

#endif
