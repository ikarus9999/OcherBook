#ifndef OCHER_DEVICE_H
#define OCHER_DEVICE_H

#include "clc/data/Buffer.h"

class Device
{
public:
    virtual clc::Buffer getMac() = 0;
    virtual clc::Buffer getIp() = 0;

    virtual void reboot() {}

    virtual clc::Buffer getVersion();
    virtual clc::Buffer getBuildDate();
};

#endif

