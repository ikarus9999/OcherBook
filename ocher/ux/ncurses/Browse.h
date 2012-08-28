#ifndef OCHER_BROWSE_CURSES_H
#define OCHER_BROWSE_CURSES_H

#include "clc/data/Buffer.h"
#include "clc/tui/Tui.h"

#include "ocher/ux/Browse.h"


class BrowseCurses : public Browse
{
public:
    BrowseCurses();
    ~BrowseCurses() {}

    bool init(clc::Tui* tui);
    void browse();
    void read(Renderer& renderer);

protected:
    clc::Tui* m_tui;
};


#endif


