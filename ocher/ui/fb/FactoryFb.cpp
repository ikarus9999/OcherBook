#include "ocher/ui/fb/FactoryFb.h"

UiFactoryFb::UiFactoryFb()
{
}

bool UiFactoryFb::init()
{
    return true;
}

const char* UiFactoryFb::getName()
{
    return "fb";
}

Browse& UiFactoryFb::getBrowser()
{
    return m_browser;
}

Renderer& UiFactoryFb::getRenderer()
{
    return m_renderer;
}



