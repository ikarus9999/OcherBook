#include "mxml.h"

#include "clc/storage/File.h"
#include "clc/support/Logger.h"

#include "ocher/ux/Factory.h"
#include "ocher/ux/Controller.h"
#include "ocher/settings/Options.h"

// TODO:  replace all this hardcoded stuff with factory:
#include "ocher/fmt/epub/Epub.h"
#include "ocher/fmt/epub/LayoutEpub.h"
#include "ocher/fmt/text/Text.h"
#include "ocher/fmt/text/LayoutText.h"


Controller::Controller(UiFactory *factory) :
    m_factory(factory)
{
}

void Controller::run()
{
    Browse& browser = m_factory->getBrowser();

    // TODO:  workflow

    browser.browse();

    clc::Buffer memLayout;

    // TODO:  complete hardcoded hack to test with here...
    // TODO:  probe file type
    // TODO:  rework Layout constructors to have separate init due to scoping

    Layout *layout;
    clc::File f(opt.file);
    char buf[2];
    if (f.read(buf, 2) != 2 || buf[0] != 'P' || buf[1] != 'K') {
        Text text(opt.file);
        layout = new LayoutText(&text);
        clc::Log::info("ocher", "Loading %s: %s", text.getFormatName().c_str(), opt.file);
        memLayout = layout->unlock();
    } else {
        Epub epub(opt.file);
        layout = new LayoutEpub(&epub);

        clc::Log::info("ocher", "Loading %s: %s", epub.getFormatName().c_str(), opt.file);

        clc::Buffer html;
        for (int i = 0; ; i++) {
            if (epub.getSpineItemByIndex(i, html) != 0)
                break;
            mxml_node_t *tree = epub.parseXml(html);
            if (tree) {
                ((LayoutEpub*)layout)->append(tree);
                mxmlDelete(tree);
            } else {
                clc::Log::warn("ocher", "No tree found for spine item %d", i);
            }
        }
        memLayout = layout->unlock();
    }

    Renderer& renderer = m_factory->getRenderer();
    renderer.set(memLayout);

    // Run through all pages without blitting to re-paginate
    // TODO:  speed stats
    // TODO:  faster? max_advance_width
    for (int pageNum = 0; ; pageNum++) {
        if (renderer.render(pageNum, false) != 0)
            break;
        clc::Log::info("ocher", "Paginated page %d", pageNum);
    }

    browser.read(renderer);

    delete layout;
}

