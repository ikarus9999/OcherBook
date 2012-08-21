#ifndef OCHER_UX_FACTORY_FB_MX50_H
#define OCHER_UX_FACTORY_FB_MX50_H

#include "ocher/ux/fb/FactoryFb.h"
#include "ocher/output/mx50/fb.h"


class UiFactoryFbMx50 : public UiFactoryFb
{
public:
    UiFactoryFbMx50();
    ~UiFactoryFbMx50();

    bool init();
    const char* getName();

protected:
    Mx50Fb m_fb;
};

#endif




