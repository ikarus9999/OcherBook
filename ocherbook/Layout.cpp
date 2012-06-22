#include "mxml.h"

#include "Layout.h"
#include "Epub.h"

#ifdef TARGET_KOBOTOUCH
#include "fb/mx50/fb.h"
Mx50Epdc e;
#endif

class Attrs {
public:
    Attrs() : ul(0), b(0), em(0), pre(0), ws(0), nl(0), pts(12) {}
    int ul;
    int b;
    int em;
    int pre;
    int ws;
    int nl;
    int pts;   ///< text points
    // TODO text size, ...
};

Attrs a[10];
int ai = 1;

void pushAttrs(mxml_node_t *node)
{
    a[ai+1] = a[ai];
    ai++;
}

void enableUl()
{
    printf("\x1b[4m");
}

void disableUl()
{
    printf("\x1b[24m");
}

void enableEm()
{
    printf("\x1b[1m");
}

void disableEm()
{
    printf("\x1b[22m");
}

void applyAttrs(int i)
{
    if (a[ai].ul && !a[ai-i].ul) {
        enableUl();
    } else if (!a[ai].ul && a[ai-i].ul) {
        disableUl();
    }

    if (a[ai].em && !a[ai-i].em) {
        enableEm();
    } else if (!a[ai].em && a[ai-i].em) {
        disableEm();
    }
}

void popAttrs()
{
    ai--;
    applyAttrs(-1);
}

void outputChar(char c)
{
    a[ai].nl = 0;
    if (isspace(c)) {
        if (! a[ai].ws) {
            a[ai].ws = 1;
            putc(' ', stdout);
        }
    } else {
        a[ai].ws = 0;
        putc(c, stdout);
    }
}

void outputNode(mxml_node_t *node)
{
    if (node->type == MXML_OPAQUE) {
        //printf("%s ", node->value.opaque);
        for (char *p = node->value.opaque; *p; ++p) {
            outputChar(*p);
        }
    }
}

void outputNl()
{
    if (! a[ai].nl) {
        a[ai].nl = 1;
        putc('\n', stdout);
    }
}

void outputBr()
{
    putc('\n', stdout);
}

void processSiblings(mxml_node_t *node, Epub *epub);

//typedef void (*PreHtmlFunc)(const char *name, mxml_node_t *node);
//typedef void (*PostHtmlFunc)(mxml_node_t *node);
//
//
//struct HtmlTag {
//    const char *name;
//    PreHtmlFunc pre;
//    PostHtmlFunc post;
//};

void processNode(mxml_node_t *node, Epub *epub)
{
    if (node->type == MXML_ELEMENT) {
        const char *name = node->value.element.name;
//        printf(">>%s<<", name);
        if (strcasecmp(name, "div") == 0) {
            pushAttrs(node);
            processSiblings(node->child, epub);
            popAttrs();
            outputChar(' '); // TODO: temp until CSS
        } else if (strcasecmp(name, "title") == 0) {

        } else if (strcasecmp(name, "link") == 0) {
            // load CSS
            const char *type = mxmlElementGetAttr(node, "type");
            if (type && strcmp(type, "text/css") == 0) {
                const char *href = mxmlElementGetAttr(node, "hrev");
                if (href) {
                    clc::Buffer css;
                    TreeFile *f = epub->getFile(href, css);
                    // TODO: parse CSS

                }
            }
        } else if (strcasecmp(name, "p") == 0) {
            pushAttrs(node);
            outputNl();
            applyAttrs(1);
            processSiblings(node->child, epub);
            popAttrs();
            outputBr();
            outputBr();
        } else if (strcasecmp(name, "br") == 0) {
            outputBr();
        } else if ((name[0] == 'h' || name[0] == 'H') && isdigit(name[1]) && !name[2]) {
            pushAttrs(node);
            // TODO text size, ...
            outputNl();
            applyAttrs(1);
            processSiblings(node->child, epub);
            popAttrs();
            outputBr();
        } else if (strcasecmp(name, "b") == 0) {
            pushAttrs(node);
            a[ai].b = 1;
            applyAttrs(1);
            processSiblings(node->child, epub);
            popAttrs();
        } else if (strcasecmp(name, "ul") == 0) {
            pushAttrs(node);
            a[ai].ul = 1;
            applyAttrs(1);
            processSiblings(node->child, epub);
            popAttrs();
        } else if (strcasecmp(name, "em") == 0) {
            pushAttrs(node);
            a[ai].em = 1;
            applyAttrs(1);
            processSiblings(node->child, epub);
            popAttrs();
        } else {
            processSiblings(node->child, epub);
        }

    } else if (node->type == MXML_OPAQUE) {
        outputNode(node);
    }
}

void processSiblings(mxml_node_t *node, Epub *epub)
{
    for ( ; node; node = mxmlGetNextSibling(node)) {
        processNode(node, epub);
    }
}


int render(clc::Buffer &html, Epub *epub)
{
    //printf("%s\n", html.c_str());
    mxml_node_t *tree = mxmlLoadString(NULL, html.c_str(), MXML_OPAQUE_CALLBACK);
    if (!tree) {
        return -1;
    }

    mxml_node_t *body = mxmlFindPath(tree, "html/body");
    if (body) {
        // TODO:  huh? mxmlFindPath seems to return the child.  Ok, so processSiblings.
        processSiblings(body, epub);
    }

    mxmlDelete(tree);
    return 0;
}


