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
    sav_texture VigTex;
    sav_texture GroundBrushTex;
    rect GroundBrushRect; // TODO: tex + rect, atlas idiom?
    sav_texture StoneWallTex;
    sav_shader GroundShader;

    camera_2d Camera;

    rect uiRect;
    sav_render_texture RTexUI;

    map Map;

    vec2i PlayerP;

    vec2 *GroundPoints;
    vec2 *GroundRots;
    int GroundPointCount;
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

sav_texture
GenerateVignette(memory_arena *TransientArena)
{
    int VigDim = 512;
    MemoryArena_Freeze(TransientArena);
    u32 *VigData = MemoryArena_PushArray(TransientArena, VigDim*VigDim, u32);
    f32 FadeOutEndR = VigDim / 2.0f;
    f32 FadeOutStartR = FadeOutEndR - 256.0f;
    for (int i = 0; i < VigDim*VigDim; i++)
    {
        vec2i P = IdxToXY(i, VigDim);
            
        u32 *Pixel = VigData + i;

        vec2 PFromCenter = (Vec2(P) + Vec2(0.5f)) - Vec2(VigDim / 2.0f);

        f32 R = SqrtF(PFromCenter.X * PFromCenter.X + PFromCenter.Y * PFromCenter.Y);

        color C;
        if (R > FadeOutEndR)
        {
            C = ColorAlpha(VA_BLACK, 0);
        }
        else if (R > FadeOutStartR && R <= FadeOutEndR)
        {
            f32 T = 1.0f - (R - FadeOutStartR) / (FadeOutEndR - FadeOutStartR);
            f32 A = EaseOutQuad(T);
            C = ColorAlpha(VA_BLACK, (u8) (A * 255.0f));
        }
        else
        {
            C = ColorAlpha(VA_BLACK, 255);
        }
            
        *Pixel = C.C32;
    }
    // SavSaveImage("temp/vig.png", VigData, VigDim, VigDim, false, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

    sav_texture Tex = SavLoadTextureFromData(VigData, VigDim, VigDim);
    SavSetTextureWrapMode(Tex, SAV_CLAMP_TO_EDGE);
    
    MemoryArena_Unfreeze(TransientArena);

    return Tex;
}

