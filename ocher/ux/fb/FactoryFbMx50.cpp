#include "ocher/ocher.h"
#include "ocher/ux/fb/FactoryFbMx50.h"

UX_DRIVER_REGISTER(FbMx50);


UiFactoryFbMx50::UiFactoryFbMx50() :
    UiFactoryFb(&m_fb)
{
}

UiFactoryFbMx50::~UiFactoryFbMx50()
{
}

bool UiFactoryFbMx50::init()
{
    return m_fb.init() && m_render.init();
}

const char* UiFactoryFbMx50::getName()
{
    return "kobo-mx50";
}

