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

    sav_texture Texture;
    
    b32 MouseRel;

    b32 Borderless;

    camera_2d Camera;

    sav_font *Font;
    
    f32 MapGlyphWidth;
    f32 MapGlyphHeight;
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
        
        GameState->Texture = SavLoadTexture("res/test.png");

        GameState->Camera.Rotation = 0.0f;
        CameraInitLogZoomSteps(&GameState->Camera, 0.2f, 5.0f, 5);

        // GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/ProtestStrike-Regular.ttf", 32);
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/GildaDisplay-Regular.ttf", 32);
        
        GameState->IsInitialized = true;
    }

    if (Reloaded)
    {
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/GildaDisplay-Regular.ttf", 32);

    }

    MemoryArena_Reset(&GameState->TransientArena);

    GameState->Camera.Offset = Vec2(GetWindowSize().Width / 2.0f, GetWindowSize().Height / 2.0f);

    if (KeyPressed(SDL_SCANCODE_F11))
    {
        GameState->Borderless = !GameState->Borderless;
        SetWindowBorderless(GameState->Borderless);
    }

    static_p f32 Rot = 0.0f;
    static_p f32 Scale = 1.0f;
    static_p b32 ScaleIncreasing = true;

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

    vec2 dP = Vec2();
    if (KeyDown(SDL_SCANCODE_W))
    {
        dP.Y -= 1.0f;
    }
    if (KeyDown(SDL_SCANCODE_S))
    {
        dP.Y += 1.0f;
    }
    if (KeyDown(SDL_SCANCODE_A))
    {
        dP.X -= 1.0f;
    }
    if (KeyDown(SDL_SCANCODE_D))
    {
        dP.X += 1.0f;
    }
    if (dP.X != 0.0f || dP.Y != 0.0f)
    {
        dP = VecNormalize(dP);
    }
    
    f32 MovementSpeed = 1000.0f;
    GameState->Camera.Target += dP * MovementSpeed * (f32) GetDeltaFixed();

#if 0
    if (GetCurrentFrame() % 50 == 0)
    {
        TraceLog("%f, %f", GameState->Camera.Target.X, GameState->Camera.Target.Y);
    }
#endif

    f32 RotSpeed = 100.0f;
    if (KeyDown(SDL_SCANCODE_RIGHT))
    {
        GameState->Camera.Rotation += RotSpeed * (f32) GetDeltaFixed();
    }
    if (KeyDown(SDL_SCANCODE_LEFT))
    {
        GameState->Camera.Rotation -= RotSpeed * (f32) GetDeltaFixed();
    }
    
    if (MouseWheel() != 0)
    {
        CameraIncreaseLogZoomSteps(&GameState->Camera, MouseWheel());
        TraceLog("Changed zoom to: %f", GameState->Camera.Zoom);
    }

    if(MousePressed(1))
    {
        vec2 SMouseP = Vec2((f32) GetMousePos().X, (f32) GetMousePos().Y);
        vec2 WMouseP = CameraScreenToWorld(&GameState->Camera, SMouseP);
        vec2 B2SMouseP = CameraWorldToScreen(&GameState->Camera, WMouseP);
        
        TraceLog("SMouseP: (%f, %f); WMouseP: (%f, %f); B2SMouseP: (%f, %f)",
                 SMouseP.X, SMouseP.Y,
                 WMouseP.X, WMouseP.Y,
                 B2SMouseP.X, B2SMouseP.Y);
    }

    if (MouseDown(2))
    {
        vec2 SMouseRelP = Vec2((f32) GetMouseRelPos().X, (f32) GetMouseRelPos().Y);

        vec2 WMouseRelP = CameraScreenToWorldRel(&GameState->Camera, SMouseRelP);
         
        GameState->Camera.Target -= WMouseRelP;

        TraceLog("%f, %f", GameState->Camera.Target.X, GameState->Camera.Target.Y);
    }

    BeginDraw();
    {
        BeginCameraMode(&GameState->Camera);
        {
            DrawRect(Rect(0, 0, 1600, 500), ColorV4(VA_AQUAMARINE));

            DrawTexture(GameState->Texture,
                        Rect(GetWindowSize().Width / 2.0f, GetWindowSize().Height / 2.0f, (f32) GameState->Texture.Width * Scale, (f32) GameState->Texture.Height * Scale),
                        Rect(GameState->Texture.Width, GameState->Texture.Height),
                        Vec2(GameState->Texture.Width * Scale / 2.0f, GameState->Texture.Height * Scale / 2.0f),
                        Rot,
                        ColorV4(VA_MAROON));
        }
        EndCameraMode();

        DrawString(TextFormat("%0.3f FPS", GetFPSAvg(), GetDeltaAvg()),
                   GameState->Font,
                   GameState->Font->PointSize,
                   10,
                   10,
                   VA_MAROON,
                   true, ColorAlpha(VA_GRAY, 30),
                   &GameState->TransientArena);
    } 
    EndDraw();
    
    char Title[256];
    sprintf_s(Title, "SAV %0.3f FPS | %0.9f ms", GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
