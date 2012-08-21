#ifndef MX50_FB_H
#define MX50_FB_H

#include <linux/mxcfb.h>

#include "ocher/output/FrameBuffer.h"

class Mx50Fb : public FrameBuffer
{
public:
    Mx50Fb();
    ~Mx50Fb();

    bool init();

    unsigned int height();
    unsigned int width();
    unsigned int dpi() { return 170; }  // Kobo Touch -- measure it yourself!

    void clear();
    void blit(unsigned char *p, int x, int y, int w, int h);
    int update(int x, int y, int w, int h, bool full);

    /**
     * @param marker  Waits on the specified update, or -1 for all
     */
    void waitUpdate(int marker = -1);

    void setPixelFormat();
    void setUpdateScheme();
    void setAutoUpdateMode(bool autoUpdate);

    int m_fd;
    char *m_fb;
    size_t m_fbSize;
    int m_marker;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
};

#endif

