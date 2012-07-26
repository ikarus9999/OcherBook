#ifndef OCHER_APP_H
#define OCHER_APP_H

#include "clc/data/List.h"

#define UX_DRIVER_REGISTER(driver) \
    class Register##driver { \
    public: \
        Register##driver() { drivers.add(new UiFactory##driver); } \
    } reg##driver

/**
 * The list of all user experience drivers.  All compiled-in drivers
 * automatically register themselves here.
 */
extern clc::List drivers;

#endif
