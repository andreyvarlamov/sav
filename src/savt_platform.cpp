#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>

#include <sdl2/SDL_scancode.h>

#include <cstdio>
#include <cstdlib>

typedef void (*UpdateAndRenderDecl)(b32 *quit, b32 reloaded, game_memory gameMemory);

static UpdateAndRenderDecl UpdateAndRender;

int main(int argc, char **argv)
{
    InitWindow("SAV", 1920, 1080);

    // RenderDecl *RenderPtr = &Render;
    InitGameCode("bin/savt_game.dll", "UpdateAndRender", (void **) &UpdateAndRender);

    game_memory gameMemory = AllocGameMemory(Megabytes(128));

    b32 quit = false;
    while (!quit)
    {
        PollEvents(&quit);
            
        if (KeyPressed(SDL_SCANCODE_ESCAPE))
        {
            quit = true;
        }

        b32 reloaded = ReloadGameCode((void **) &UpdateAndRender);
                        
        if (UpdateAndRender)
        {
            UpdateAndRender(&quit, reloaded, gameMemory);
        }
    }

    Quit();
    
    return 0;
}
