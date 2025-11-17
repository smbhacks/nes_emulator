#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "core/NES.h"

const int WINDOW_SIZE = 2;
const int FPS = 60;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL Init problema: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("NES emulátor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * WINDOW_SIZE, 240 * WINDOW_SIZE, 0);
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

    NES* nes = CreateNES();
    SetCartNES(nes, "rom.nes");
    ResetNES(nes);

    // SDL Texture létrehozása, amit majd a renderer megjelenít
    SDL_Texture* displayTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    uint32_t timerStart, time;
    const unsigned int MSPF = 1000 / FPS; //milliszekundumok száma egy frame-ben
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

        // https://wiki.libsdl.org/SDL2/SDL_GetKeyboardState
        // Azt írja, hogy kéne SDL_PumpEvents() ezelőtt, de a PollEvent már meghívja amúgy is, szóval nem kell mégegyszer
        // 1-et ad, ha le van nyomva, 0-t ha nincs.
        // Ez tökéletes lesz az 1-bites kontroller változokhoz
        const uint8_t* keyStates = SDL_GetKeyboardState(NULL);
        nes->controller->a      = keyStates[SDL_SCANCODE_C];
        nes->controller->b      = keyStates[SDL_SCANCODE_X];
        nes->controller->select = keyStates[SDL_SCANCODE_SPACE];
        nes->controller->start  = keyStates[SDL_SCANCODE_RETURN];
        nes->controller->up     = keyStates[SDL_SCANCODE_UP];
        nes->controller->down   = keyStates[SDL_SCANCODE_DOWN];
        nes->controller->left   = keyStates[SDL_SCANCODE_LEFT];
        nes->controller->right  = keyStates[SDL_SCANCODE_RIGHT];

        int tmp;
        SDL_LockTexture(displayTexture, NULL, (void**)&nes->ppu->display, &tmp);
        TickNES(nes); // futtasuk az emulátort 1 frame-t
        SDL_UnlockTexture(displayTexture);

        //UpdateDisplayTexture(&nes.ppu, displayTexture, numLoops);
        SDL_RenderCopy(renderer, displayTexture, NULL, NULL); // texture megjelenítése az egész képernyőn
        SDL_RenderPresent(renderer);

        // ha hamarabb befejezzük ezt a frame-t, mint MSPF, akkor várjuk meg (így elérjük a kívánt FPS-t)
        time = SDL_GetTicks() - timerStart;
        if (MSPF > time)
        {
            SDL_Delay(MSPF - time);
        }
    }

    RemoveCartNES(nes);
    DestroyNES(nes);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}