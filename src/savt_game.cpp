#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>

#include <cstdio>

struct game_state
{
    b32 isInitialized;

    u32 shaderProgram;
    u32 vbo;
    u32 vao;

    u64 currentFrame;
};

GAME_API void Render(b32 *quit, game_memory gameMemory) 
{
    game_state *gameState = (game_state *) gameMemory.data;
    
    if (!gameState->isInitialized)
    {
        gameState->shaderProgram = BuildShader();
        PrepareGpuData(&gameState->vbo, &gameState->vao);

        gameState->currentFrame = 0;

        gameState->isInitialized = true;
    }

    // const u8 *sdlKeyboardState = GetSdlKeyboardState();

    BeginDraw();

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

    char title[256];
    sprintf_s(title, "SAV (%zu)", gameState->currentFrame++ / 10);
    SetWindowTitle(title);

    EndDraw();
}
