#include "clc/tui/Tui.h"
#include "ocher/ui/ncurses/Browse.h"


BrowseCurses::BrowseCurses(clc::Tui &tui) :
    clc::Window(tui.mainWindow())
{
}

void BrowseCurses::repaint()
{

    refresh();
}



