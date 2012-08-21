#include "clc/storage/Path.h"
#include "clc/storage/File.h"

#include "ocher/device/Filesystem.h"

Filesystem::Filesystem()
{
}

void Filesystem::mkdirs()
{
#ifdef TARGET_KOBO
    // Kobo uses ext4 on the root filesystem (even keeping .ash_history on
    // /), so apparently they feel okay about writing to it a lot, even
    // without a wear leveling filesystem.  Strange.
    // TODO:  what flash device does it use?
    // But will mirror what they are doing, on /mnt/onboard/.
    clc::File::mkdir("/mnt");
    clc::File::mkdir("/mnt/onboard");
    clc::File::mkdir("/mnt/onboard/.ocher");

#elif defined(__linux__)

#endif
}

