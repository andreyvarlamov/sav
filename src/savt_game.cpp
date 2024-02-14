#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>
#include <varand/varand_memarena.h>
#include <varand/varand_linmath.h>

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

    u32 ShaderProgram;
    u32 VBO;
    u32 VAO;
    u32 EBO;
    u32 TextureID;
    
    
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
        
        GameState->ShaderProgram = BuildBasicShader();
        PrepareGpuData(&GameState->VBO, &GameState->VAO, &GameState->EBO);

        GameState->TextureID = LoadTexture("res/test.png");
        
        GameState->CurrentFrame = 0;

        GameState->IsInitialized = true;
    }

    MemoryArena_Reset(&GameState->TransientArena);

    vec3 Positions[] = {
        Vec3(-0.1f, -0.1f, 0.0f),
        Vec3(0.1f, -0.1f, 0.0f),
        Vec3(0.1f, 0.1f, 0.0f),
        Vec3(-0.1f, 0.1f, 0.0f)
    };
    vec2 TexCoords[] = {
        Vec2(0, 0),
        Vec2(0, 1),
        Vec2(1, 1),
        Vec2(1, 0)
    };
    u32 Indices[] = { 0, 1, 2, 2, 3, 0 };

    BeginDraw();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GameState->TextureID);

    DrawVertices(GameState->ShaderProgram, GameState->VBO, GameState->VAO, GameState->EBO,
                 Positions, TexCoords, 0, Indices,
                 ArrayCount(Positions), ArrayCount(Indices));
    EndDraw();
    
    char Title[256];
    sprintf_s(Title, "SAV %0.3f FPS | %0.3f ms", GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
