#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>
#include <varand/varand_memarena.h>
#include "va_linmath.h"
#include "va_colors.h"
#include "va_rect.h"

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

    if (KeyPressed(SDL_SCANCODE_F11))
    {
        GameState->Borderless = !GameState->Borderless;
        SetWindowBorderless(GameState->Borderless);
    }

    static f32 Rot = 0.0f;
    static f32 Scale = 1.0f;
    static b32 ScaleIncreasing = true;

    Rot += 30.0f * (f32) GetDeltaPrev();
    if (Rot >= 360.0f) Rot -= 360.0f;

    if (ScaleIncreasing)
    {
        Scale += 1.0f * (f32) GetDeltaPrev();
        if (Scale >= 2.0f)
        {
            Scale = 2.0f;
            ScaleIncreasing = false;
        }
    }
    else
    {
        Scale -= 1.0f * (f32) GetDeltaPrev();
        if (Scale <= 0.5f)
        {
            Scale = 0.5f;
            ScaleIncreasing = true;
        }
    }

    BeginDraw();
    {
        DrawRect(Rect(300, 300, 1600, 500), ColorV4(VA_AQUAMARINE));

        DrawTexture(GameState->Texture,
                    Rect(GetWindowSize().Width / 2.0f, GetWindowSize().Height / 2.0f, (f32) GameState->Texture.Width * Scale, (f32) GameState->Texture.Height * Scale),
                    Rect(GameState->Texture.Width, GameState->Texture.Height),
                    Vec2(GameState->Texture.Width * Scale / 2.0f, GameState->Texture.Height * Scale / 2.0f),
                    Rot,
                    ColorV4(VA_MAROON));
    } 
    EndDraw();
    
    char Title[256];
    sprintf_s(Title, "SAV %0.3f FPS | %0.3f ms", GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
