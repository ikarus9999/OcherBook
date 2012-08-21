#ifndef OCHER_SETTINGS_H
#define OCHER_SETTINGS_H

struct Settings {
    Settings();

    void load();
    void save();

    int trackReading;

    unsigned int minutesUntilSleep;
    unsigned int minutesUntilPoweroff;

    // Libraries:
    //   filesystem point(s)

    // sleep screen text/icon
    // power-off screen text/icon

};

};

#endif
