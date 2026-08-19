#ifndef GUARD_PLATFORM_BOX_TRANSFER_H
#define GUARD_PLATFORM_BOX_TRANSFER_H

#ifdef PLATFORM_SDL2
#include <SDL2/SDL.h>
void RunBoxTransferUI(SDL_Window *win, SDL_GLContext glCtx);
#endif

#endif
