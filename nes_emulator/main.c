#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "NES.h"

const int WINDOW_SIZE = 2;
const int FPS = 60;

/*void UpdateDisplayTexture(PPU* ppu, SDL_Texture* texture, int numLoops)
{
    void* pixelsPtr;
    int bytesPerRow;
    SDL_LockTexture(texture, NULL, &pixelsPtr, &bytesPerRow);

    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            uint8_t r = x + numLoops;
            uint8_t g = y + numLoops;
            uint8_t b = numLoops;
            uint8_t* pixel = (uint8_t*)pixelsPtr + y * bytesPerRow + x * 3;
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
        }
    }

    SDL_UnlockTexture(texture);
}*/

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

    NES nes = CreateNES();
    nes.cpu.ppu = &nes.ppu;
    SetCartNES(&nes, "nestest.nes");
    ResetNES(&nes);

    // SDL Texture létrehozása, amit majd a renderer megjelenít
    SDL_Texture* displayTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    uint32_t timerStart, time;
    const int MSPF = 1000 / FPS; //milliszekundumok száma egy frame-ben
    bool running = true;
    int numLoops = 0;
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

        int tmp;
        SDL_LockTexture(displayTexture, NULL, &nes.ppu.display, &tmp);
        TickNES(&nes); // futtasuk az emulátort 1 frame-t
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
        numLoops++;
    }

    RemoveCartNES(&nes);
    DestroyNES(&nes);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}