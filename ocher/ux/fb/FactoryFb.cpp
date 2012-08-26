#include "ocher/ux/fb/FactoryFb.h"
#include "ocher/ocher.h"


UiFactoryFb::UiFactoryFb(FrameBuffer *fb) :
    m_ft(fb),
    m_render(&m_ft, fb)
{
}

bool UiFactoryFb::init()
{
    return true;
}

void UiFactoryFb::deinit()
{
}

Browse& UiFactoryFb::getBrowser()
{
    return m_browser;
}

Renderer& UiFactoryFb::getRenderer()
{
    return m_render;
}



