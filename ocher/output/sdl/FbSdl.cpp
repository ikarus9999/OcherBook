#include <exception>
#include "SDL/SDL.h"

#include "clc/support/Logger.h"

#include "ocher/output/sdl/FbSdl.h"


FbSdl::FbSdl() :
    m_screen(0)
{
}

FbSdl::~FbSdl()
{
}

bool FbSdl::init()
{
    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0) {
        const char *err = SDL_GetError();
        clc::Log::error("ocher.sdl", "SDL_Init failed: %s", err);
        return false;
    }
    atexit(SDL_Quit);

    // TODO:  store window size in user settings
    m_screen = SDL_SetVideoMode(600, 800, 8, SDL_SWSURFACE);
    if (! m_screen) {
        const char *err = SDL_GetError();
        clc::Log::error("ocher.sdl", "SDL_SetVideoMode failed: %s", err);
        return false;
    }
    clear();
    return true;
}

unsigned int FbSdl::height()
{
    return m_screen->h;
}

unsigned int FbSdl::width()
{
    return m_screen->w;
}

unsigned int FbSdl::dpi()
{
    return 120;  // TODO
}

void FbSdl::clear()
{
    clc::Log::debug("ocher.sdl", "clear");
    SDL_FillRect(m_screen, NULL, 0x0);
    SDL_UpdateRect(m_screen, 0, 0, 0, 0);
}

void FbSdl::blit(unsigned char *p, int x, int y, int w, int h)
{
    clc::Log::debug("ocher.sdl", "blit");
    if ( SDL_MUSTLOCK(m_screen) ) {
        if ( SDL_LockSurface(m_screen) < 0 ) {
            return;
        }
    }

    //SDL_BlitSurface(m_screen, NULL, 
    for (int i = 0; i < h; ++i) {
        memcpy(((unsigned char*)m_screen->pixels) + y*m_screen->pitch + x, p, w);
        y++;
        p = p + w;
    }
    if ( SDL_MUSTLOCK(m_screen) ) {
        SDL_UnlockSurface(m_screen);
    }
}

int FbSdl::update(int x, int y, int w, int h, bool /*full*/)
{
    clc::Log::debug("ocher.sdl", "update");
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;
    SDL_UpdateRects(m_screen, 1, &dest);
    return 0;
}

