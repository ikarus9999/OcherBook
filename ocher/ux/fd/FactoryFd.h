#ifndef OCHER_UX_FACTORY_FD_H
#define OCHER_UX_FACTORY_FD_H

#include "ocher/ux/Factory.h"
#include "ocher/ux/fd/BrowseFd.h"
#include "ocher/ux/fd/RenderFd.h"


class UiFactoryFd : public UiFactory
{
public:
    UiFactoryFd();
    virtual ~UiFactoryFd() {}

    bool init();
    const char* getName();
    Browse& getBrowser();
    Renderer& getRenderer();

protected:
    BrowseFd m_browser;
    RendererFd m_renderer;
};

#endif


