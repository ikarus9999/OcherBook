#ifndef OCHER_SETTINGS_H
#define OCHER_SETTINGS_H

// TODO:  conditionalize some settings based on platform?  so can maintain/
// share a single settings files


struct Settings {
    Settings();

    void load();
    void save();

    int trackReading;  ///< Track major reading events? (start/end book, ...)
    int encryptReading;  ///< Encrypt reading events?

    unsigned int minutesUntilSleep;
    int sleepShowBook;  ///< When sleeping, show latest book cover?

    unsigned int minutesUntilPowerOff;  ///< idle min until poweroff (lower bound by mintilsleep)

    clc::Buffer sleepHtml;
    clc::Buffer powerOffHtml;

    //time zone

    //language

    clc::Buffer wirelessSsid;
    int wirelessAirplaneMode;  ///< Only turn on wireless on-demand?
    
    unsigned int fullRefreshPages;

    int showPageNumbers;

    int fontPoints;
    // force font
    // force font size
    // line spacing
    int marginTop;
    int marginBottom;;
    int marginLeft;
    int marginRight;
    // justification

    // icons

    // filesystem point(s)
};

extern Settings settings;

#endif
