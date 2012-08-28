#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define uint unsigned int
#include <linux/mxcfb.h>

#include "clc/support/Logger.h"

#include "ocher/output/mx50/fb.h"


// Kobo Touch: MX508
// http://mediaz.googlecode.com/svn-history/r19/trunk/ReaderZ/native/einkfb/einkfb.c

Mx50Fb::Mx50Fb() :
    m_fd(-1),
    m_fb(0),
    m_fbSize(0),
    m_marker(-1),
    m_clears(999)
{
}

bool Mx50Fb::init()
{
    const char *dev = "/dev/fb0";
    m_fd = open(dev, O_RDWR);
    if (m_fd == -1) {
        clc::Log::error("ocher.mx50", "Failed to open %s: %s", dev, strerror(errno));
        goto fail;
    }

    // Get fixed screen information
    if (ioctl(m_fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        clc::Log::error("ocher.mx50", "Failed to get fixed screen info: %s", strerror(errno));
        goto fail1;
    }

    // Get variable screen information
    if (ioctl(m_fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        clc::Log::error("ocher.mx50", "Failed to get variable screen info: %s", strerror(errno));
        goto fail1;
    }

    // Configure for what we actually want
    vinfo.bits_per_pixel = 8;
    vinfo.grayscale = 1;
    // 0 is landscape right handed, 3 is portrait
    vinfo.rotate = 3;
    if (ioctl(m_fd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        clc::Log::error("ocher.mx50", "Failed to set variable screen info: %s", strerror(errno));
        goto fail1;
    }

    clc::Log::info("ocher.mx50", "virtual %dx%d", vinfo.xres_virtual, vinfo.yres_virtual);
    // Figure out the size of the screen in bytes
    m_fbSize = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    m_fb = (char*)mmap(0, m_fbSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (m_fb == MAP_FAILED) {
        clc::Log::error("ocher.mx50", "mmap: %s", strerror(errno));
        goto fail1;
    }

    clear();
    return true;

fail1:
    close(m_fd);
    m_fd = -1;
fail:
    return false;
}

unsigned int Mx50Fb::height()
{
    return 800;
}

unsigned int Mx50Fb::width()
{
    return 600;
}

Mx50Fb::~Mx50Fb()
{
    if (m_fd != -1) {
        close(m_fd);
        m_fd = -1;
    }
}

void Mx50Fb::clear()
{
    ++m_clears;
    memset(m_fb, 0xff, m_fbSize);
}

static void invcpy(unsigned char *dst, unsigned char *src, size_t n)
{
#if 0
    // Assuming src is word aligned.
    // dst may not be aligned.

#else
    // TODO  invert then memcpy is total hack.  do it right:
    // respect alignment, invert while copying.  (Or is it possible
    // to have FreeType output inverted?)
    for (size_t i = 0; i < n; ++i) {
        src[i] = ~src[i];
    }
    memcpy(dst, src, n);
#endif
}

void Mx50Fb::blit(unsigned char *p, int x, int y, int w, int h)
{
    for (int i = 0; i < h; ++i) {
        invcpy(((unsigned char*)m_fb) + y*vinfo.xres_virtual + x, p, w);
        y++;
        p = p + w;
    }
}
#if 0
#if 0
    char buf[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    for(int i = 0; i < 1000; ++i)
        write(m_fd, buf, 16);
#else
    // TODO
#endif

    // 0 0 320 258 101 1 1 1000 0 0 0 0 0 0 0 0 0 
    struct mxcfb_update_data u;
    printf("%u\n", sizeof(u));
    memset(&u, 0, sizeof(u));
    u.update_region.top = x;
    u.update_region.left = y;
    u.update_region.width = w;
    u.update_region.height = h;
    u.waveform_mode = 257;
    u.update_mode = UPDATE_MODE_FULL;
    u.update_marker = 1;
//    u.update_mode = UPDATE_MODE_PARTIAL;
    u.temp = 23; //0x1000;
//    u.flags = EPDC_FLAG_ENABLE_INVERSION;
#if 0
    char *buf = (char*) malloc(800*600);
    memset(buf, 0, 800*600);
    for (int x = 0; x < 600; ++x)
        *(buf+(x*600)+x) = 1;
    u.flags = EPDC_FLAG_USE_ALT_BUFFER;
    u.alt_buffer_data.virt_addr = buf;
//    u.alt_buffer_data.phys_addr = (uint32_t)buf;  //DDD
    u.alt_buffer_data.width = 800;
    u.alt_buffer_data.height = 600;
    u.alt_buffer_data.alt_update_region.top = 0;
    u.alt_buffer_data.alt_update_region.left = 0;
    u.alt_buffer_data.alt_update_region.width = 800;
    u.alt_buffer_data.alt_update_region.height = 600;
#endif
    int retval = ioctl(m_fd, MXCFB_SEND_UPDATE, &u);
    if (retval)
        perror("ioctl MXCFB_SEND_UPDATE");
#endif

int Mx50Fb::update(int x, int y, int w, int h, bool full)
{
    // TODO
    if (m_clears > 5) {
        m_clears = 0;
        full = true;
    } else {
        full = false;
    }

    struct mxcfb_update_data region;

    region.update_region.left = x;
    region.update_region.top = y;
    region.update_region.width = w;
    region.update_region.height = h;
    region.waveform_mode = WAVEFORM_MODE_AUTO;
    region.update_mode = full ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;
    region.update_marker = ++m_marker;
    region.temp = TEMP_USE_AMBIENT;
    region.flags = 0;

    if (ioctl(m_fd, MXCFB_SEND_UPDATE, &region) == -1) {
        clc::Log::error("ocher.mx50", "MXCFB_SEND_UPDATE(%d, %d, %d, %d, %d): %s",
                x, y, w, h, m_marker, strerror(errno));
    }
    return m_marker;
}

void Mx50Fb::waitUpdate(int marker)
{
    if (ioctl(m_fd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &marker) == -1) {
        clc::Log::error("ocher.mx50", "MXCFB_WAIT_FOR_UPDATE_COMPLETE(%d): %s", marker, strerror(errno));
    }
}

void Mx50Fb::setPixelFormat()
{
    fb_var_screeninfo screen_info;
    screen_info.bits_per_pixel = 8;
    screen_info.grayscale = GRAYSCALE_8BIT;
    int retval = ioctl(m_fd, FBIOPUT_VSCREENINFO, &screen_info);
    if (retval)
        clc::Log::error("ocher.mx50", "FBIOPUT_VSCREENINFO: %s", strerror(errno));
}

void Mx50Fb::setAutoUpdateMode(bool autoUpdate)
{
    uint32_t mode;
    mode = autoUpdate ? AUTO_UPDATE_MODE_AUTOMATIC_MODE : AUTO_UPDATE_MODE_REGION_MODE;
    int retval = ioctl(m_fd, MXCFB_SET_AUTO_UPDATE_MODE, &mode);
    if (retval)
        clc::Log::error("ocher.mx50", "MXCFB_SET_AUTO_UPDATE_MODE(%d): %s", mode, strerror(errno));
}

void Mx50Fb::setUpdateScheme()
{
    fb_var_screeninfo screen_info;
//    screen_info.scheme = UPDATE_SCHEME_SNAPSHOT;
    int retval = ioctl(m_fd, MXCFB_SET_UPDATE_SCHEME, &screen_info);
    if (retval)
        clc::Log::error("ocher.mx50", "MXCFB_SET__UPDATE_SCHEME: %s", strerror(errno));
}

