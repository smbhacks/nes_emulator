#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_SDL2
#define CIMGUI_USE_GLFW
#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <cimgui.h>
#include <cimgui_impl.h>
#include <GL/gl3w.h>
#include <nfd.h>

#include "core/NES.h"

const int WINDOW_SIZE = 2;
const int FPS = 60;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL Init problema: %s", SDL_GetError());
        return 1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    igCreateContext(NULL);
    ImGuiIO* io = igGetIO_Nil();

    SDL_Window* window = SDL_CreateWindow(
        "NES emulátor", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        256 * WINDOW_SIZE, 240 * WINDOW_SIZE, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (window == NULL) {
        printf("Nem sikerult letrehozni az ablakot: %s", SDL_GetError());
        return 1;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); //vsync

    if(gl3wInit() != 0)
    {
        printf("Nem sikerult inicializalni az OpenGL Loadert!");
        return 1;
    }

    io->IniFilename = NULL; 
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        printf("Nem sikerult letrehozni a renderert: %s", SDL_GetError());
        return 1;
    }
    SDL_RenderClear(renderer);

    NES* nes = CreateNES();

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
            ImGui_ImplSDL2_ProcessEvent(&e);
            switch (e.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            default:
                break;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        igNewFrame();

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

        float menubarHeight = 0;
        if (igBeginMainMenuBar())
        {
            menubarHeight = igGetWindowSize().y;
            if (igBeginMenu("File", true))
            {
                if (igMenuItem_Bool("Open iNES file..", "Ctrl+O", false, true)) 
                { 
                    nfdchar_t *outPath = NULL;
                    nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );
                                
                    if ( result == NFD_OKAY ) 
                    {
                        RemoveCartNES(nes);
                        SetCartNES(nes, outPath);
                        ResetNES(nes);
                        free(outPath);
                        SDL_SetWindowSize(window, 256 * WINDOW_SIZE, 240 * WINDOW_SIZE + menubarHeight);
                    }
                    else 
                    {
                        printf("Nem sikerult megnyitni a .nes fajlt: %s\n", NFD_GetError() );
                    }
                }
                if (igMenuItem_Bool("Close ROM", "Ctrl+C", false, true)) 
                { 
                    RemoveCartNES(nes);              
                }
                if (igMenuItem_Bool("Reset", "Ctrl+R", false, true)) 
                { 
                    ResetNES(nes);
                }
                if (igMenuItem_Bool("Exit", "Alt+F4", false, true)) 
                { 
                    running = false;
                }
                igEndMenu();
            }
            if (igBeginMenu("Video", true))
            {
                if (igMenuItem_Bool("Load .pal file..", "Ctrl+P", false, true))
                {
                    nfdchar_t* outPath = NULL;
                    nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

                    if (result == NFD_OKAY)
                    {
                        uint8_t* p = MallocPalette(outPath);
                        if (p != NULL)
                        {
                            UseCustomPalette(nes, p);
                        }
                        else
                        {
                            printf("Nem sikerult hasznalni a .pal fajlt (nem jo fajlmeret, 64x RGB24 kell)\n");
                        }
                    }
                    else
                    {
                        printf("Nem sikerult megnyitni a .pal fajlt: %s\n", NFD_GetError());
                    }
                }
                if (igMenuItem_Bool("Use default palette", "Ctrl+D", false, true))
                {
                    ResetPalette(nes->ppu);
                }
                igEndMenu();
            }
            igEndMainMenuBar();
        }

        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_RenderClear(renderer);
        SDL_Rect dst = {
            0, menubarHeight, 256*WINDOW_SIZE, 240*WINDOW_SIZE
        };
        SDL_RenderCopy(renderer, displayTexture, NULL, &dst);        

        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

        SDL_GL_SwapWindow(window);

        // ha hamarabb befejezzük ezt a frame-t, mint MSPF, akkor várjuk meg (így elérjük a kívánt FPS-t)
        time = SDL_GetTicks() - timerStart;
        if (MSPF > time)
        {
            SDL_Delay(MSPF - time);
        }
    }

    RemoveCartNES(nes);
    DestroyNES(nes);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(NULL);
    SDL_GL_DeleteContext(gl_context);
    if (window != NULL)
    {
      SDL_DestroyWindow(window);
      window = NULL;
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}