#ifndef OCHER_UX_FACTORY_FB_H
#define OCHER_UX_FACTORY_FB_H

#include "ocher/ux/Factory.h"
#include "ocher/ux/fb/BrowseFb.h"


class UiFactoryFb : public UiFactory
{
public:
    UiFactoryFb();
    virtual ~UiFactoryFb() {}

    bool init();
    const char* getName();
    Browse& getBrowser();
    Renderer& getRenderer();

protected:
    BrowseFb m_browser;
};

#endif



