#ifndef SDL_FB_H
#define SDL_FB_H

#include "SDL/SDL.h"

#include "ocher/output/FrameBuffer.h"


class FbSdl : public FrameBuffer
{
public:
    FbSdl();
    virtual ~FbSdl();

    bool init();

    unsigned int height();
    unsigned int width();
    unsigned int dpi();

    void clear();
    void blit(unsigned char *p, int x, int y, int w, int h);
    int update(int x, int y, int w, int h, bool full=true);

protected:
    SDL_Surface *m_screen;
};

#endif

