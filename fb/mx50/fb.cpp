#include <sys/ioctl.h>
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

#include "mx50/fb.h"


Mx50Epdc::Mx50Epdc()
{
    fd = open("/dev/fb0", O_RDWR);
//    setAutoUpdateMode(false);
    setPixelFormat();
#if 1
    char buf[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
    for(int i = 0; i < 1000; ++i)
        write(fd, buf, 16);
#endif

    // 0 0 320 258 101 1 1 1000 0 0 0 0 0 0 0 0 0 
    struct mxcfb_update_data u;
    printf("%u\n", sizeof(u));
    memset(&u, 0, sizeof(u));
    u.update_region.top = 0;
    u.update_region.left = 0;
    u.update_region.width = 800;
    u.update_region.height = 600;
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
    int retval = ioctl(fd, MXCFB_SEND_UPDATE, &u);
    if (retval)
        perror("ioctl MXCFB_SEND_UPDATE");
}

Mx50Epdc::~Mx50Epdc()
{
    close(fd);
}

void Mx50Epdc::setPixelFormat()
{
    fb_var_screeninfo screen_info;
    screen_info.bits_per_pixel = 8;
    screen_info.grayscale = GRAYSCALE_8BIT;
    int retval = ioctl(fd, FBIOPUT_VSCREENINFO, &screen_info);
    if (retval)
        perror("ioctl FBIOPUT_VSCREENINFO");
}

void Mx50Epdc::setAutoUpdateMode(bool autoUpdate)
{
    uint32_t mode;
    mode = autoUpdate ? AUTO_UPDATE_MODE_AUTOMATIC_MODE : AUTO_UPDATE_MODE_REGION_MODE;
    int retval = ioctl(fd, MXCFB_SET_AUTO_UPDATE_MODE, &mode);
    if (retval)
        perror("ioctl MXCFB_SET_AUTO_UPDATE_MODE");
}

void Mx50Epdc::setUpdateScheme()
{
    fb_var_screeninfo screen_info;
//    screen_info.scheme = UPDATE_SCHEME_SNAPSHOT;
    int retval = ioctl(fd, MXCFB_SET_UPDATE_SCHEME, &screen_info);
    if (retval)
        perror("ioctl MXCFB_SET_UPDATE_SCHEME");
}

