#ifndef OCHER_DEVICE_H
#define OCHER_DEVICE_H

#include "clc/data/Buffer.h"
#include "clc/os/Thread.h"

#include "ocher/device/Filesystem.h"


#ifdef OCHER_TARGET_KOBO
class KoboDeviceThread : public clc::Thread
{
public:
    KoboDeviceThread();
    ~KoboDeviceThread();
protected:
    void run();
    int m_buttonFd;
    int m_touchFd;
    int m_pipe[2];
};
#endif

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

#ifdef OCHER_TARGET_KOBO
    KoboDeviceThread m_devThread;
#endif
};

/**
 * The device singleton
 */
extern Device *device;

void initDevice();

#endif

