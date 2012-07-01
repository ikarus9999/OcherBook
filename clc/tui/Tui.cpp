#include "clc/config.h"

#include <stdarg.h>
#include <form.h>
#if defined(HAVE_NCURSES_H)
#include <ncurses.h>
#elif defined(HAVE_NCURSES_NCURSES_H)
#include <ncurses/ncurses.h>
#endif

#include "clc/tui/Tui.h"

namespace clc
{

Tui::Tui()
{
    ::initscr();
    ::cbreak();
    ::raw();
    ::noecho();
    ::keypad(stdscr, TRUE);
    start_color();
    cursor(true);

    mousemask(ALL_MOUSE_EVENTS, NULL);

    // TODO
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_YELLOW, COLOR_RED);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLUE);
}

Tui::~Tui()
{
    ::endwin();
}

void Tui::cursor(bool cursorOn)
{
    curs_set(cursorOn ? 1 : 0);
}

WINDOW* Tui::mainWindow()
{
    return stdscr;
}

Window::Window(int x, int y, int dx, int dy) :
    attr(0),
    w(newwin(dy, dx, y, x)),
    m_delWin(true)
{
    // TODO:delwin
}

Window::Window(WINDOW* w) :
    attr(0),
    w(w),
    m_delWin(false)
{
    wclear(w);
}

Window::~Window()
{
    if (m_delWin)
        delwin(w);
}

void Window::printw(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vw_printw(w, fmt, ap);
    va_end(ap);
}

void Window::changeAttr(int a)
{
    attr = a;
    (void)wattrset(w, COLOR_PAIR(attr));
}

void Window::clearToEol()
{
    wclrtoeol(w);
}

void Window::chAttr(int x, int y)
{
    // TODO
    (void) mvwchgat(w, y, x, -1, COLOR_PAIR(attr), 1, NULL);
}

void Window::getMaxXY(int& x, int& y)
{
    getmaxyx(w, y, x);
}

void Window::gotoXY(int x, int y)
{
    wmove(w, y, x);
}

void Window::refresh()
{
    ::wrefresh(w);
}

Keystroke Window::getKey(Keystroke::Modifiers* mod)
{
    if (mod) {
        mod->m = 0;
        mod->x = 0;
        mod->y = 0;
    }

    int c = getch();
    Keystroke k;
    if (c >= 32 && c < 128) {
        k.v = (Keystroke::K)c;
        return k;
    }
    switch(c) {
        case 27:
            k.v = Keystroke::K_Escape;
            break;
        case KEY_SF:
        case 'E'-'A'+1:
            k.v = Keystroke::K_ScrollForward;
            break;
        case KEY_SR:
        case 'Y'-'A'+1:
            k.v = Keystroke::K_ScrollBackward;
            break;
        case 9:
            k.v = Keystroke::K_Tab;
            break;
        case KEY_BTAB:
            k.v = Keystroke::K_BackTab;
            break;
        case KEY_HOME:
            k.v = Keystroke::K_Home;
            break;
        case KEY_END:
            k.v = Keystroke::K_End;
            break;
        case KEY_DOWN:
            k.v = Keystroke::K_Down;
            break;
        case KEY_UP:
            k.v = Keystroke::K_Up;
            break;
        case KEY_LEFT:
            k.v = Keystroke::K_Left;
            break;
        case KEY_RIGHT:
            k.v = Keystroke::K_Right;
            break;
        case KEY_ENTER:
        case 0xa:
            k.v = Keystroke::K_Enter;
            break;
        case KEY_CANCEL:
            k.v = Keystroke::K_Cancel;
            break;
        case KEY_MOUSE:
            {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    // TODO:
                    k.v = Keystroke::K_MouseB1_Clicked;
                    if (mod) {
                        mod->x = event.x;
                        mod->y = event.y;
                    }
                    break;
                }
            }
        default:
            k.v = Keystroke::K_Unknown;
            break;
    }

    return k;
}

Field::Field(int x, int y, int dx, int dy)
{
    m_field = new_field(y, x, dy, dx, 0, 0);
    set_field_back(m_field, COLOR_PAIR(4));
    field_opts_off(m_field, O_AUTOSKIP | O_BLANK);
}

Field::~Field()
{
    free_field(m_field);
}

#if 0
Label::Label(Buffer& text) :
    m_text(text)
{
}

Form::Form()
{
}

void Form::addFields()
{
}
#endif

}
//addch(ACS_CKBOARD);

