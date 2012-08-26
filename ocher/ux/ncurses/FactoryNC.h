#ifndef OCHER_UX_FACTORY_NC_H
#define OCHER_UX_FACTORY_NC_H

#include "ocher/ux/Factory.h"
#include "ocher/ux/ncurses/Browse.h"
#include "ocher/ux/ncurses/RenderCurses.h"

class UiFactoryCurses : public UiFactory
{
public:
    UiFactoryCurses();
    virtual ~UiFactoryCurses() {}

    bool init();
    void deinit();
    const char* getName();
    Browse& getBrowser();
    Renderer& getRenderer();

protected:
    BrowseCurses m_browser;
    RenderCurses m_renderer;
    clc::Tui* m_tui;
};

#endif

