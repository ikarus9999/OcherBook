#include "ocher/ux/ncurses/FactoryNC.h"
#include "ocher/ocher.h"


UX_DRIVER_REGISTER(Curses);


UiFactoryCurses::UiFactoryCurses()
{
}

bool UiFactoryCurses::init()
{
    // TODO  cleanup
    m_tui = new clc::Tui;
    return m_browser.init(m_tui) && m_renderer.init(m_tui);
}

void UiFactoryCurses::deinit()
{
    delete m_tui;
}

const char* UiFactoryCurses::getName()
{
    return "ncurses";
}

Browse& UiFactoryCurses::getBrowser()
{
    return m_browser;
}

Renderer& UiFactoryCurses::getRenderer()
{
    return m_renderer;
}


