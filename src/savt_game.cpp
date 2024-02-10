#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>
#include <varand/varand_memarena.h>

#include <sdl2/SDL_scancode.h>
#include <sdl2/SDL_mouse.h>

#include <cstdio>

struct game_state
{
    b32 isInitialized;

    memory_arena rootArena;
    memory_arena worldArena;
    memory_arena transientArena;

    u32 shaderProgram;
    u32 vbo;
    u32 vao;

    u64 currentFrame;

    b32 mouseRel;

    b32 borderless;
};

GAME_API void UpdateAndRender(b32 *quit, b32 reloaded, game_memory gameMemory) 
{
    game_state *gameState = (game_state *) gameMemory.data;
    
    if (!gameState->isInitialized)
    {
        Assert(sizeof(game_state) < Megabytes(16));
        u8 *rootArenaBase = (u8 *) gameMemory.data + Megabytes(16);
        size_t rootArenaSize = gameMemory.size - Megabytes(16);
        gameState->rootArena = MemoryArena(rootArenaBase, rootArenaSize);

        gameState->worldArena = MemoryArenaNested(&gameState->rootArena, Megabytes(16));
        gameState->transientArena = MemoryArenaNested(&gameState->rootArena, Megabytes(16));
        
        gameState->shaderProgram = BuildShader();
        PrepareGpuData(&gameState->vbo, &gameState->vao);

        gameState->currentFrame = 0;

        gameState->isInitialized = true;
    }

    if (reloaded)
    {
        gameState->mouseRel = GetMouseRelativeMode();
    }

    MemoryArena_Reset(&gameState->transientArena);

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
        gameState->mouseRel = !gameState->mouseRel;
        SetMouseRelativeMode(gameState->mouseRel);
        TraceLog("Mouse Relative Mode: %d", gameState->mouseRel);
    }

    if (MouseDown(SDL_BUTTON_X1))
    {
        TraceLog("MouseDown: X1; Abs(%d, %d)", GetMousePos().x, GetMousePos().y);
    }

    if (MouseDown(SDL_BUTTON_X2))
    {
        TraceLog("MouseDown: X2; Rel(%d, %d)", GetMouseRelPos().x, GetMouseRelPos().y);
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
        TraceLog("Window size: Orig(%d, %d), Curr(%d, %d)", GetWindowSize().originalWidth, GetWindowSize().originalHeight, GetWindowSize().width, GetWindowSize().height); 
    }

    if (KeyPressed(SDL_SCANCODE_F11))
    {
        gameState->borderless = !gameState->borderless;
        SetWindowBorderless(gameState->borderless);
    }

    BeginDraw();
    {
        if ((gameState->currentFrame / 100) % 2 == 0)
        {
            float triangle[] = {
                // -0.5f, -1.0f, 0.0f,
                // -1.0f, 1.0f, 0.0f,
                // 0.0f, 1.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f
            };
    
            DrawVertices(gameState->shaderProgram, gameState->vbo, gameState->vao, triangle, ArrayCount(triangle) / 3);
        }
        else
        {
            float triangle[] = {
                -0.5f, -1.0f, 0.0f,
                -1.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                // 0.0f, -1.0f, 0.0f,
                // 0.0f, 1.0f, 0.0f,
                // 1.0f, 1.0f, 0.0f
            };
    
            DrawVertices(gameState->shaderProgram, gameState->vbo, gameState->vao, triangle, ArrayCount(triangle) / 3);
        }
    }
    EndDraw();

    char title[256];
    sprintf_s(title, "SAV Instantaneous: (%.3f FPS | %0.3f ms) Average: (%0.3f FPS | %0.3f ms)", GetFPSPrev(), GetDeltaPrev(), GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(title);
}
