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
    memory_arena ResourceArena;
    memory_arena TransientArena;

    camera_2d Camera;

    sav_font *Font;

    rect uiRect;
    sav_render_texture RTexUI;
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
        GameState->ResourceArena = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        GameState->TransientArena = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        
        GameState->Camera.Rotation = 0.0f;
        CameraInitLogZoomSteps(&GameState->Camera, 0.2f, 5.0f, 5);

        GameState->uiRect = GetWindowRect();
        GameState->RTexUI = SavLoadRenderTexture(GetWindowSize().OriginalWidth, GetWindowSize().OriginalHeight, false);
        
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/ProtestStrike-Regular.ttf", 32);

        GameState->IsInitialized = true;
    }

    MemoryArena_Reset(&GameState->TransientArena);

    if (KeyPressed(SDL_SCANCODE_F11)) ToggleWindowBorderless();
    
    if (MouseWheel() != 0)
    {
        CameraIncreaseLogZoomSteps(&GameState->Camera, MouseWheel());
    }

    if (MouseDown(SDL_BUTTON_MIDDLE))
    {
        GameState->Camera.Target -= CameraScreenToWorldRel(&GameState->Camera, GetMouseRelPos());
    }

    BeginTextureMode(GameState->RTexUI, GameState->uiRect);
    {
        ClearBackground(ColorAlpha(VA_WHITE, 0));

        DrawString(TextFormat("%0.3f FPS", GetFPSAvg(), GetDeltaAvg()),
                   GameState->Font,
                   GameState->Font->PointSize,
                   10,
                   10,
                   VA_MAROON,
                   true, ColorAlpha(VA_BLACK, 128),
                   &GameState->TransientArena);
    }
    EndTextureMode();
    
    BeginDraw();
    {
        ClearBackground(VA_GRAY);
        
        BeginCameraMode(&GameState->Camera);
        {
        }
        EndCameraMode();

        DrawTexture(GameState->RTexUI.Texture, GameState->uiRect, VA_WHITE);
    }
    EndDraw();
}
