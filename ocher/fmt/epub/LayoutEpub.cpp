#include "mxml.h"

#include "clc/support/Logger.h"

#include "ocher/fmt/epub/Epub.h"
#include "ocher/fmt/epub/LayoutEpub.h"
#include "ocher/fmt/epub/TreeMem.h"


// TODO:  meta should be attached to the bytecode
// TODO:  css
// TODO:  canonicalize:  whitespace, HTML escapes, ...


void LayoutEpub::processNode(mxml_node_t *node)
{
    if (node->type == MXML_ELEMENT) {
        const char *name = node->value.element.name;
        clc::Log::trace("ocher.fmt.epub.layout", "found element '%s'", name);
        if (strcasecmp(name, "div") == 0) {
//            pushAttrs(node);
            processSiblings(node->child);
//            popTextAttr();
//            outputChar(' '); // TODO: temp until CSS
        } else if (strcasecmp(name, "title") == 0) {

        } else if (strcasecmp(name, "link") == 0) {
            // load CSS
            const char *type = mxmlElementGetAttr(node, "type");
            if (type && strcmp(type, "text/css") == 0) {
                const char *href = mxmlElementGetAttr(node, "hrev");
                if (href) {
                    clc::Buffer css;
                    TreeFile *f = m_epub->getFile(href, css);
                    // TODO: parse CSS

                }
            }
        } else if (strcasecmp(name, "p") == 0) {
//            pushAttrs(node);
            outputNl();
//            applyAttrs(1);
            processSiblings(node->child);
//            popAttrs();
            outputBr();
            outputBr();
        } else if (strcasecmp(name, "br") == 0) {
            outputBr();
        } else if ((name[0] == 'h' || name[0] == 'H') && isdigit(name[1]) && !name[2]) {
            // TODO CSS: text size, ...
            //TODO: outputNl();
            pushTextAttr(AttrBold, 0);
            pushTextAttr(AttrSizeAbs, 12+(9-name[1]-'0')*2);
            processSiblings(node->child);
            popTextAttr(2);
            //TODO: outputBr();
        } else if (strcasecmp(name, "b") == 0) {
            pushTextAttr(AttrBold, 0);
            processSiblings(node->child);
            popTextAttr();
        } else if (strcasecmp(name, "ul") == 0) {
            pushTextAttr(AttrUnderline, 0);
            processSiblings(node->child);
            popTextAttr();
        } else if (strcasecmp(name, "em") == 0) {
            pushTextAttr(AttrItalics, 0);
            processSiblings(node->child);
            popTextAttr();
        } else {
            processSiblings(node->child);
        }
    } else if (node->type == MXML_OPAQUE) {
        clc::Log::trace("ocher.fmt.epub.layout", "found opaque");
        for (char *p = node->value.opaque; *p; ++p) {
            outputChar(*p);
        }
        flushText();
    }
}


void LayoutEpub::processSiblings(mxml_node_t *node)
{
    for ( ; node; node = mxmlGetNextSibling(node)) {
        processNode(node);
    }
}

void LayoutEpub::append(mxml_node_t *tree)
{
    mxml_node_t *body = mxmlFindPath(tree, "html/body");
    if (body) {
        // TODO:  huh? mxmlFindPath seems to return the child.  Ok, so processSiblings.
        processSiblings(body);
    }
}


