#include "mxml.h"

#include "clc/support/Logger.h"

#include "ocher/ui/Browse.h"
#include "ocher/ui/Controller.h"
#include "ocher/settings/Options.h"

// TODO:  replace all this hardcoded stuff with factory:
#include "ocher/fmt/epub/Epub.h"
#include "ocher/fmt/epub/LayoutEpub.h"
#include "ocher/ui/fd/RenderFd.h"


Controller::Controller(UiFactory *factory) :
    m_factory(factory)
{
}

void Controller::run()
{
    Browse& browser = m_factory->getBrowser();

    // TODO:  workflow

    browser.browse();

    // TODO:  probe file type
    Epub epub(opt.file);
    LayoutEpub layout(&epub);

    clc::Log::info("ocher", "Loading %s: %s", epub.getFormatName().c_str(), opt.file);

    clc::Buffer html;
    for (int i = 0; ; i++) {
        if (epub.getSpineItemByIndex(i, html) != 0)
            break;
        mxml_node_t *tree = epub.parseXml(html);
        if (tree) {
            layout.append(tree);
            mxmlDelete(tree);
        }
    }

    RendererFd r(layout.unlock(), 1);
    r.render(0, 1);


}

