#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "NES.h"

const int WINDOW_SIZE = 1;
const int NES_WIDTH = 256;
const int NES_HEIGHT = 240;
const int FPS = 60;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL Init problema: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("NES emulátor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, NES_WIDTH * WINDOW_SIZE, NES_HEIGHT * WINDOW_SIZE, 0);
    if (window == NULL) {
        printf("Nem sikerult letrehozni az ablakot: %s", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        printf("Nem sikerult letrehozni a renderert: %s", SDL_GetError());
        return 1;
    }
    SDL_RenderClear(renderer);

    NES nes = CreateNES();
    SetCartNES(&nes, "ld.nes");
    ResetNES(&nes);

    uint32_t timerStart, time;
    const int MSPF = 1000 / FPS; //milliszekundumok száma egy frame-ben
    bool running = true;
    while (running)
    {
        timerStart = SDL_GetTicks();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            default:
                break;
            }
        }

        TickNES(&nes); // futtasuk az emulátort 1 frame-t

        // ha hamarabb befejezzük ezt a frame-t, mint MSPF, akkor várjuk meg (így elérjük a kívánt FPS-t)
        time = SDL_GetTicks() - timerStart;
        if (MSPF > time)
        {
            SDL_Delay(MSPF - time);
        }
    }

    RemoveCartNES(&nes);
    DestroyNES(&nes);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}