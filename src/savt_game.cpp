#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>
#include <varand/varand_memarena.h>
#include "va_linmath.h"
#include "va_colors.h"

#include <sdl2/SDL_scancode.h>
#include <sdl2/SDL_mouse.h>
#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>

#include <cstdio>

struct game_state
{
    b32 IsInitialized;

    memory_arena RootArena;
    memory_arena WorldArena;
    memory_arena TransientArena;

    sav_texture Texture;
    
    b32 MouseRel;

    b32 Borderless;

    f32 TriX;
    f32 TriY;
};

GAME_API void
UpdateAndRender(b32 *Quit, b32 Reloaded, game_memory GameMemory) 
{
    game_state *GameState = (game_state *) GameMemory.Data;
    
    if (!GameState->IsInitialized)
    {
        Assert(sizeof(game_state) < Megabytes(16));
        u8 *RootArenaBase = (u8 *) GameMemory.Data + Megabytes(16);
        size_t RootArenaSize = GameMemory.Size - Megabytes(16);
        GameState->RootArena = MemoryArena(RootArenaBase, RootArenaSize);

        GameState->WorldArena = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        GameState->TransientArena = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        
        GameState->Texture = SavLoadTexture("res/test.png");

        color TestColor = Color(128, 0, 255, 255);
        color TestColor2 = Color(0xAA00FFFF);
        
        GameState->IsInitialized = true;
    }

    MemoryArena_Reset(&GameState->TransientArena);

    BeginDraw();

    DrawTexture(GameState->Texture, ColorV4(VA_MAROON));

    DrawRect(ColorV4(VA_AQUAMARINE));

    EndDraw();
    
    char Title[256];
    sprintf_s(Title, "SAV %0.3f FPS | %0.3f ms", GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
