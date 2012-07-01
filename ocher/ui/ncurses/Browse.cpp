#include "clc/tui/Tui.h"
#include "ui/curses/Browse.h"

BrowseCurses::BrowseCurses(clc::Tui &tui) :
    clc::Window(tui.mainWindow())
{
}

void BrowseCurses::repaint()
{

    refresh();
}



