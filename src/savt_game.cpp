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

static_g int gMapWidth = 24;
static_g int gMapHeight = 24;
static_g u8 gMapGlyphs[] = {
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 254, 46, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 254, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 35, 
    35, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 46, 35, 
    35, 46, 254, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 254, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 254, 46, 35, 
    35, 46, 46, 46, 46, 254, 254, 254, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 254, 46, 46, 46, 254, 254, 46, 46, 46, 46, 254, 46, 46, 46, 247, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 247, 247, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 247, 247, 46, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 254, 254, 46, 46, 247, 247, 247, 247, 46, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 247, 247, 247, 247, 247, 247, 46, 46, 46, 35, 
    35, 46, 254, 46, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 247, 247, 247, 247, 247, 247, 247, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 247, 247, 247, 247, 247, 247, 46, 46, 35, 
    35, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 254, 46, 46, 46, 46, 46, 247, 247, 247, 46, 46, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 46, 35, 
    35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 
};

struct glyph_atlas
{
    sav_texture T;
    int GlyphsPerRow;
    int GlyphPxW;
    int GlyphPxH;
};

struct map
{
    u8 *Glyphs;
    int *Enemies;
    int Width;
    int Height;
    int TilePxW;
    int TilePxH;
};

struct game_state
{
    b32 IsInitialized;

    memory_arena RootArena;
    memory_arena WorldArena;
    memory_arena ResourceArena;
    memory_arena TransientArena;

    sav_font *Font;
    glyph_atlas GlyphAtlas;

    camera_2d Camera;

    rect uiRect;
    sav_render_texture RTexUI;

    map Map;

    vec2i PlayerP;
};

inline vec2i
IdxToXY(int I, int Width)
{
    return Vec2I(I % Width, I / Width);
}

inline int
XYToIdx(vec2i P, int Width)
{
    return P.Y * Width + P.X;
}

inline rect
GetMapDestRect(map Map, vec2i P)
{
    return Rect(P.X * Map.TilePxW, P.Y * Map.TilePxH, Map.TilePxW, Map.TilePxH);
}

inline rect
GetGlyphSourceRect(glyph_atlas Atlas, u8 Glyph)
{
    vec2i P = IdxToXY((int) Glyph, Atlas.GlyphsPerRow);
    return Rect(P.X * Atlas.GlyphPxW, P.Y * Atlas.GlyphPxH, Atlas.GlyphPxW, Atlas.GlyphPxH);
}

void
UpdateCameraToMapTarget(camera_2d *Camera, map Map, vec2i MapP)
{
    f32 TargetPxX = (f32) MapP.X * Map.TilePxW + Map.TilePxW / 2.0f;
    f32 TargetPxY = (f32) MapP.Y * Map.TilePxH + Map.TilePxH / 2.0f;
    Camera->Target = Vec2(TargetPxX, TargetPxY);
}

struct collision_info
{
    b32 Collided;
    int *Enemy;
};

collision_info
CheckCollisions(map Map, vec2i P)
{
    int MapI = XYToIdx(P, Map.Width);
    u8 Glyph = Map.Glyphs[MapI];
    int *Enemy = Map.Enemies + MapI;
    
    collision_info CI;
    CI.Collided = Glyph == '#' ||  *Enemy > 0;
    CI.Enemy = (*Enemy > 0) ? Enemy : 0;
    return CI;
}

