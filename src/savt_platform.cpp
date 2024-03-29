#include "sav_lib.h"

#include "va_util.h"
#include "va_types.h"

#include <sdl2/SDL_scancode.h>

#include <cstdio>
#include <cstdlib>

typedef void (*UpdateAndRenderDecl)(b32 *quit, b32 reloaded, game_memory gameMemory);

static UpdateAndRenderDecl UpdateAndRender;

int main(int argc, char **argv)
{
    InitWindow("SAV", 1920, 1080);

    InitAudio();

    // RenderDecl *RenderPtr = &Render;
    InitGameCode("bin/savt_game.dll", "UpdateAndRender", (void **) &UpdateAndRender);

    game_memory GameMemory = AllocGameMemory(Megabytes(128));

    SetTargetFPS(60.0);

    b32 ShouldQuit = false;
    while (!ShouldQuit)
    {
        BeginFrameTiming();
        
        PollEvents(&ShouldQuit);
            
        if (KeyPressed(SDL_SCANCODE_ESCAPE))
        {
            ShouldQuit = true;
        }

        if (KeyPressed(SDL_SCANCODE_F5))
        {
            DumpGameMemory(GameMemory);
        }

        if (KeyPressed(SDL_SCANCODE_F7))
        {
            ReloadGameMemoryDump(GameMemory);
        }

        b32 Reloaded = ReloadGameCode((void **) &UpdateAndRender);
                        
        if (UpdateAndRender)
        {
            UpdateAndRender(&ShouldQuit, Reloaded, GameMemory);
        }

        EndFrameTiming();
    }

    Quit();
    
    return 0;
}
