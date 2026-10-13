/* video.c — window and presentation (scaled, aspect-correct). */
#include <SDL.h>

#include "pc_port.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static int tex_w, tex_h;

int PcVideoInit(const char *title)
{
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              GBA_WIDTH * 4, GBA_HEIGHT * 4, SDL_WINDOW_RESIZABLE);
    if (!window) return -1;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) return -1;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    return 0;
}

void PcVideoPresent(const uint32_t *frame, int width, int height)
{
    int ww, wh;
    SDL_Rect dst;

    if (!texture || tex_w != width || tex_h != height) {
        if (texture) SDL_DestroyTexture(texture);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                    width, height);
        tex_w = width;
        tex_h = height;
    }
    SDL_UpdateTexture(texture, NULL, frame, width * 4);

    SDL_GetRendererOutputSize(renderer, &ww, &wh);
    dst.w = ww;
    dst.h = ww * height / width;
    if (dst.h > wh) {
        dst.h = wh;
        dst.w = wh * width / height;
    }
    dst.x = (ww - dst.w) / 2;
    dst.y = (wh - dst.h) / 2;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_RenderPresent(renderer);
}

void PcVideoShutdown(void)
{
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
}
