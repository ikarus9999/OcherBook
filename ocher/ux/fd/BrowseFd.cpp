#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#include "ocher/device/Filesystem.h"
#include "ocher/ux/fd/BrowseFd.h"
#include "ocher/ux/Renderer.h"
#include "ocher/settings/Options.h"

// TODO:  handle non-ttys

static char getKey()
{
    int key;
    struct termios oldTermios, newTermios;

    tcgetattr(0, &oldTermios);
    newTermios = oldTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    newTermios.c_cc[VTIME] = 0;
    newTermios.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &newTermios);

    key = getc(stdin);

    tcsetattr(0, TCSANOW, &oldTermios);

    return (char)key;
}

BrowseFd::BrowseFd()
{
}

bool BrowseFd::init()
{
    m_in = opt.inFd;
    m_out = opt.outFd;
    return true;
}

void BrowseFd::browse()
{
    //for (const char *d = fs.ocherLibraries[0]; d; ++d) {
    //    printf("%s\n", d);
    //}

}

void BrowseFd::read(Renderer& renderer)
{
    for (int pageNum = 0; ; ) {
        if (renderer.render(pageNum, true) < 0)
            return;

        char key = getKey();
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