GAME_API void
UpdateAndRender(b32 *Quit, b32 Reloaded, game_memory GameMemory) 
{
    game_state *GameState = (game_state *) GameMemory.Data;

    // SECTION: Init
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

        GameState->uiRect = Rect(GetWindowSize());
        GameState->RTexUI = SavLoadRenderTexture((int) GetWindowOrigSize().X, (int) GetWindowOrigSize().Y, false);
        
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/ProtestStrike-Regular.ttf", 32);
        GameState->GlyphAtlas.T = SavLoadTexture("res/bloody_font.png");
        GameState->GlyphAtlas.GlyphsPerRow = 16;
        GameState->GlyphAtlas.GlyphPxW = 48;
        GameState->GlyphAtlas.GlyphPxH = 72;

        GameState->Map.Glyphs = MemoryArena_PushArray(&GameState->WorldArena, ArrayCount(gMapGlyphs), u8);
        GameState->Map.Enemies = MemoryArena_PushArray(&GameState->WorldArena, ArrayCount(gMapGlyphs), int);
        GameState->Map.Width = gMapWidth;
        GameState->Map.Height = gMapHeight;
        GameState->Map.TilePxW = GameState->GlyphAtlas.GlyphPxW;
        GameState->Map.TilePxH = GameState->GlyphAtlas.GlyphPxH;
        
        for (int i = 0; i < ArrayCount(gMapGlyphs); i++)
        {
            GameState->Map.Glyphs[i] = gMapGlyphs[i];
            
            if (GameState->Map.Glyphs[i] != '#' && GetRandomValue(0,100) < 5)
            {
                GameState->Map.Enemies[i] = 10;
            }
        }

        GameState->PlayerP = Vec2I(1, 1);
        UpdateCameraToMapTarget(&GameState->Camera, GameState->Map, GameState->PlayerP);

        GameState->IsInitialized = true;
    }

    // SECTION: First updates
    MemoryArena_Reset(&GameState->TransientArena);

    GameState->Camera.Offset = GetWindowSize() / 2.0f;

    // SECTION: Check inputs
    if (KeyPressed(SDL_SCANCODE_F11)) ToggleWindowBorderless();
    
    if (MouseWheel() != 0) CameraIncreaseLogZoomSteps(&GameState->Camera, MouseWheel());
    if (MouseDown(SDL_BUTTON_MIDDLE)) GameState->Camera.Target -= CameraScreenToWorldRel(&GameState->Camera, GetMouseRelPos());

    vec2i RequestedPlayerDP = Vec2I();
    if (KeyPressedOrRepeat(SDL_SCANCODE_Q)) RequestedPlayerDP = Vec2I(-1, -1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_W)) RequestedPlayerDP = Vec2I( 0, -1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_E)) RequestedPlayerDP = Vec2I( 1, -1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_A)) RequestedPlayerDP = Vec2I(-1,  0);
    if (KeyPressedOrRepeat(SDL_SCANCODE_S)) RequestedPlayerDP = Vec2I( 0,  1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_D)) RequestedPlayerDP = Vec2I( 1,  0);
    if (KeyPressedOrRepeat(SDL_SCANCODE_Z)) RequestedPlayerDP = Vec2I(-1,  1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_C)) RequestedPlayerDP = Vec2I( 1,  1);

    // SECTION: Game logic
    vec2i NewPlayerP = GameState->PlayerP + RequestedPlayerDP;
    collision_info Col = CheckCollisions(GameState->Map, NewPlayerP);
    if (Col.Collided)
    {
        if (Col.Enemy)
        {
            *Col.Enemy -= 3;
            TraceLog("Player hit enemy. Enemy health: %d", *Col.Enemy);
        }
    }
    else
    {
        GameState->PlayerP = NewPlayerP;
        UpdateCameraToMapTarget(&GameState->Camera, GameState->Map, GameState->PlayerP);
    }

    // SECTION: Render
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
            for (int i = 0; i < GameState->Map.Width * GameState->Map.Height; i++)
            {
                vec2i MapP = IdxToXY(i, GameState->Map.Width);
                rect Dest = GetMapDestRect(GameState->Map, MapP);

                color C;
                u8 Glyph;
                if (MapP == GameState->PlayerP)
                {
                    Glyph = '@';
                    C = VA_LIGHTBLUE;
                }
                else if (GameState->Map.Enemies[i] > 0)
                {
                    Glyph = 1 + 9*16;
                    C = VA_CORAL;
                }
                else
                {
                    Glyph = GameState->Map.Glyphs[i];
                    C = VA_WHITE;
                }
                rect Source = GetGlyphSourceRect(GameState->GlyphAtlas, Glyph);

                DrawTexture(GameState->GlyphAtlas.T, Dest, Source, C);
            }
        }
        EndCameraMode();

        DrawTexture(GameState->RTexUI.Texture, GameState->uiRect, VA_WHITE);
    }
    EndDraw();
}
