#include "sav_lib.h"
#include "savt_game.h"

#include "va_util.h"
#include "va_types.h"
#include "va_memarena.h"
#include "va_linmath.h"
#include "va_colors.h"
#include "va_rect.h"

#include <sdl2/SDL_scancode.h>
#include <sdl2/SDL_mouse.h>
#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>

#include <cstdio>

static_g int gWorldWidth = 100;
static_g int gWorldHeight = 100;

static_g u8 gWorldWalls[] = {
    '#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','#','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','#','.','.','.','.','.','.','.','.','#','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','#','#','.','.','.','.','.','#','#','#','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','#','.','.','#','#','#','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','#','#','#','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','#','#','#','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','#','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','#','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#',
    '#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',
};

#if 1
static_g u8 gWorldTiles[] = {
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 254, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 254, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 
    46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 46, 46, 
    46, 46, 254, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 254, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 254, 46, 46, 
    46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 254, 46, 46, 46, 254, 254, 46, 46, 46, 46, 254, 46, 46, 46, 247, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 247, 247, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 247, 247, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 254, 254, 46, 46, 247, 247, 247, 247, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 247, 247, 247, 247, 247, 247, 46, 46, 46, 46, 
    46, 46, 254, 46, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 247, 247, 247, 247, 247, 247, 247, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 247, 247, 247, 247, 247, 247, 46, 46, 46, 
    46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 254, 46, 46, 46, 46, 46, 247, 247, 247, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
};
#else
static_g u8 gWorldTiles[] = {
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
};
#endif

enum { DARKNESS_UNSEEN = 255, DARKNESS_SEEN = 180, DARKNESS_IN_VIEW = 0 };

void
UpdateCameraToWorldTarget(camera_2d *Camera, world World, vec2i WorldP)
{
    f32 TargetPxX = (f32) WorldP.X * World.TilePxW + World.TilePxW / 2.0f;
    f32 TargetPxY = (f32) WorldP.Y * World.TilePxH + World.TilePxH / 2.0f;
    Camera->Target = Vec2(TargetPxX, TargetPxY);
}

inline b32
IsPInBounds(world *World, vec2i P)
{
    return (P.X >= 0 && P.X < World->Width && P.Y >= 0 && P.Y < World->Height);
}

#include "savt_world.cpp"

