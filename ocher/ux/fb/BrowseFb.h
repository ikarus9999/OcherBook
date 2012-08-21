#ifndef OCHER_UX_FB_BROWSE_H
#define OCHER_UX_FB_BROWSE_H

#include "ocher/ux/Browse.h"

class BrowseFb : public Browse
{
public:
    BrowseFb() {}
    ~BrowseFb() {}

    bool init() { return true; }
    void browse() {}

};

#endif


