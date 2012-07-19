#ifndef OCHER_UI_FACTORY_FD_H
#define OCHER_UI_FACTORY_FD_H

#include "ocher/ui/Factory.h"
#include "ocher/ui/fd/BrowseFd.h"
#include "ocher/ui/fd/RenderFd.h"


class UiFactoryFd : public UiFactory
{
public:
    UiFactoryFd(int inFd, int outFd);

    bool init();
    const char* getName();
    Browse& getBrowser();
    Renderer& getRenderer();

protected:
    BrowseFd m_browser;
    RendererFd m_renderer;
};

#endif


