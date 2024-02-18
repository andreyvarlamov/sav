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

    rect uiRect;
    sav_render_texture RTexUI;
    sav_render_texture RTexPxUI;
    
    f32 MapGlyphWidth;
    f32 MapGlyphHeight;

    



    u32 FBO;
    u32 RenderTexture;
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

        GameState->uiRect = GetWindowRect();
        GameState->uiRect.X = 100;
        GameState->uiRect.Y = 100;
        GameState->uiRect.Width = 1500;
        GameState->uiRect.Height = 750;
        GameState->RTexUI = SavLoadRenderTexture(1000, 500, false);
        GameState->RTexPxUI = SavLoadRenderTexture(1000, 500, true);
        // GameState->RTexUI = SavLoadRenderTexture(GetWindowSize().OriginalWidth, GetWindowSize().OriginalHeight, false);
        // GameState->RTexPxUI = SavLoadRenderTexture(GetWindowSize().OriginalWidth, GetWindowSize().OriginalHeight, true);
        
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/ProtestStrike-Regular.ttf", 32);
        // GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/GildaDisplay-Regular.ttf", 32);

        GameState->IsInitialized = true;
    }

    MemoryArena_Reset(&GameState->TransientArena);

    GameState->Camera.Offset = Vec2(GetWindowSize().Width / 2.0f, GetWindowSize().Height / 2.0f);

    if (KeyPressed(SDL_SCANCODE_F11))
    {
        GameState->Borderless = !GameState->Borderless;
        SetWindowBorderless(GameState->Borderless);
    }

    static_p f32 Scale = 1.0f;
    static_p b32 ScaleIncreasing = true;

    if (ScaleIncreasing)
    {
        Scale += 0.1f * (f32) GetDeltaPrev();
        if (Scale >= 1.05f)
        {
            Scale = 1.05f;
            ScaleIncreasing = false;
        }
    }
    else
    {
        Scale -= 0.1f * (f32) GetDeltaPrev();
        if (Scale <= 0.99f)
        {
            Scale = 0.99f;
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

    if(MousePressed(SDL_BUTTON_LEFT))
    {
        vec2 ScreenCoords = Vec2((f32) GetMousePos().X, (f32) GetMousePos().Y);
        vec2 RectCoords = ScreenToRectCoords(GameState->uiRect,
                                             (f32) GameState->RTexUI.Texture.Width,
                                             (f32) GameState->RTexUI.Texture.Height,
                                             ScreenCoords);
        vec2 B2SCoords = RectToScreenCoords(GameState->uiRect,
                                            (f32) GameState->RTexUI.Texture.Width,
                                            (f32) GameState->RTexUI.Texture.Height,
                                            RectCoords);
        
        TraceLog("ScreenCoords: (%f, %f); RectCoords: (%f, %f); B2SCoords: (%f, %f)",
                 ScreenCoords.X, ScreenCoords.Y,
                 RectCoords.X, RectCoords.Y,
                 B2SCoords.X, B2SCoords.Y);
    }

    if (MouseDown(SDL_BUTTON_MIDDLE))
    {
        vec2 SMouseRelP = Vec2((f32) GetMouseRelPos().X, (f32) GetMouseRelPos().Y);

        vec2 WMouseRelP = CameraScreenToWorldRel(&GameState->Camera, SMouseRelP);
         
        GameState->Camera.Target -= WMouseRelP;

        TraceLog("%f, %f", GameState->Camera.Target.X, GameState->Camera.Target.Y);
    }

    static f32 ColorPos = 0.0f;
    static b32 ColorIncreasing = true;
    f32 ColorSpeed = 0.5f;

    if (ColorIncreasing)
    {
        ColorPos += ColorSpeed * (f32) GetDeltaFixed();
        if (ColorPos >= 1.0f)
        {
            ColorPos = 1.0f;
            ColorIncreasing = false;
        }
    }
    else
    {
        ColorPos -= ColorSpeed * (f32) GetDeltaFixed();
        if (ColorPos <= 0.0f)
        {
            ColorPos = 0.0f;
            ColorIncreasing = true;
        }
    }

    color WatermarkColor = LerpColor(VA_MAROON, VA_ORANGERED, ColorPos);

    BeginTextureMode(GameState->RTexUI);
    {
        ClearBackground(ColorAlpha(VA_WHITE, 255));

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

    BeginTextureMode(GameState->RTexPxUI);
    {
        ClearBackground(ColorAlpha(VA_WHITE, 0));

        // DrawTexture(GameState->Texture, 1780, 990, 0.15f, WatermarkColor);
    }
    EndTextureMode();
    
    BeginDraw();
    {
        ClearBackground(VA_GRAY);
        
        BeginCameraMode(&GameState->Camera);
        {
            DrawRect(Rect(0, 0, 1600, 500), VA_AQUAMARINE);

            DrawTexture(GameState->Texture,
                        Rect(0.0f, 0.0f, (f32) GameState->Texture.Width * Scale, (f32) GameState->Texture.Height * Scale),
                        Rect(GameState->Texture.Width, GameState->Texture.Height),
                        Vec2(GameState->Texture.Width * Scale / 2.0f, GameState->Texture.Height * Scale / 2.0f),
                        0.0f,
                        VA_DARKGREEN);
        }
        EndCameraMode();

        DrawTexture(GameState->RTexUI.Texture, GameState->uiRect);
        DrawTexture(GameState->RTexPxUI.Texture, GameState->uiRect);
    }
    EndDraw();
    
    char Title[256];
    sprintf_s(Title, "SAV %0.3f FPS | %0.9f ms", GetFPSAvg(), GetDeltaAvg());
    SetWindowTitle(Title);
}
