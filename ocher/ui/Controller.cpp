#include "mxml.h"

#include "ocher/fmt/Format.h"
#include "ocher/fmt/epub/Epub.h"
#include "ocher/layout/Layout.h"
#include "ocher/settings/Options.h"
#include "ocher/ui/Browse.h"
#include "ocher/ui/Controller.h"


Controller::Controller(UiFactory *factory) :
    m_factory(factory)
{
}

void Controller::run()
{
    // TODO:  workflow

    // TODO:  probe file type

 
    Epub epub(opt.file);

    clc::Buffer html;
    for (int i = 0; ; i++) {
        if (epub.getSpineItemByIndex(i, html) != 0)
            break;
        mxml_node_t *tree = epub.parseXml(html);
        if (tree) {
            render(tree, &epub);
            mxmlDelete(tree);
        }
    }
    
}

