#include "ocher/ux/fd/FactoryFd.h"
#include "ocher/ocher.h"


UX_DRIVER_REGISTER(Fd);


UiFactoryFd::UiFactoryFd()
{
}

bool UiFactoryFd::init()
{
    return m_browser.init() && m_renderer.init();
}

const char* UiFactoryFd::getName()
{
    return "fd";
}

Browse& UiFactoryFd::getBrowser()
{
    return m_browser;
}

Renderer& UiFactoryFd::getRenderer()
{
    return m_renderer;
}

