#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>

#include <sdl2/SDL_scancode.h>

#include <cstdio>
#include <cstdlib>

typedef void (*RenderDecl)(b32 *quit, game_memory gameMemory);

static RenderDecl Render;

int main(int argc, char **argv)
{
    InitWindow("SAV", 1920, 1080);

    Render = (RenderDecl) InitGameCode("bin/savt_game.dll");

    game_memory gameMemory = AllocGameMemory(Megabytes(128));

    b32 quit = false;
    while (!quit)
    {
        PollEvents(&quit);
            
        const u8 *sdlKeyboardState = GetSdlKeyboardState();
        if (sdlKeyboardState[SDL_SCANCODE_ESCAPE])
        {
            quit = true;
        }

        Render = (RenderDecl) ReloadGameCode();
                        
        if (Render)
        {
            Render(&quit, gameMemory);
        }
    }

    Quit();
    
    return 0;
}
