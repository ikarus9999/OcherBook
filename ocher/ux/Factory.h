#ifndef OCHER_UX_FACTORY_H
#define OCHER_UX_FACTORY_H

#include "ocher/ux/Browse.h"
#include "ocher/ux/Renderer.h"

class UiFactory
{
public:
    /**
     * Called after user options have been parsed.
     * @return True iff sucessfully initialized, else output msg
     */
    virtual bool init() = 0;

    virtual void deinit() = 0;

    virtual const char* getName() = 0;

    virtual Browse& getBrowser() = 0;

    virtual Renderer& getRenderer() = 0;
};

#endif

