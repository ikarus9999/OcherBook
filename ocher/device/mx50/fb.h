#ifndef MX50_FB_H
#define MX50_FB_H

class Mx50Epdc
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

