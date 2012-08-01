#include "ocher/ocher.h"
#include "ocher/ux/fb/FactoryFbSdl.h"


UX_DRIVER_REGISTER(FbSdl);


UiFactoryFbSdl::UiFactoryFbSdl() :
    UiFactoryFb(&m_fb)
{
}

UiFactoryFbSdl::~UiFactoryFbSdl()
{
}

bool UiFactoryFbSdl::init()
{
    return m_fb.init() && m_render.init();
}

const char* UiFactoryFbSdl::getName()
{
    return "sdl";
}


