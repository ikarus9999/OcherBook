#ifndef OCHER_DEVICE_H
#define OCHER_DEVICE_H

#include "clc/data/Buffer.h"

#include "ocher/device/Filesystem.h"


/**
 * Represents the physical e-reader device.
 */
class Device
{
public:
    clc::Buffer getVersion();
    clc::Buffer getBuildDate();

    clc::Buffer getMac();
    clc::Buffer getIp();

    void reboot() {}

    Filesystem fs;
};

/**
 * The device singleton
 */
extern Device *device;

void initDevice();

#endif