sav_texture
GenerateVignette(memory_arena *TrArenaA)
{
    int VigDim = 512;
    MemoryArena_Freeze(TrArenaA);
    u32 *VigData = MemoryArena_PushArray(TrArenaA, VigDim*VigDim, u32);
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
    
    MemoryArena_Unfreeze(TrArenaA);

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

            ClearBackground(VA_BLACK);
            
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            for (int GroundVariant = 1; GroundVariant <= 3; GroundVariant++)
            {
                MemoryArena_Freeze(&GameState->TrArenaA);
                
                int TileCount = GameState->World.Width * GameState->World.Height;
                vec3 *VertPositions = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec3);
                vec4 *VertTexCoords = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec4);
                vec4 *VertColors = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec4);
                int CurrentVert = 0;
                u32 *VertIndices = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 6, u32);
                int CurrentIndex = 0;

                for (int WorldI = 0; WorldI < GameState->World.Width * GameState->World.Height; WorldI++)
                {
                    u8 DarknessLevel = GameState->World.DarknessLevels[WorldI];
                    if (DarknessLevel == DARKNESS_UNSEEN) continue;
                    
                    switch (GameState->World.Tiles[WorldI])
                    {
                       case 254:
                       {
                           if (GroundVariant != 2) continue;
                       } break;

                       case 247:
                       {
                           if (GroundVariant != 3) continue;
                       } break;

                       default:
                       {
                           if (GroundVariant != 1) continue;
                       } break;
                    }
                        
                    vec2i WorldP = IdxToXY(WorldI, GameState->World.Width);
                    rect Dest = GetWorldDestRect(GameState->World, WorldP);

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

                if (CurrentVert > 0 && CurrentIndex > 0)
                {
                    DrawVertices(VertPositions, VertTexCoords, VertColors, VertIndices, CurrentVert, CurrentIndex);
                }

                MemoryArena_Unfreeze(&GameState->TrArenaA);
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

                    MemoryArena_Freeze(&GameState->TrArenaA);
                    
                    vec3 *VertPositions = MemoryArena_PushArray(&GameState->TrArenaA, GameState->GroundPointCount * 4, vec3);
                    vec4 *VertTexCoords = MemoryArena_PushArray(&GameState->TrArenaA, GameState->GroundPointCount * 4, vec4);
                    vec4 *VertColors = MemoryArena_PushArray(&GameState->TrArenaA, GameState->GroundPointCount * 4, vec4);
                    vec4 C = ColorV4(VA_WHITE);
                    for (int i = 0; i < GameState->GroundPointCount * 4; i++)
                    {
                        VertColors[i] = C;
                    }
                    int CurrentVert = 0;
                    u32 *VertIndices = MemoryArena_PushArray(&GameState->TrArenaA, GameState->GroundPointCount * 6, u32);
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

                    MemoryArena_Unfreeze(&GameState->TrArenaA);
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

void
DrawLighting(game_state *GameState)
{
    BeginTextureMode(GameState->LightingRenderTex, Rect(0)); BeginCameraMode(&GameState->Camera); 
    {
        ClearBackground(ColorAlpha(VA_WHITE, 0));

        MemoryArena_Freeze(&GameState->TrArenaA);
        
        int TileCount = GameState->World.Width * GameState->World.Height;
        vec3 *VertPositions = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec3);
        vec4 *VertTexCoords = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec4);
        vec4 *VertColors = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec4);
        int CurrentVert = 0;
        u32 *VertIndices = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 6, u32);
        int CurrentIndex = 0;
        
        for (int WorldI = 0; WorldI < TileCount; WorldI++)
        {
            u8 DarknessLevel = GameState->World.DarknessLevels[WorldI];
            if (GameState->World.DarknessLevels[WorldI] == DARKNESS_SEEN)
            {
                vec2i WorldP = IdxToXY(WorldI, GameState->World.Width);
                rect Dest = GetWorldDestRect(GameState->World, WorldP);

                vec3 Positions[4];
                RectGetPoints(Dest, Positions);
            
                vec4 ColV = ColorV4(ColorAlpha(VA_BLACK, DarknessLevel));

                int BaseVert = CurrentVert;
                for (int i = 0; i < ArrayCount(Positions); i++)
                {
                    VertPositions[CurrentVert] = Positions[i];
                    VertColors[CurrentVert] = ColV;
                    CurrentVert++;
                }

                u32 Indices[] = { 0, 1, 2, 2, 3, 0 };
                
                for (int i = 0; i < ArrayCount(Indices); i++)
                {
                    VertIndices[CurrentIndex++] = Indices[i] + BaseVert;
                }
            }
        }

        if (CurrentVert > 0 && CurrentIndex > 0)
        {
            DrawVertices(VertPositions, VertTexCoords, VertColors, VertIndices, CurrentVert, CurrentIndex);
        }

        MemoryArena_Unfreeze(&GameState->TrArenaA);
    }
    EndCameraMode(); EndTextureMode();
}

