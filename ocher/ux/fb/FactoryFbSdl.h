#ifndef OCHER_UX_FACTORY_FB_SDL_H
#define OCHER_UX_FACTORY_FB_SDL_H

#include "ocher/ux/fb/FactoryFb.h"
#include "ocher/output/sdl/FbSdl.h"


class UiFactoryFbSdl : public UiFactoryFb
{
public:
    UiFactoryFbSdl();
    ~UiFactoryFbSdl();

    bool init();
    const char* getName();

protected:
    FbSdl m_fb;
};

#endif

