#ifndef OCHER_UX_FACTORY_FB_H
#define OCHER_UX_FACTORY_FB_H

#include "ocher/ux/Factory.h"
#include "ocher/ux/fb/BrowseFb.h"
#include "ocher/ux/fb/RenderFb.h"
#include "ocher/output/FreeType.h"


class UiFactoryFb : public UiFactory
{
public:
    UiFactoryFb(FrameBuffer *fb);
    ~UiFactoryFb() {}

    bool init();
    void deinit();
    Browse& getBrowser();
    Renderer& getRenderer();

protected:
    FreeType m_ft;
    BrowseFb m_browser;
    RenderFb m_render;
};

#endif



