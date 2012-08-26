#ifndef OCHER_FILESYSTEM_H
#define OCHER_FILESYSTEM_H


class Filesystem
{
public:
    Filesystem();
    ~Filesystem();

    inline const char* getHome() { return m_home; }
    inline const char* getSettings() { return m_settings; }

    const char **ocherLibraries;

protected:
    void mkdirs();
    char* m_home;
    char* m_settings;
};

extern struct Filesystem fs;

#endif
