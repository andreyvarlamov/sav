#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>

// #define GLAD_GLAPI_EXPORT
// #include <glad/glad.h>
#include <sdl2/SDL.h>

#include <cstdio>

#include <vector>

GAME_API void Render(b32 *quit, b32 *isInitialized, SDL_Window *window, int currentFrame, u32 *shaderProgram, u32 *vbo, u32 *vao) 
{
    if (!*isInitialized)
    {
        printf("%u, %u\n", *shaderProgram, *vao);

        *shaderProgram = BuildShader();
        PrepareGpuData(vbo, vao);
        
        *isInitialized = true;
    }

    const u8 *sdlKeyboardState = GetSdlKeyboardState();
    if (sdlKeyboardState[SDL_SCANCODE_2])
    {
        printf("Main exe: 2 button\n");
    }

    if (currentFrame % 60 == 0)
    {
        printf("%u, %u, %u\n", *vbo, *vao, *shaderProgram);
    }

    BeginDraw();

    if ((currentFrame / 100) % 2 == 0)
    {
        float triangle[] = {
            -0.5f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            // 0.0f, 1.0f, 0.0f,
            // 0.0f, -1.0f, 0.0f,
            // 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        };
    
        DrawVertices(*shaderProgram, *vbo, *vao, triangle, ArrayCount(triangle) / 3);
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
    
        DrawVertices(*shaderProgram, *vbo, *vao, triangle, ArrayCount(triangle) / 3);
    }

    char title[256];
    sprintf_s(title, "SAV (%d)", currentFrame / 10);
    SetWindowTitle(window, title);

    EndDraw(window);
}
