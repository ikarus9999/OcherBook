#include "clc/tui/Tui.h"

#include "ocher/ux/Renderer.h"
#include "ocher/ux/ncurses/Browse.h"


BrowseCurses::BrowseCurses()
{
}

bool BrowseCurses::init(clc::Tui* tui)
{
    m_tui = tui;
    return true;
}

void BrowseCurses::browse()
{
    //for (const char *d = fs.ocherLibraries[0]; d; ++d) {
    //    printf("%s\n", d);
    //}
}

void BrowseCurses::read(Renderer& renderer)
{
    for (int pageNum = 0; ; ) {
        if (renderer.render(pageNum, true) < 0)
            return;

        clc::Keystroke::Modifiers m;
        clc::Keystroke key = clc::Tui::getKey(&m);
        if (key == 'p' || key == 'b') {
            if (pageNum > 0)
                pageNum--;
        } else if (key == 'q') {
            break;
        } else {
            pageNum++;
        }
    }
}

