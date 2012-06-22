#include <stdio.h>
#include <stdlib.h>

#include "clc/support/Logger.h"
#include "clc/support/LogAppenders.h"

#include "Epub.h"
#include "Browse.h"


void initLog()
{
    static clc::LogAppenderCFile appender(stderr);
    clc::Logger *l = clc::Log::get("ocher");
    l->setLevel(clc::Log::Debug);  // Release builds compile logging out
    l->setAppender(&appender);
}

void openEpub(const char *epubFile)
{
}

int main(int argc, char **argv)
{
    initLog();

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <epub-file>\n", argv[0]);
        exit(1);
    }

    return browse(argc, argv);
}