void
DrawEntities(game_state *GameState)
{
    BeginDraw();
    {
        // NOTE: Draw entities
        BeginCameraMode(&GameState->Camera);
        {
            MemoryArena_Freeze(&GameState->TrArenaA);

            int TileCount = GameState->World.Width * GameState->World.Height;
            vec3 *VertPositions = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec3);
            vec4 *VertTexCoords = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec4);
            vec4 *VertColors = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 4, vec4);
            int CurrentVert = 0;
            u32 *VertIndices = MemoryArena_PushArray(&GameState->TrArenaA, TileCount * 6, u32);
            int CurrentIndex = 0;

            for (int WorldI = 0; WorldI < TileCount; WorldI++)
            {
                u8 DarknessLevel = GameState->World.DarknessLevels[WorldI];
                if (DarknessLevel != DARKNESS_UNSEEN)
                {
                    entity *Entity = GameState->World.SpatialEntities[WorldI];
                    // TODO: Draw the top npc entity, if none, draw the top item pickup entity
                    if (Entity)
                    {
                        if (DarknessLevel == DARKNESS_IN_VIEW || (Entity->Type != ENTITY_NPC))
                        {
                            vec2i WorldP = IdxToXY(WorldI, GameState->World.Width);
                            rect Dest = GetWorldDestRect(GameState->World, WorldP);
                            rect Source = GetGlyphSourceRect(GameState->GlyphAtlas, Entity->Glyph);

                            vec3 Positions[4];
                            RectGetPoints(Dest, Positions);
                            vec2 TexCoords[4];
                            GetTexCoordsForTex(GameState->GlyphAtlas.T, Source, TexCoords);
                            vec4 TexCoordsV4[4];
                            for (int i = 0; i < ArrayCount(TexCoords); i++)
                            {
                                TexCoordsV4[i] = Vec4(TexCoords[i], 0, 0);
                            }
                            vec4 ColV = ColorV4(Entity->Color);

                            int BaseVert = CurrentVert;
                            for (int i = 0; i < ArrayCount(Positions); i++)
                            {
                                VertPositions[CurrentVert] = Positions[i];
                                VertTexCoords[CurrentVert] = TexCoordsV4[i];
                                VertColors[CurrentVert] = ColV;
                                CurrentVert++;
                            }

                            u32 Indices[] = { 0, 1, 2, 2, 3, 0 };
                
                            for (int i = 0; i < ArrayCount(Indices); i++)
                            {
                                VertIndices[CurrentIndex++] = Indices[i] + BaseVert;
                            }
                        }
                    }
                }
            }

            if (CurrentVert > 0 && CurrentIndex > 0)
            {
                BindTextureSlot(0, GameState->GlyphAtlas.T);
                DrawVertices(VertPositions, VertTexCoords, VertColors, VertIndices, CurrentVert, CurrentIndex);
                UnbindTextureSlot(0);
            }

            MemoryArena_Unfreeze(&GameState->TrArenaA);
        }
        EndCameraMode();
    }
    EndDraw();
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

        GameState->WorldArena = MemoryArenaNested(&GameState->RootArena, Megabytes(32));
        GameState->ResourceArena = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        GameState->TrArenaA = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        GameState->TrArenaB = MemoryArenaNested(&GameState->RootArena, Megabytes(16));

        TraceLog("Root arena used mem: %zu/%zu MB", GameState->RootArena.Used / 1024 / 1024, GameState->RootArena.Size / 1024 / 1024);

        GameState->GroundShader = BuildCustomShader("res/ground.vs", "res/ground.fs");
        BeginShaderMode(GameState->GroundShader);
        {
            SetUniformI("sprite", 1);
            SetUniformI("vig", 2);
        }
        EndShaderMode();
        
        GameState->Camera.Rotation = 0.0f;
        CameraInitLogZoomSteps(&GameState->Camera, 0.2f, 5.0f, 5);
        GameState->Camera.Offset = GetWindowSize() / 2.0f;

        GameState->UiRect = Rect(GetWindowSize());
        GameState->UiRenderTex = SavLoadRenderTexture((int) GetWindowOrigSize().X, (int) GetWindowOrigSize().Y, true);
        GameState->LightingRenderTex = SavLoadRenderTexture((int) GetWindowSize().X, (int) GetWindowSize().Y, false);
        GameState->DebugOverlay = SavLoadRenderTexture((int) GetWindowSize().X, (int) GetWindowSize().Y, false);
        
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/ProtestStrike-Regular.ttf", 32);
        GameState->GlyphAtlas.T = SavLoadTexture("res/NewFontTest.png");
        GameState->GlyphAtlas.GlyphsPerRow = 16;
        GameState->GlyphAtlas.GlyphPadX = 1;
        GameState->GlyphAtlas.GlyphPadY = 1;
        GameState->GlyphAtlas.GlyphPxW = 42;
        GameState->GlyphAtlas.GlyphPxH = 54;
        GameState->GlyphAtlas.GlyphPxWPad = GameState->GlyphAtlas.GlyphPxW + 2 * GameState->GlyphAtlas.GlyphPadX;
        GameState->GlyphAtlas.GlyphPxHPad = GameState->GlyphAtlas.GlyphPxH + 2 * GameState->GlyphAtlas.GlyphPadY;
        GameState->VigTex = GenerateVignette(&GameState->TrArenaA);
        GameState->GroundBrushTex = SavLoadTexture("res/GroundBrushes5.png");
        GameState->GroundBrushRect = Rect(GameState->GroundBrushTex.Width, GameState->GroundBrushTex.Width);
        GameState->StoneWallTex = SavLoadTexture("res/PurgStoneWall2.png");
        SavSetTextureWrapMode(GameState->StoneWallTex, SAV_CLAMP_TO_EDGE);

        GenerateWorld(GameState);

        UpdateCameraToWorldTarget(&GameState->Camera, GameState->World, GameState->PlayerEntity->Pos);

        int GroundPointsWidth = 32;
        int GroundPointsHeight = 40;
        GameState->GroundPointCount = GroundPointsWidth * GroundPointsHeight;
        GameState->GroundPoints = MemoryArena_PushArray(&GameState->WorldArena, GameState->GroundPointCount, vec2);
        GameState->GroundRots = MemoryArena_PushArray(&GameState->WorldArena, GameState->GroundPointCount, vec2);

        f32 WorldPxWidth = (f32) GameState->World.TilePxW * GameState->World.Width;
        f32 WorldPxHeight = (f32) GameState->World.TilePxH * GameState->World.Height;
        f32 GroundPointDistX = WorldPxWidth / (GroundPointsWidth - 1);
        f32 GroundPointDistY = WorldPxHeight / (GroundPointsHeight - 1);
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

        Noop;
    }

    // SECTION: First updates

    MemoryArena_Reset(&GameState->TrArenaA);
    if (WindowSizeChanged())
    {
        GameState->Camera.Offset = GetWindowSize() / 2.0f;
        GameState->UiRect = Rect(GetWindowSize());
        SavDeleteRenderTexture(&GameState->LightingRenderTex);
        GameState->LightingRenderTex = SavLoadRenderTexture((int) GetWindowSize().X, (int) GetWindowSize().Y, false);
        SavDeleteRenderTexture(&GameState->DebugOverlay);
        GameState->DebugOverlay = SavLoadRenderTexture((int) GetWindowSize().X, (int) GetWindowSize().Y, false);
    }

    // SECTION: Check inputs

    if (KeyPressed(SDL_SCANCODE_F11)) ToggleWindowBorderless();
    
    if (MouseWheel() != 0) CameraIncreaseLogZoomSteps(&GameState->Camera, MouseWheel());
    if (MouseDown(SDL_BUTTON_MIDDLE)) GameState->Camera.Target -= CameraScreenToWorldRel(&GameState->Camera, GetMouseRelPos());

    vec2i RequestedPlayerDP = Vec2I();
    b32 RequestedPlayerTurnSkip = false;
    if (KeyPressedOrRepeat(SDL_SCANCODE_Q)) RequestedPlayerDP = Vec2I(-1, -1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_W)) RequestedPlayerDP = Vec2I( 0, -1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_E)) RequestedPlayerDP = Vec2I( 1, -1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_A)) RequestedPlayerDP = Vec2I(-1,  0);
    if (KeyPressedOrRepeat(SDL_SCANCODE_S)) RequestedPlayerDP = Vec2I( 0,  1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_D)) RequestedPlayerDP = Vec2I( 1,  0);
    if (KeyPressedOrRepeat(SDL_SCANCODE_Z)) RequestedPlayerDP = Vec2I(-1,  1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_C)) RequestedPlayerDP = Vec2I( 1,  1);
    if (KeyPressedOrRepeat(SDL_SCANCODE_X)) RequestedPlayerTurnSkip = true;

    // SECTION: Game logic

    b32 PlayerTookTurn = false;
    if (RequestedPlayerTurnSkip)
    {
        PlayerTookTurn = true;
    }
    else if (RequestedPlayerDP.X != 0 || RequestedPlayerDP.Y != 0)
    {
        vec2i NewP = GameState->PlayerEntity->Pos + RequestedPlayerDP;
        if (MoveEntity(&GameState->World, GameState->PlayerEntity, NewP))
        {
            PlayerTookTurn = true;
            UpdateCameraToWorldTarget(&GameState->Camera, GameState->World, NewP);
        }
    }

    if (KeyPressed(SDL_SCANCODE_B))
    {
        Breakpoint;
    }

    // TODO: Optimize when we recalculate field of view
    // TODO: Optimize by spinning through entities and making a map of occupied tiles before hand
    // TODO: Optimize by only calculating entity fov if player is in (some) range
    for (int i = 0; i < GameState->World.EntityUsedCount; i++)
    {
        entity *Entity = GameState->World.Entities + i;
        if ((Entity->Type == ENTITY_PLAYER || Entity->Type == ENTITY_NPC) && Entity->FieldOfView != NULL && Entity->ViewRange > 1)
        {
            memset(Entity->FieldOfView, 0, GameState->World.Width * GameState->World.Height * sizeof(Entity->FieldOfView[0]));
            CalculateLineOfSight(&GameState->World, Entity->Pos, Entity->FieldOfView, Entity->ViewRange);
        }
    }

    for (int i = 0; i < GameState->World.Width * GameState->World.Height; i++)
    {
        if (GameState->World.DarknessLevels[i] == DARKNESS_IN_VIEW)
        {
            GameState->World.DarknessLevels[i] = DARKNESS_SEEN;
        }

        // if (GameState->PlayerEntity->FieldOfView == 0 || GameState->PlayerEntity->FieldOfView[i] == 1)
        {
            GameState->World.DarknessLevels[i] = DARKNESS_IN_VIEW;
        }
    }

    if (PlayerTookTurn)
    {
        for (int i = 0; i < GameState->World.EntityUsedCount; i++)
        {
            entity *Entity = GameState->World.Entities + i;
            if (Entity->Type == ENTITY_NPC)
            {
#if 0
                int ShouldMove = GetRandomValue(0, 12); if (ShouldMove >= 6) {int RandDir = GetRandomValue(0, 4); vec2i NewEntityP = Entity->Pos; switch (RandDir) {case 0: {NewEntityP += Vec2I(0, -1);} break; case 1: {NewEntityP += Vec2I(1, 0);} break; case 2: {NewEntityP += Vec2I(0, 1);} break; case 3: {NewEntityP += Vec2I(-1, 0);} break; default: break;} MoveEntity(&GameState->World, Entity, NewEntityP);}
#elif 1
                if (Entity->FieldOfView != NULL && IsInFOV(&GameState->World, Entity->FieldOfView, GameState->PlayerEntity->Pos))
                {
                    path_result Path = CalculatePath(&GameState->World,
                                                     Entity->Pos, GameState->PlayerEntity->Pos,
                                                     &GameState->TrArenaA, &GameState->TrArenaB,
                                                     0);

                    if (Path.FoundPath && Path.Path)
                    {
                        vec2i NewEntityP = Path.Path[0];
                        MoveEntity(&GameState->World, Entity, NewEntityP);
                    }
                }
#endif
            }
        }
    }

    // SECTION: End of frame logic

