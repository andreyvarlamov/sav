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
    u32 VAO2;
    u32 EBO;
    sav_texture Texture;
    sav_texture DefaultTexture;
    
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

        glGenVertexArrays(1, &GameState->VAO2);
        glBindVertexArray(GameState->VAO2);
        glBindBuffer(GL_ARRAY_BUFFER, GameState->VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) (8192 * 3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // float c[] = { 0.0f, 1.0f ,0.0f, 1.0f };
        // glVertexAttrib4fv(2, c);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GameState->EBO);
        // glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, IndexCount * sizeof(float), Indices);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        u32 White = 0xFFFFFFFF;
        GameState->DefaultTexture = SavLoadTextureFromData(&White, 1, 1);
        GameState->Texture = SavLoadTexture("res/test.png");
        
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
    vec4 Colors[] = {
        Vec4(1,0,0,1),
        Vec4(0,1,0,1),
        Vec4(1,0,0,1),
        Vec4(0,1,0,1),
    };
    vec4 Colors1[] = {
        Vec4(0,1,0,1),
        Vec4(0,0,1,1),
        Vec4(0,1,0,1),
        Vec4(0,0,1,1),        
    };
    u32 Indices[] = { 0, 1, 2, 2, 3, 0 };

    BeginDraw();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GameState->Texture.Glid);

    float c[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glVertexAttrib4fv(2, c);

    DrawVertices(GameState->ShaderProgram, GameState->VBO, GameState->VAO2, GameState->EBO,
                 Positions, TexCoords, Colors, Indices,
                 ArrayCount(Positions), ArrayCount(Indices));

    glBindTexture(GL_TEXTURE_2D, 0);

    for (int i = 0; i < ArrayCount(Positions); i++)
    {
        Positions[i] += Vec3(0.4f, 0.0f, 0.0f);
    }

    glBindTexture(GL_TEXTURE_2D, GameState->DefaultTexture.Glid);
    
    DrawVertices(GameState->ShaderProgram, GameState->VBO, GameState->VAO, GameState->EBO,
                 Positions, TexCoords, Colors1, Indices,
                 ArrayCount(Positions), ArrayCount(Indices));

    EndDraw();
    
    char Title[256];
    sprintf_s(Title, "SAV %0.3f FPS | %0.3f ms", GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
