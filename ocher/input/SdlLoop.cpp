#include <SDL/SDL.h>


void SdlEventLoop()
{
    SDL_Event event;

    if (SDL_PollEvent(&event) == 0)
        return;
    switch (event.type) {
    case SDL_ACTIVEEVENT:
    {
        if (event.active.state & SDL_APPACTIVE) {
            if (event.active.gain) {
            } else {
            }
        }
        break;
    }

    case SDL_KEYUP:
    {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                break;
            case SDLK_RIGHT:
                break;
            case SDLK_UP:
                break;
            case SDLK_DOWN:
                break;
            default:
                break;
        }
        break;
    }

    case SDL_KEYDOWN:
    {
        switch (event.key.keysym.sym) {
        case SDLK_TAB:
            break;
        case SDLK_SPACE:
            break;
        case SDLK_LEFT:
            break;
        case SDLK_RIGHT:
            break;
        case SDLK_UP:
            break;
        case SDLK_DOWN:
            break;
        case SDLK_F1:
            break;
        case SDLK_ESCAPE:
            break;
        default:
            break;
        }
        break;
    }

    case SDL_MOUSEMOTION:
    {
        //event.motion.x
        //event.motion.y
        break;
    }

    case SDL_MOUSEBUTTONUP:
    {
        if (event.button.button == SDL_BUTTON_LEFT) {
            int x, y;
            x = event.button.x;
            y = event.button.y;
        }
        break;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event.button.button == SDL_BUTTON_LEFT) {
        }
        break;
    }

    case SDL_QUIT:
    {
    }
    }
}

