#include "ocher/ux/fd/BrowseFd.h"
#include "ocher/settings/Options.h"


BrowseFd::BrowseFd()
{
}

bool BrowseFd::init()
{
    m_in = opt.inFd;
    m_out = opt.outFd;
    return true;
}

void BrowseFd::browse()
{
}
