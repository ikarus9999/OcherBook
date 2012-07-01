#ifndef OCHER_UI_FACTORY_H
#define OCHER_UI_FACTORY_H

#include "ocher/ui/Browse.h"

class UiFactory
{
public:
    virtual bool init() = 0;
    virtual const char* getName() = 0;
    virtual Browse& getBrowser() = 0;
};

#endif

