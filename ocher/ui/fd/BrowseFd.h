#ifndef OCHER_UI_FD_BROWSE_H
#define OCHER_UI_FD_BROWSE_H

#include "ocher/ui/Browse.h"

class BrowseFd : public Browse
{
public:
    BrowseFd(int inFd, int outFd);
    ~BrowseFd() {}

    void browse();

protected:
    int m_in;
    int m_out;
};

#endif