void
DrawGround(game_state *GameState)
{
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    BeginDraw();
    {
        BeginCameraMode(&GameState->Camera);
        {
            glStencilMask(0xFF);

            ClearBackground(VA_GRAY);
            
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            for (int GroundVariant = 1; GroundVariant <= 3; GroundVariant++)
            {
                MemoryArena_Freeze(&GameState->TransientArena);
                
                int TileCount = GameState->Map.Width * GameState->Map.Height;
                vec3 *VertPositions = MemoryArena_PushArray(&GameState->TransientArena, TileCount * 4, vec3);
                vec4 *VertTexCoords = MemoryArena_PushArrayAndZero(&GameState->TransientArena, TileCount * 4, vec4);
                vec4 *VertColors = MemoryArena_PushArrayAndZero(&GameState->TransientArena, TileCount * 4, vec4);
                int CurrentVert = 0;
                u32 *VertIndices = MemoryArena_PushArray(&GameState->TransientArena, TileCount * 6, u32);
                int CurrentIndex = 0;

                for (int MapI = 0; MapI < GameState->Map.Width * GameState->Map.Height; MapI++)
                {
                    switch (GameState->Map.Glyphs[MapI])
                    {
                        case '#':
                        case 46:
                        {
                            if (GroundVariant != 1) continue;
                        } break;

                        case 254:
                        {
                            if (GroundVariant != 2) continue;
                        } break;

                        case 247:
                        {
                            if (GroundVariant != 3) continue;
                        } break;

                        default: continue;
                    }
                        
                    vec2i MapP = IdxToXY(MapI, GameState->Map.Width);
                    rect Dest = GetMapDestRect(GameState->Map, MapP);

                    vec3 Positions[4];
                    RectGetPoints(Dest, Positions);
                    u32 Indices[] = { 0, 1, 2, 2, 3, 0 };

                    int BaseVert = CurrentVert;

                    for (int i = 0; i < ArrayCount(Positions); i++)
                    {
                        VertPositions[CurrentVert++] = Positions[i];
                    }
                
                    for (int i = 0; i < ArrayCount(Indices); i++)
                    {
                        VertIndices[CurrentIndex++] = Indices[i] + BaseVert;
                    }
                }

                glStencilFunc(GL_ALWAYS, GroundVariant, 0xFF);
                    
                DrawVertices(VertPositions, VertTexCoords, VertColors, VertIndices, CurrentVert, CurrentIndex);

                MemoryArena_Unfreeze(&GameState->TransientArena);
            }

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        EndCameraMode();
    }
    EndDraw();

    BeginShaderMode(GameState->GroundShader);
    {
        BeginDraw();
        {
            BeginCameraMode(&GameState->Camera);
            {
                glStencilMask(0x00);
                
                BindTextureSlot(1, GameState->GroundBrushTex);
                BindTextureSlot(2, GameState->VigTex);
                
                for (int GroundVariant = 1; GroundVariant <= 3; GroundVariant++)
                {
                    glStencilFunc(GL_EQUAL, GroundVariant, 0xFF);

                    MemoryArena_Freeze(&GameState->TransientArena);
                    
                    vec3 *VertPositions = MemoryArena_PushArray(&GameState->TransientArena, GameState->GroundPointCount * 4, vec3);
                    vec4 *VertTexCoords = MemoryArena_PushArray(&GameState->TransientArena, GameState->GroundPointCount * 4, vec4);
                    vec4 *VertColors = MemoryArena_PushArray(&GameState->TransientArena, GameState->GroundPointCount * 4, vec4);
                    vec4 C = ColorV4(VA_WHITE);
                    for (int i = 0; i < GameState->GroundPointCount * 4; i++)
                    {
                        VertColors[i] = C;
                    }
                    int CurrentVert = 0;
                    u32 *VertIndices = MemoryArena_PushArray(&GameState->TransientArena, GameState->GroundPointCount * 6, u32);
                    int CurrentIndex = 0;
                    
                    for (int GroundPointI = 0; GroundPointI < GameState->GroundPointCount; GroundPointI++)
                    {
                        vec2 P = GameState->GroundPoints[GroundPointI];
                        vec2 Rots = GameState->GroundRots[GroundPointI];
                        f32 Scale = 10.0f;

                        rect Dest = Rect(P.X, P.Y, GameState->GroundBrushRect.Width * Scale, GameState->GroundBrushRect.Height * Scale);
                        Dest.X -= Dest.Width / 2.0f;
                        Dest.Y -= Dest.Height / 2.0f;
    
                        vec3 Positions[4];
                        RectGetPoints(Dest, Positions);
    
                        vec2 TexCoords[4];
                        rect Source = Rect(0.0f, (GroundVariant-1) * GameState->GroundBrushRect.Height, GameState->GroundBrushRect.Width, GameState->GroundBrushRect.Height);
                        RectGetPoints(Source, TexCoords);
                        Rotate4PointsAroundOrigin(TexCoords, RectGetMid(Source), Rots.E[0]);
                        NormalizeTexCoords(GameState->GroundBrushTex, TexCoords);
                        FlipTexCoords(TexCoords);

                        vec2 VigTexCoords[4];
                        rect VigSource = Rect(GameState->VigTex.Width, GameState->VigTex.Height);
                        RectGetPoints(VigSource, VigTexCoords);
                        // Rotate4PointsAroundOrigin(VigTexCoords, RectGetMid(VigSource), Rots.E[1]);
                        NormalizeTexCoords(GameState->VigTex, VigTexCoords);
                        FlipTexCoords(VigTexCoords);

                        vec4 TexCoordsV4[4];
                        for (int i = 0; i < ArrayCount(TexCoords); i++)
                        {
                            TexCoordsV4[i] = Vec4(TexCoords[i].X, TexCoords[i].Y, VigTexCoords[i].X, VigTexCoords[i].Y);
                        }

                        u32 Indices[] = { 0, 1, 2, 2, 3, 0 };

                        int BaseVert = CurrentVert;

                        for (int i = 0; i < ArrayCount(Positions); i++)
                        {
                            VertPositions[CurrentVert] = Positions[i];
                            VertTexCoords[CurrentVert] = TexCoordsV4[i];
                            CurrentVert++;
                        }
                
                        for (int i = 0; i < ArrayCount(Indices); i++)
                        {
                            VertIndices[CurrentIndex++] = Indices[i] + BaseVert;
                        }
                    }

                    DrawVertices(VertPositions, VertTexCoords, VertColors, VertIndices, CurrentVert, CurrentIndex);

                    MemoryArena_Unfreeze(&GameState->TransientArena);
                }

                glStencilMask(0xFF); // NOTE: So that stencil mask can be cleared glClear
            }
            EndCameraMode();
        }
        EndDraw();
    }
    EndShaderMode();

    glDisable(GL_STENCIL_TEST);
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

        GameState->GroundShader = BuildCustomShader("res/ground.vs", "res/ground.fs");
        BeginShaderMode(GameState->GroundShader);
        {
            SetUniformI("sprite", 1);
            SetUniformI("vig", 2);
        }
        EndShaderMode();
        
        GameState->Camera.Rotation = 0.0f;
        CameraInitLogZoomSteps(&GameState->Camera, 0.2f, 5.0f, 5);

        GameState->uiRect = Rect(GetWindowSize());
        GameState->RTexUI = SavLoadRenderTexture((int) GetWindowOrigSize().X, (int) GetWindowOrigSize().Y, true);
        
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

        GameState->VigTex = GenerateVignette(&GameState->TransientArena);
        GameState->GroundBrushTex = SavLoadTexture("res/GroundBrushes4.png");
        GameState->GroundBrushRect = Rect(GameState->GroundBrushTex.Width, GameState->GroundBrushTex.Width);
        GameState->StoneWallTex = SavLoadTexture("res/PurgStoneWall2.png");
        SavSetTextureWrapMode(GameState->StoneWallTex, SAV_CLAMP_TO_EDGE);

        GameState->PlayerP = Vec2I(1, 1);
        UpdateCameraToMapTarget(&GameState->Camera, GameState->Map, GameState->PlayerP);

        int GroundPointsWidth = 10;
        int GroundPointsHeight = 15;
        GameState->GroundPointCount = GroundPointsWidth * GroundPointsHeight;
        GameState->GroundPoints = MemoryArena_PushArray(&GameState->WorldArena, GameState->GroundPointCount, vec2);
        GameState->GroundRots = MemoryArena_PushArray(&GameState->WorldArena, GameState->GroundPointCount, vec2);

        f32 MapPxWidth = (f32) GameState->Map.TilePxW * GameState->Map.Width;
        f32 MapPxHeight = (f32) GameState->Map.TilePxH * GameState->Map.Height;
        f32 GroundPointDistX = MapPxWidth / (GroundPointsWidth - 1);
        f32 GroundPointDistY = MapPxHeight / (GroundPointsHeight - 1);
        for (int i = 0; i < GameState->GroundPointCount; i++)
        {
            vec2i P = IdxToXY(i, GroundPointsWidth);
            f32 PxX = P.X * GroundPointDistX;
            f32 PxY = P.Y * GroundPointDistY;
            f32 RndX = (GetRandomFloat() * 0.1f - 0.05f) * GroundPointDistX;
            f32 RndY = (GetRandomFloat() * 0.1f - 0.05f) * GroundPointDistY;
            GameState->GroundPoints[i] = Vec2(PxX + RndX, PxY + RndY);

            f32 SpriteRot = GetRandomFloat() * 360.0f - 180.0f;
            f32 VigRot = GetRandomFloat() * 360.0f - 180.0f;
            GameState->GroundRots[i] = Vec2(SpriteRot, VigRot);
        }

        GameState->IsInitialized = true;
    }

    if (Reloaded)
    {
        DeleteShader(&GameState->GroundShader);
        GameState->GroundShader = BuildCustomShader("res/ground.vs", "res/ground.fs");
        BeginShaderMode(GameState->GroundShader);
        {
            SetUniformI("sprite", 1);
            SetUniformI("vig", 2);
        }
        EndShaderMode();
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

    // TODO: SHOULD UI BE DRAWN BEFORE GAME LOGIC, SO WE GET DON'T HAVE TO PROCESS BUTTON PRESSES ON THE NEXT FRAME???????
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

    // SECTION: Game logic
    if (RequestedPlayerDP.X != 0 || RequestedPlayerDP.Y != 0)
    {
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
    }

    // f32 RotSpeed = 100.0f;
    // for (int i = 0; i < GameState->GroundPointCount; i++)
    // {
    //     GameState->GroundRots[i].E[0] += (GetRandomFloat() * 5.0f - 2.5f) * RotSpeed * (f32) GetDeltaFixed();
    // }

    // SECTION: Render
    DrawGround(GameState);
    
    BeginDraw();
    {
        BeginCameraMode(&GameState->Camera);
        {
            for (int i = 0; i < GameState->Map.Width * GameState->Map.Height; i++)
            {
                vec2i MapP = IdxToXY(i, GameState->Map.Width);
                rect Dest = GetMapDestRect(GameState->Map, MapP);

                if (GameState->Map.Glyphs[i] == '#')
                {
                    DrawTexture(GameState->StoneWallTex, Dest, VA_WHITE);
                }
                else
                {
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
                        if (Glyph == 46 || Glyph == 254 || Glyph == 247)
                        {
                            continue;
                        }
                        C = VA_WHITE;
                    }
                    rect Source = GetGlyphSourceRect(GameState->GlyphAtlas, Glyph);

                    DrawTexture(GameState->GlyphAtlas.T, Dest, Source, C);
                }
            }
        }
        EndCameraMode();

        DrawTexture(GameState->RTexUI.Texture, GameState->uiRect, VA_WHITE);
    }
    EndDraw();

    SavSwapBuffers();
}
