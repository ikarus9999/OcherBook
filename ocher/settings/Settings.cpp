#include "clc/storage/File.h"

#include "ocher/device/Filesystem.h"
#include "ocher/settings/Settings.h"

Settings settings;


Settings::Settings() :
    trackReading(0),
    encryptReading(1),
    minutesUntilSleep(15),
    sleepShowBook(1),
    minutesUntilPowerOff(60),
    wirelessAirplaneMode(0),
    fullRefreshPages(6),
    showPageNumbers(1),
    fontPoints(12),
    marginTop(10),
    marginBottom(10),
    marginLeft(10),
    marginRight(10)
{
}

void Settings::load()
{
    clc::File s;
    try {
        s.setTo(fs.getSettings());
    } catch(...) {
        return;
    }

    clc::Buffer line;
    while (!s.isEof()) {
        line = s.readLine(false, 1024);

        const char *p = line.c_str();
        size_t n = line.size();

        // TODO
    }
}

void Settings::save()
{
    clc::File s(fs.getSettings(), "w");

    clc::Buffer b;

    b.format("MinutesUntilSleep=%u\n", minutesUntilSleep);
    s.write(b);
    b.format("MinutesUntilPowerOff=%u\n", minutesUntilPowerOff);
    s.write(b);
    b.format("TrackReading=%u\n", trackReading ? 1 : 0);
    s.write(b);
}
