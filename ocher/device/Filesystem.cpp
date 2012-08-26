#include "clc/storage/Directory.h"
#include "clc/storage/File.h"
#include "clc/storage/Path.h"

#include "ocher/device/Filesystem.h"

#include <stdlib.h>
#ifdef _WIN32
#include <shlobj.h>
#endif
#if !defined(_WIN32) && !defined(__HAIKU__)
#include <pwd.h>
#endif


#if _WIN32

#ifndef DEFINE_KNOWN_FOLDER
#define DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID DECLSPEC_SELECTANY name = {l, w1, w2, {b1, b2, b3, b4, b5, b6, b7, b8}}
#endif
DEFINE_KNOWN_FOLDER(FOLDERID_LocalAppDataLow, 0xA520A1A4, 0x1780, 0x4FF6, 0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16);
DEFINE_KNOWN_FOLDER(FOLDERID_LocalAppData,    0xF1B32785, 0x6FBA, 0x4FCF, 0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91);
DEFINE_KNOWN_FOLDER(FOLDERID_UserProfiles,    0x0762D272, 0xC50A, 0x4BB0, 0xA3, 0x82, 0x69, 0x7D, 0xCD, 0x72, 0x9B, 0x80);
#define KF_FLAG_CREATE 0x00008000


static bool getKnownFolder(int id, clc::Buffer& d)
{
    TCHAR szPath[MAX_PATH];
    bool success = SHGetFolderPath(NULL, id, NULL, 0, szPath) == S_OK;
    if (success) {
        d = szPath;
        DEBUG_PRINT(("getKnownFolder: %s\n", d.c_str()));
    }
    return success;
}

static bool getKnownFolderVista(const GUID& id, clc::Buffer& d)
{
    bool success = false;
    typedef HRESULT (STDAPICALLTYPE * SHGetKnownFolderPathFN)(
        const GUID & rfid,
        DWORD dwFlags,
        HANDLE hToken,
        PWSTR *ppszPath);
    HMODULE lib = LoadLibrary("Shell32.dll");
    do {
        if (! lib)
            break;
        SHGetKnownFolderPathFN SHGetKnownFolderPath = (SHGetKnownFolderPathFN)GetProcAddress(lib, "SHGetKnownFolderPath");
        if (!SHGetKnownFolderPath)
            break;
        PWSTR pwszPath;
        if ((SHGetKnownFolderPath)(id, KF_FLAG_CREATE, NULL, &pwszPath) != S_OK)
            break;
        d = Utf::convertUTF16toUTF8((uint16_t*)pwszPath, wcslen(pwszPath)*2);
        DEBUG_PRINT(("getKnownFolderVista: %s\n", d.c_str()));
        success = true;
        CoTaskMemFree(pwszPath);
    } while(0);
    if (lib)
        FreeLibrary(lib);
    return success;
}
#endif

static clc::Buffer settingsDir()
{
    clc::Buffer dir;
#ifdef _WIN32
    if (! getKnownFolderVista(FOLDERID_LocalAppDataLow, dir) &&
        ! getKnownFolderVista(FOLDERID_LocalAppData, dir) &&
        ! getKnownFolder(CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, dir))
    {
        char* e;
        char* e2;
        if ((e = getenv("LOCALAPPDATA"))) {  // Vista
            dir = e;
            DEBUG_PRINT(("LOCALAPPDATA: %s\n", dir.c_str()));
        } else if ((e = getenv("HOMEDRIVE")) && (e2 = getenv("HOMEPATH"))) {  // 2000
            dir = e;
            clc::Path::join(dir, e2);
            clc::Path::join(dir, "Local Settings");
            clc::Path::join(dir, "Application Data");
            DEBUG_PRINT(("HOMEDRIVE: %s\n", dir.c_str()));
        } else if ((e = getenv("USERPROFILE"))) {  // may be roaming
            dir = e;
            clc::Path::join(dir, "Local Settings");
            clc::Path::join(dir, "Application Data");
            DEBUG_PRINT(("USERPROFILE: %s\n", dir.c_str()));
        } else if ((e = getenv("APPDATA"))) {  // may be roaming
            dir = e;
            DEBUG_PRINT(("APPDATA: %s\n", dir.c_str()));
        } else {
            dir = "c:";
        }
    }
#elif defined(__HAIKU__)
    dir = "/boot/home/config/settings";
#else
    const char *e = getenv("HOME");
    if (!e) {
        struct passwd *p = getpwuid(getuid());
        if (p) {
            e = p->pw_dir;
        } else {
            e = "/tmp";
        }
    }
    dir = e;
#if defined(__APPLE__)
    clc::Path::join(dir, "Library");
    clc::Path::join(dir, "Application Support");
#endif
#endif
    return dir;
}

Filesystem fs;

Filesystem::Filesystem()
{
#ifdef TARGET_KOBO
    m_home = "/mnt/onboard/.ocher";
    m_settings = "/mnt/onboard/.ocher/settings";
#else
    clc::Buffer s = settingsDir();
    m_home = strdup(s.c_str());
#if defined(_WIN32) || defined(__HAIKU__) || defined(__APPLE__)
    clc::Path::join(s, "OcherBook");
#else
    clc::Path::join(s, ".OcherBook");
#endif
    ::mkdir(s.c_str(), 0775);
    clc::Path::join(s, "settings");
    m_settings = strdup(s.c_str());
#endif
}

Filesystem::~Filesystem()
{
#ifdef TARGET_KOBO
#elif defined(_WIN32)
#elif defined(__HAIKU__)
#else
    free(m_home);
    free(m_settings);
#endif
}


#if 0
    static const char *d[] = {
        "/mnt/sd",
        "/mnt/onboard",
        0
    };
    ocherLibraries = d;

void Filesystem::mkdirs()
{
    ::mkdir(ocherHome, 0775);
}
#endif