#ifdef SAV_DEBUG    
    entity **CharEntities;
    int Count;
    GetAllCharacterEntities(&GameState->World, &GameState->TrArenaA, &CharEntities, &Count);

    Noop;

    Assert(ValidateEntitySpatialPartition(&GameState->World));
#endif
    
    for (int i = 0; i < GameState->World.EntityUsedCount; i++)
    {
        entity *Entity = GameState->World.Entities + i;

        if (EntityExists(Entity) && Entity->Health <= 0.0f)
        {
            DeleteEntity(&GameState->World, Entity);
        }
    }


    vec2 MouseP = GetMousePos();
    vec2 MouseWorldPxP = CameraScreenToWorld(&GameState->Camera, MouseP);
    vec2i MouseTileP = GetTilePFromPxP(&GameState->World, MouseWorldPxP);

    // SECTION: Render
    
    // NOTE: Draw debug overlay
    BeginTextureMode(GameState->DebugOverlay, Rect(0)); BeginCameraMode(&GameState->Camera); 
    {
        ClearBackground(ColorAlpha(VA_WHITE, 0));

        entity *Entity = GetEntitiesAt(&GameState->World, MouseTileP);
        while (Entity)
        {
            if (Entity->FieldOfView)
            {
                for (int i = 0; i < GameState->World.Width * GameState->World.Width; i++)
                {
                    if (Entity->FieldOfView[i] == 1)
                    {
                        DrawRect(&GameState->World, IdxToXY(i, GameState->World.Width), ColorAlpha(VA_BLUE, 150));
                    }
                }
            }
            
            if (Entity->Type == ENTITY_NPC)
            {
                path_result Path = CalculatePath(&GameState->World,
                                                 Entity->Pos, GameState->PlayerEntity->Pos,
                                                 &GameState->TrArenaA, &GameState->TrArenaB,
                                                 0);

                    
                for (int Step = 0; Step < Path.PathSteps; Step++)
                {
                    DrawRect(&GameState->World, Path.Path[Step], ((Step < Path.PathSteps - 1) ? ColorAlpha(VA_YELLOW, 150) : ColorAlpha(VA_RED, 150)));
                }
            }

            Entity = Entity->Next;
        }
    }
    EndCameraMode(); EndTextureMode();

    // NOTE: Draw UI to its own render texture
    BeginTextureMode(GameState->UiRenderTex, GameState->UiRect);
    {
        ClearBackground(ColorAlpha(VA_WHITE, 0));

        DrawString(TextFormat("%0.3f FPS", GetFPSAvg(), GetDeltaAvg()),
                   GameState->Font,
                   GameState->Font->PointSize,
                   10,
                   10,
                   VA_MAROON,
                   true, ColorAlpha(VA_BLACK, 128),
                   &GameState->TrArenaA);

        DrawString(TextFormat("%d, %d", MouseTileP.X, MouseTileP.Y, GameState->World.DarknessLevels[XYToIdx(MouseTileP.X, MouseTileP.Y, GameState->World.Width)]),
                   GameState->Font,
                   GameState->Font->PointSize,
                   10,
                   60,
                   VA_MAROON,
                   true, ColorAlpha(VA_BLACK, 128),
                   &GameState->TrArenaA);

        DrawString(TextFormat("Darkness Level: %d", GameState->World.DarknessLevels[XYToIdx(MouseTileP.X, MouseTileP.Y, GameState->World.Width)]),
                   GameState->Font,
                   GameState->Font->PointSize,
                   10,
                   110,
                   VA_MAROON,
                   true, ColorAlpha(VA_BLACK, 128),
                   &GameState->TrArenaA);

        DrawRect(Rect(1820, 980, 100, 100), VA_AQUAMARINE);
    }
    EndTextureMode();

    DrawLighting(GameState);
    
    DrawGround(GameState);

    DrawEntities(GameState);
    
    BeginDraw();
    {
        // NOTE: Draw overlay render textures: lighting, debug, UI
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ONE, GL_ZERO);
        DrawTexture(GameState->LightingRenderTex.Texture, Rect(GetWindowSize()), VA_WHITE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

        DrawTexture(GameState->DebugOverlay.Texture, Rect(GetWindowSize()), VA_WHITE);

        DrawTexture(GameState->UiRenderTex.Texture, GameState->UiRect, VA_WHITE);
    }
    EndDraw();

    SavSwapBuffers();
}
