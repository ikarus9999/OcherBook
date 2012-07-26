#ifndef OCHER_CONTROLLER_H
#define OCHER_CONTROLLER_H

#include "ocher/ux/Factory.h"

class Controller
{
public:
    Controller(UiFactory *factory);

    void run();

protected:
    void open();

    UiFactory *m_factory;
};

#endif

