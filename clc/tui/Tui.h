#ifndef LIBCLC_TUI_H
#define LIBCLC_TUI_H

/**
 * @file
 * C++ bindings for Curses.
 * One design goal is to minimize namespace pollution due to all of curses' C macros.
 */

typedef struct _win_st WINDOW;
typedef struct fieldnode FIELD;

namespace clc
{

/**
*/
class Keystroke
{
public:
    enum Mod {
        M_Shift,
        M_Ctrl,
        M_Alt,
    };

    struct Modifiers {
        unsigned int m;
        int x;
        int y;
    };

    enum K {
        K_Up = 256,
        K_Tab,
        K_BackTab,
        K_Home,
        K_End,
        K_Down,
        K_Left,
        K_Right,
        K_Backspace,
        K_Enter,
        K_Cancel,
        K_ScrollForward,
        K_ScrollBackward,
        K_Escape,
        K_F1,
        K_F2,
        K_F3,
        K_Unknown,
        K_MouseB1_Down,
        K_MouseB1_Up,
        K_MouseB1_Clicked,
        K_MouseB1_DoubleClicked,
        K_MouseB2_Down,
        K_MouseB2_Up,
        K_MouseB2_Clicked,
        K_MouseB2_DoubleClicked,
    };

    enum K v;

    bool operator==(char c) const { return (unsigned int)v == (unsigned int)c; }
    bool operator!=(char c) const { return (unsigned int)v != (unsigned int)c; }
};

/**
*/
class Window
{
public:
    Window(WINDOW* w);
    Window(int x, int y, int dx, int dy);

    virtual ~Window();

    int x;
    int y;
    int attr;

    void refresh();
    void getMaxXY(int& x, int& y);
    void gotoXY(int x, int y);

    void printw(const char* fmt, ...);
    void changeAttr(int a);
    void chAttr(int x, int y);
    void clearToEol();

    /**
     * Waits until a keystroke or mouse click occurs.
     */
    Keystroke getKey(Keystroke::Modifiers *mod = 0);

    virtual void repaint() {}

protected:
    WINDOW* w;
    bool m_delWin;
};

/**
 * A Field is an element within a Form.
 */
class Field
{
public:
    Field(int x, int y, int dx, int dy);
    ~Field();

protected:
    FIELD *m_field;
};

#if 0
class Form
{
public:
    Form();
    ~Form();

protected:

};
#endif

/**
*/
class Tui
{
public:
    Tui();
    ~Tui();

    /**
     * Shows or hides the cursor.  By default, it is shown.
     */
    void cursor(bool cursorOn);

    WINDOW* mainWindow();
};

}

#endif

