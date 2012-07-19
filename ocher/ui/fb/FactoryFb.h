#ifndef OCHER_UI_FACTORY_FB_H
#define OCHER_UI_FACTORY_FB_H

#include "ocher/ui/Factory.h"
#include "ocher/ui/fb/BrowseFb.h"


class UiFactoryFb : public UiFactory
{
public:
    UiFactoryFb();

    bool init();
    const char* getName();
    Browse& getBrowser();
    Renderer& getRenderer();

protected:
    BrowseFb m_browser;
};

#endif



