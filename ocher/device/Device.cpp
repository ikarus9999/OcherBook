#include "ocher/device/Version.h"
#include "ocher/device/Device.h"

clc::Buffer Device::getVersion()
{
    clc::Buffer version;
    version.format("%d.%d.%d", OCHER_MAJOR, OCHER_MINOR, OCHER_PATCH);
    return version;
}

clc::Buffer Device::getBuildDate()
{
    clc::Buffer date(__DATE__);
    return date;
}

