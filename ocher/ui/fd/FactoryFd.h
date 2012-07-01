#ifndef OCHER_UI_FACTORY_FD_H
#define OCHER_UI_FACTORY_FD_H

#include "ocher/ui/Factory.h"
#include "ocher/ui/fd/BrowseFd.h"

class UiFactoryFd : public UiFactory
{
public:
    UiFactoryFd(int inFd, int outFd);

    bool init();
    const char* getName();
    Browse& getBrowser();

protected:
    BrowseFd m_browser;
};

#endif


