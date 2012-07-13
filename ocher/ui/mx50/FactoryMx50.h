#ifndef OCHER_UI_FACTORY_MX50_H
#define OCHER_UI_FACTORY_MX50_H

#include "ocher/ui/Factory.h"

class UiFactoryMx50 : public UiFactory
{
public:
    UiFactoryMx50();

    bool init();
    const char* getName();
    Browse& getBrowser();

protected:
};

#endif



