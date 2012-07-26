#ifndef MX50_FB_H
#define MX50_FB_H

#include "ocher/output/FrameBuffer.h"

class Mx50Epdc : public FrameBuffer
{
public:
    Mx50Epdc();
    ~Mx50Epdc();

    void setPixelFormat();
    void setUpdateScheme();
    void setAutoUpdateMode(bool autoUpdate);
    int fd;
};

#endif

