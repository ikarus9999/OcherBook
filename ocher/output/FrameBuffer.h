#ifndef OCHER_FRAMEBUFFER_H
#define OCHER_FRAMEBUFFER_H


class FrameBuffer
{
public:
    FrameBuffer() {}

    virtual unsigned int height() = 0;
    virtual unsigned int width() = 0;
    virtual unsigned int dpi() = 0;

    virtual void clear() = 0;
    virtual void blit(unsigned char *p, int x, int y, int w, int h) = 0;
    virtual int update(int x, int y, int w, int h, bool full) = 0;
};

#endif

