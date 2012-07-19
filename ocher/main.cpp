#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "clc/support/Logger.h"
#include "clc/support/LogAppenders.h"

#include "ocher/settings/Options.h"
#include "ocher/ui/Controller.h"
#include "ocher/ui/Factory.h"
#ifdef OCHER_UI_FB
#include "ocher/ui/fb/FactoryFb.h"
#endif
#ifdef OCHER_UI_NCURSES
#include "ocher/ui/ncurses/FactoryNcurses.h"
#endif
#ifdef OCHER_UI_FD
#include "ocher/ui/fd/FactoryFd.h"
#endif

struct Options opt;


void initLog()
{
    static clc::LogAppenderCFile appender(stderr);
    clc::Logger *l = clc::Log::get("ocher");
    l->setAppender(&appender);
    if (opt.verbose < 0)
        l->setLevel(clc::Log::Fatal);
    else if (opt.verbose == 0)
        l->setLevel(clc::Log::Error);
    else if (opt.verbose == 1)
        l->setLevel(clc::Log::Warn);
    else if (opt.verbose == 2)
        l->setLevel(clc::Log::Info);
    else if (opt.verbose == 3)
        l->setLevel(clc::Log::Debug);
    else
        l->setLevel(clc::Log::Trace);
}

void usage(const char *msg)
{
    printf(
        "OcherBook  Copyright (C) 2012 Chuck Coffing  <clc@alum.mit.edu>\n"
        "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
        "\n");
    if (msg) {
        printf("%s\n\n", msg);
    }
    printf(
      // 12345678901234567890123456789012345678901234567890123456789012345678901234567890
        "-d,--directory <dir> Process (browse or validate) all epubs in the directory.\n"
        "-f,--flatten         Flatten (do not show to user) the directory heirarchy.\n"
        "-t,--test            Test (validate) the epubs rather than view.\n"
        "-h,--help            Help.\n"
        "-v,--verbose         Increase logging verbosity.\n"
        "-q,--quiet           Quiet; decrease logging verbosity.\n"
        "   --list-drivers\n"
        "   --driver <driver>\n"
      //"-w             Allow re-writing the epubs.\n"
        "<file>         \n"
    );
    exit(0);
}

#define OPT_DRIVER 256
#define OPT_LIST_DRIVERS 257

int main(int argc, char **argv)
{
    clc::List drivers;
    bool listDrivers = false;

    struct option long_options[] =
    {
        {"directory",    required_argument, 0,'d'},
        {"flatten",      no_argument,       0,'f'},
        {"help",         no_argument,       0,'h'},
        {"quiet",        no_argument,       0,'q'},
        {"test",         no_argument,       0,'t'},
        {"verbose",      no_argument,       0,'v'},
        {"driver",       required_argument, 0, OPT_DRIVER},
        {"list-drivers", no_argument,       0, OPT_LIST_DRIVERS},
        {0, 0, 0, 0}
    };

    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "d:fhtvq", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 0:
                break;
            case 'd':
                opt.dir = optarg;
                break;
            case 'v':
                opt.verbose++;
                break;
            case 'q':
                opt.verbose--;
                break;
            case 'h':
                usage(0);
                break;
            case OPT_DRIVER:
                opt.driverName = optarg;
                break;
            case OPT_LIST_DRIVERS:
                listDrivers = true;
                break;
            default:
                usage("Unknown argument");
                break;
        }
    }

    initLog();

#ifdef OCHER_UI_FB
    drivers.add(new UiFactoryFb());
#endif
#ifdef OCHER_UI_NCURSES
    drivers.add(new UiFactoryNcurses());
#endif
#ifdef OCHER_UI_FD
    drivers.add(new UiFactoryFd(opt.inFd, opt.outFd));
#endif

    UiFactory *driver = 0;
    for (unsigned int i = 0; i < drivers.size(); ++i) {
        UiFactory *factory = (UiFactory*)drivers.get(i);

        if (listDrivers) {
            printf("\t%s\n", factory->getName());
        } else if (opt.driverName) {
            if (strcmp(factory->getName(), opt.driverName) == 0) {
                driver = factory;
                if (!driver->init()) {
                    return 1;
                }
                break;
            }
        } else {
            if (factory->init()) {
                driver = factory;
                break;
            }
        }
    }
    if (listDrivers) {
        return 0;
    }
    if (! driver) {
        printf("No suitable output driver found\n");
        return 1;
    }

    while (optind < argc) {
        // TODO  list
        opt.file = argv[optind++];
    }

    if (!opt.file && !opt.dir) {
        usage("Please specify an epub file or directory.");
    }

    Controller c(driver);
    c.run();

    return 0;
}


