libclc is a library of utility functions that I have built up over the years
to support my projects.  It is intended to be:
 - small and fast (minimal C++ abstraction over OS's primitives)
 - minimal use of STL, for size concerns
 - correct (high unit test coverage)
 - well documented
 - portable to POSIX-like systems (WIN32 on an as-needed basis, but not to the
   point that it interferes with clean coding)
 - examples of what I consider "best practice" (subject to evolution!)
 - minimal configuration, for easy cherry-picking of files into other projects
 - static linking, for easy drop-in to other projects


BUILDING


UNIT TESTING

1. ./configure.sh
2. cd build/projects/libclc/
3. make
4. ./clcTest ../../../projects/libclc/tests/resources


UNIT TEST CODE COVERAGE

1. ./configure.sh
2. cd build/projects/libclc/
3. make
4. coverage/coverage.sh


DOCUMENTATION

Documentation is maintained via Doxygen.  Releases include the generated
documentation.

To view the documentation, open libclc-$VER/doc/doxygen/html/index.html.

To regenerate the documentation:
1. cd libclc-$VER/src
2. doxygen  ../doc/Doxyfile
