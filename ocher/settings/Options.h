#ifndef OCHER_OPTIONS_H
#define OCHER_OPTIONS_H

struct Options {
    Options() : verbose(0), listDrivers(0), dir(0), inFd(0), outFd(1) {}

    int verbose;

    const char *driverName;
    int listDrivers;

    const char *dir;
    const char *file;

    int inFd;
    int outFd;

};

extern struct Options opt;

#endif

