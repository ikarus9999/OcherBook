#ifndef OCHER_UX_FD_BROWSE_H
#define OCHER_UX_FD_BROWSE_H

#include "ocher/ux/Browse.h"

class BrowseFd : public Browse
{
public:
    BrowseFd();
    ~BrowseFd() {}

    bool init();
    void browse();

protected:
    int m_in;
    int m_out;
};

#endif

