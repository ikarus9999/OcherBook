#include "ocher/device/Filesystem.h"
#include "ocher/ux/Renderer.h"
#include "ocher/ux/fb/BrowseFb.h"
#include "ocher/settings/Options.h"


BrowseFb::BrowseFb()
{
}

bool BrowseFb::init()
{
    return true;
}

void BrowseFb::browse()
{
    //for (const char *d = fs.ocherLibraries[0]; d; ++d) {
    //    printf("%s\n", d);
    //}

}

void BrowseFb::read(Renderer& renderer)
{
    for (int pageNum = 0; ; ) {
        if (renderer.render(pageNum, true) < 0)
            break;

        break;  //DDD
    }
}

