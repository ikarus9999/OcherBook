#include "ocher/ui/fd/FactoryFd.h"

UiFactoryFd::UiFactoryFd(int inFd, int outFd) :
    m_browser(inFd, outFd),
    m_renderer(outFd)
{
}

bool UiFactoryFd::init()
{
    return true;
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

