#include "clc/storage/File.h"

#include "ocher/device/Filesystem.h"
#include "ocher/settings/Settings.h"


void initSettings()
{
}

Settings::Settings() :
    minutesUntilSleep(15)
    minutesUntilSleep(60)
{
}

void Settings::load()
{
    // TODO: handle exceptions

    clc::Buffer settings = Filesystem::getOcherRoot();
    clc::Path::join(settings, "settings");
    clc::File s(settings);
    clc::Buffer line;

    while (!s.isEof()) {
        line = s.readLine(false, 1024);

        const char *p = line.c_str();
        size_t n = line.size();

    }
}

void Settings::save()
{
    clc::Buffer settings = Filesystem::getOcherRoot();
    clc::Path::join(settings, "settings");
    clc::File s(settings, "w");
}
