#include "mxml.h"

#include "clc/support/Logger.h"

#include "ocher/fmt/epub/Epub.h"
#include "ocher/fmt/epub/LayoutEpub.h"
#include "ocher/fmt/epub/TreeMem.h"


// TODO:  meta should be attached to the bytecode
// TODO:  css
// TODO:  canonicalize:  HTML escapes, ...


void LayoutEpub::processNode(mxml_node_t *node)
{
    if (node->type == MXML_ELEMENT) {
        const char *name = node->value.element.name;
        clc::Log::trace("ocher.fmt.epub.layout", "found element '%s'", name);
        if (strcasecmp(name, "div") == 0) {
            processSiblings(node->child);
        } else if (strcasecmp(name, "title") == 0) {

        } else if (strcasecmp(name, "link") == 0) {
            // load CSS
            const char *type = mxmlElementGetAttr(node, "type");
            if (type && strcmp(type, "text/css") == 0) {
                const char *href = mxmlElementGetAttr(node, "href");
                if (href) {
                    clc::Buffer css;
                    css = m_epub->getFile(href);
                    // TODO: parse CSS
                }
            }
        } else if (strcasecmp(name, "p") == 0) {
            outputNl();
            processSiblings(node->child);
            outputNl();
            outputBr();
        } else if (strcasecmp(name, "br") == 0) {
            outputBr();
        } else if ((name[0] == 'h' || name[0] == 'H') && isdigit(name[1]) && !name[2]) {
            // TODO CSS: text size, ...
            outputNl();
            pushTextAttr(AttrBold, 0);
            pushTextAttr(AttrSizeAbs, 12+(9-name[1]-'0')*2);
            processSiblings(node->child);
            popTextAttr(2);
            outputNl();
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
    // TODO:  "html/body" matches nothing if the root node is "html" (no ?xml) so using "*/body"
    mxml_node_t *body = mxmlFindPath(tree, "*/body");
    if (body) {
        // mxmlFindPath returns the first child node.  Ok, so processSiblings.
        processSiblings(body);
    } else {
        clc::Log::warn("ocher.fmt.epub.layout", "no body");
    }
}


