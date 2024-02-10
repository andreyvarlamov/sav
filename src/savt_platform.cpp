#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>

#include <sdl2/SDL_scancode.h>

#include <cstdio>
#include <cstdlib>

typedef void (*RenderDecl)(b32 *quit, b32 reloaded, game_memory gameMemory);

static RenderDecl Render;

int main(int argc, char **argv)
{
    InitWindow("SAV", 1920, 1080);

    // RenderDecl *RenderPtr = &Render;
    InitGameCode("bin/savt_game.dll", (void **) &Render);

    game_memory gameMemory = AllocGameMemory(Megabytes(128));

    b32 quit = false;
    while (!quit)
    {
        PollEvents(&quit);
            
        if (KeyPressed(SDL_SCANCODE_ESCAPE))
        {
            quit = true;
        }

        b32 reloaded = ReloadGameCode((void **) &Render);
                        
        if (Render)
        {
            Render(&quit, reloaded, gameMemory);
        }
    }

    Quit();
    
    return 0;
}
