#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>
#include <varand/varand_memarena.h>

#include <sdl2/SDL_scancode.h>
#include <sdl2/SDL_mouse.h>

#include <cstdio>

struct game_state
{
    b32 IsInitialized;

    memory_arena RootArena;
    memory_arena WorldArena;
    memory_arena TransientArena;

    u32 ShaderProgram;
    u32 VBO;
    u32 VAO;

    u64 CurrentFrame;

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
        
        GameState->ShaderProgram = BuildShader();
        PrepareGpuData(&GameState->VBO, &GameState->VAO);

        GameState->CurrentFrame = 0;

        GameState->IsInitialized = true;
    }

    if (Reloaded)
    {
        GameState->MouseRel = GetMouseRelativeMode();
    }

    MemoryArena_Reset(&GameState->TransientArena);

    if (KeyDown(SDL_SCANCODE_1))
    {
        TraceLog("KeyDown");
    }

    if (KeyPressed(SDL_SCANCODE_2))
    {
        TraceLog("KeyPressed");
    }
    if (KeyReleased(SDL_SCANCODE_2))
    {
        TraceLog("KeyReleased");
    }

    if (KeyRepeat(SDL_SCANCODE_3))
    {
        TraceLog("KeyRepeat");
    }

    if (KeyPressedOrRepeat(SDL_SCANCODE_4))
    {
        TraceLog("KeyPressedOrRepeat");
    }

    if (KeyPressed(SDL_SCANCODE_SPACE))
    {
        GameState->MouseRel = !GameState->MouseRel;
        SetMouseRelativeMode(GameState->MouseRel);
        TraceLog("Mouse Relative Mode: %d", GameState->MouseRel);
    }

    if (MouseDown(SDL_BUTTON_X1))
    {
        TraceLog("MouseDown: X1; Abs(%d, %d)", GetMousePos().X, GetMousePos().Y);
    }

    if (MouseDown(SDL_BUTTON_X2))
    {
        TraceLog("MouseDown: X2; Rel(%d, %d)", GetMouseRelPos().X, GetMouseRelPos().Y);
    }

    if (MouseClicks(SDL_BUTTON_LEFT, 1))
    {
        TraceLog("MouseClicks: left, single");
    }

    if (MouseClicks(SDL_BUTTON_LEFT, 2))
    {
        TraceLog("MouseClicks: left, double");
    }

    if (MouseClicks(SDL_BUTTON_LEFT, 3))
    {
        TraceLog("MouseClicks: left, triple");
    }

    if (MousePressed(SDL_BUTTON_RIGHT))
    {
        TraceLog("MousePressed: right");
    }

    if (MouseReleased(SDL_BUTTON_RIGHT))
    {
        TraceLog("MouseReleased: right");
    }

    if (MousePressed(SDL_BUTTON_MIDDLE))
    {
        TraceLog("MousePressed: middle");
    }

    if (MouseWheel())
    {
        TraceLog("MouseWheel: %d", MouseWheel());
    }

    if (KeyPressed(SDL_SCANCODE_F1))
    {
        TraceLog("Window size: Orig(%d, %d), Curr(%d, %d)", GetWindowSize().OriginalWidth, GetWindowSize().OriginalHeight, GetWindowSize().Width, GetWindowSize().Height); 
    }

    if (KeyPressed(SDL_SCANCODE_F11))
    {
        GameState->Borderless = !GameState->Borderless;
        SetWindowBorderless(GameState->Borderless);
    }

    f32 Rate = 1.0f;
    if (KeyDown(SDL_SCANCODE_W))
    {
        GameState->TriY += Rate * (f32) GetDeltaPrev();
    }
    if (KeyDown(SDL_SCANCODE_S))
    {
        GameState->TriY -= Rate * (f32) GetDeltaPrev();
    }
    if (KeyDown(SDL_SCANCODE_A))
    {
        GameState->TriX -= Rate * (f32) GetDeltaPrev();
    }
    if (KeyDown(SDL_SCANCODE_D))
    {
        GameState->TriX += Rate * (f32) GetDeltaPrev();
    }

    if (GameState->TriX >= 1.0f)
    {
        GameState->TriX = -1.0f;
    }
    else if (GameState->TriX <= -1.0f)
    {
        GameState->TriX = 1.0f;
    }
    if (GameState->TriY >= 1.0f)
    {
        GameState->TriY = -1.0f;
    }
    else if (GameState->TriY <= -1.0f)
    {
        GameState->TriY = 1.0f;
    }

    BeginDraw();
    {
        float Triangle[] = {
            GameState->TriX + 0.0f, GameState->TriY + 0.1f, 0.0f,
            GameState->TriX + 0.1f, GameState->TriY - 0.1f, 0.0f,
            GameState->TriX - 0.1f, GameState->TriY - 0.1f, 0.0,
        };
    
        DrawVertices(GameState->ShaderProgram, GameState->VBO, GameState->VAO, Triangle, ArrayCount(Triangle) / 3);
    }
    EndDraw();

    char Title[256];
    sprintf_s(Title, "SAV Instantaneous: (%.3f FPS | %0.3f ms) Average: (%0.3f FPS | %0.3f ms)", GetFPSPrev(), GetDeltaPrev(), GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
