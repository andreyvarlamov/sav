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

static_g int gWorldWidth = 24;
static_g int gWorldHeight = 24;
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

void
UpdateCameraToWorldTarget(camera_2d *Camera, world World, vec2i WorldP)
{
    f32 TargetPxX = (f32) WorldP.X * World.TilePxW + World.TilePxW / 2.0f;
    f32 TargetPxY = (f32) WorldP.Y * World.TilePxH + World.TilePxH / 2.0f;
    Camera->Target = Vec2(TargetPxX, TargetPxY);
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
                
                int TileCount = GameState->World.Width * GameState->World.Height;
                vec3 *VertPositions = MemoryArena_PushArray(&GameState->TransientArena, TileCount * 4, vec3);
                vec4 *VertTexCoords = MemoryArena_PushArrayAndZero(&GameState->TransientArena, TileCount * 4, vec4);
                vec4 *VertColors = MemoryArena_PushArrayAndZero(&GameState->TransientArena, TileCount * 4, vec4);
                int CurrentVert = 0;
                u32 *VertIndices = MemoryArena_PushArray(&GameState->TransientArena, TileCount * 6, u32);
                int CurrentIndex = 0;

                for (int WorldI = 0; WorldI < GameState->World.Width * GameState->World.Height; WorldI++)
                {
                    switch (GameState->World.Tiles[WorldI])
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

entity *
GetEntitiesAt(world *World, vec2i P)
{
    int WorldI = XYToIdx(P, World->Width);
    return World->SpatialEntities[WorldI];
}

collision_info
CheckCollisions(world *World, vec2i P)
{
    entity *EntityHead = GetEntitiesAt(World, P);

    b32 FoundBlocking = false;

    entity *Entity;
    for (Entity = EntityHead; Entity; Entity = Entity->Next)
    {
        // NOTE: Right now the assumption is that only one entity is blocking per tile
        // (Otherwise how did an entity move to a blocked tile?)
        // if (EntityExists(Entity) && Entity->Pos == P && CheckFlags(Entity->Flags, ENTITY_IS_BLOCKING))
        if (EntityExists(Entity) && Entity->Pos == P)
        {
            if (CheckFlags(Entity->Flags, ENTITY_IS_BLOCKING))
            {
                FoundBlocking = true;
                break;
            }
            else
            {
                Noop;
            }
        }
    }

    Assert(FoundBlocking == (Entity != NULL));
    
    collision_info CI;
    CI.Collided = FoundBlocking;
    CI.Entity = Entity;
    return CI;
}

void
AddEntityToSpatial(world *World, vec2i Pos, entity *Entity)
{
    int WorldI = XYToIdx(Pos, World->Width);

    entity *HeadEntity = World->SpatialEntities[WorldI];
    if (HeadEntity)
    {
        Entity->Next = HeadEntity;
    }

    World->SpatialEntities[WorldI] = Entity;
}

void
RemoveEntityFromSpatial(world *World, vec2i Pos, entity *Entity)
{
    int WorldI = XYToIdx(Pos, World->Width);

    entity *HeadEntity = World->SpatialEntities[WorldI];

    Assert(HeadEntity);

    entity *PrevEntity = NULL;
    entity *SearchEntity = HeadEntity;
    while (SearchEntity)
    {
        if (SearchEntity == Entity)
        {
            break;
        }

        PrevEntity = SearchEntity;
        SearchEntity = SearchEntity->Next;
    }

    Assert(SearchEntity);
        
    if (PrevEntity)
    {
        PrevEntity->Next = Entity->Next;
    }
    else
    {
        World->SpatialEntities[WorldI] = Entity->Next;
    }

    Entity->Next = NULL;
}

entity *
FindNextFreeEntitySlot(world *World)
{
    entity *Entity = World->Entities + World->EntityTightCount;
    while (Entity->Type > 0)
    {
        World->EntityTightCount++;
        Entity++;
    }
    
    World->EntityTightCount++;
    
    entity *NextNextFreeEntity = Entity + 1;
    while (NextNextFreeEntity->Type > 0)
    {
        World->EntityTightCount++;
        NextNextFreeEntity++;
    }

    if (World->EntityTightCount > World->EntityUsedCount)
    {
        World->EntityUsedCount = World->EntityTightCount;
    }

    return Entity;
}

entity *
AddEntity(world *World, vec2i Pos, entity *CopyEntity)
{
    entity *Entity = FindNextFreeEntitySlot(World);
    
    *Entity = *CopyEntity;
    Entity->Pos = Pos;

    AddEntityToSpatial(World, Pos, Entity);

    return Entity;
}

b32
MoveEntity(world *World, entity *Entity, vec2i NewP)
{
    collision_info Col = CheckCollisions(World, NewP);

    if (Col.Collided)
    {
        if (Col.Entity)
        {
            Col.Entity->Health -= 3;
            TraceLog("Entity %p hits entity %p. Remaining health: %f", Entity, Col.Entity, Col.Entity->Health);
            if (Col.Entity->Health < 0.0f)
            {
                TraceLog("Entity %p is dead.", Col.Entity);
            }
        }

        return false;
    }
    else
    {
        RemoveEntityFromSpatial(World, Entity->Pos, Entity);
        AddEntityToSpatial(World, NewP, Entity);

        Entity->Pos = NewP;

        return true;
    }
}

void
DeleteEntity(world *World, entity *Entity)
{
    RemoveEntityFromSpatial(World, Entity->Pos, Entity);

    Entity->Type = ENTITY_NONE;
    
    int EntityI = (int) (Entity - World->Entities);
    Assert(EntityI < World->EntityUsedCount);
    
    if (EntityI < World->EntityTightCount)
    {
        World->EntityTightCount = EntityI;
    }
}

b32
ValidateEntitySpatialPartition(world *World)
{
    for (int i = 0; i < World->Width * World->Height; i++)
    {
        entity *HeadEntity = World->SpatialEntities[i];

        for (entity *Entity = HeadEntity; Entity; Entity = Entity->Next)
        {
            Assert(EntityExists(Entity));
        }
    }

    return true;
}

void
GenerateWorld(game_state *GameState)
{
    world *World = &GameState->World;
    World->Width = gWorldWidth;
    World->Height = gWorldHeight;
    World->TilePxW = GameState->GlyphAtlas.GlyphPxW;
    World->TilePxH = GameState->GlyphAtlas.GlyphPxH;
    
    World->Tiles = MemoryArena_PushArray(&GameState->WorldArena, ArrayCount(gWorldTiles), u8);

    World->EntityUsedCount = 0;
    World->EntityMaxCount = ENTITY_MAX_COUNT;
    World->Entities = MemoryArena_PushArray(&GameState->WorldArena, World->EntityMaxCount, entity);
    World->SpatialEntities = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, entity *);

    for (int i = 0; i < ArrayCount(gWorldTiles); i++)
    {
        GameState->World.Tiles[i] = gWorldTiles[i];
    }

    entity WallBlueprint = {};
    WallBlueprint.Type = ENTITY_STATIC;
    WallBlueprint.IsTex = true;
    WallBlueprint.Tex = GameState->StoneWallTex;
    WallBlueprint.Health = WallBlueprint.MaxHealth = 100.0f;
    SetFlags(&WallBlueprint.Flags, ENTITY_IS_BLOCKING);
        
    for (int X = 0; X < World->Width; X++)
    {
        AddEntity(World, Vec2I(X, 0), &WallBlueprint);
        AddEntity(World, Vec2I(X, World->Height - 1), &WallBlueprint);
    }

    for (int Y = 1; Y < World->Height - 1; Y++)
    {
        AddEntity(World, Vec2I(0, Y), &WallBlueprint);
        AddEntity(World, Vec2I(World->Width - 1, Y), &WallBlueprint);
    }

    entity PlayerBlueprint = {};
    PlayerBlueprint.Type = ENTITY_PLAYER;
    PlayerBlueprint.IsTex = false;
    PlayerBlueprint.Color = VA_LIGHTBLUE;
    PlayerBlueprint.Glyph = '@';
    PlayerBlueprint.Health = PlayerBlueprint.MaxHealth = 30.0f;
    SetFlags(&PlayerBlueprint.Flags, ENTITY_IS_BLOCKING);
    GameState->PlayerEntity = AddEntity(World, Vec2I(1, 1), &PlayerBlueprint);
    
    entity EnemyBlueprint = {};
    EnemyBlueprint.Type = ENTITY_NPC;
    EnemyBlueprint.IsTex = false;
    EnemyBlueprint.Color = VA_CORAL;
    EnemyBlueprint.Glyph = 1 + 9*16;
    EnemyBlueprint.Health = EnemyBlueprint.MaxHealth = 10.0f;
    SetFlags(&EnemyBlueprint.Flags, ENTITY_IS_BLOCKING);
    
    int AttemptsToAdd = 15;
    for (int i = 0; i < AttemptsToAdd; i++)
    {
        int X = GetRandomValue(0, World->Width);
        int Y = GetRandomValue(0, World->Height);

        vec2i P = Vec2I(X, Y);
        if (!CheckCollisions(World, P).Collided)
        {
            AddEntity(World, P, &EnemyBlueprint);
        }
    }

    entity TestBlueprint = GetTestEntityBlueprint(ENTITY_ITEM_PICKUP, 2 + 9*16, VA_PINK);
    AddEntity(World, Vec2I(3, 3), &TestBlueprint);
}

#include "savt_path.cpp"

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

        GameState->VigTex = GenerateVignette(&GameState->TransientArena);
        GameState->GroundBrushTex = SavLoadTexture("res/GroundBrushes4.png");
        GameState->GroundBrushRect = Rect(GameState->GroundBrushTex.Width, GameState->GroundBrushTex.Width);
        GameState->StoneWallTex = SavLoadTexture("res/PurgStoneWall2.png");
        SavSetTextureWrapMode(GameState->StoneWallTex, SAV_CLAMP_TO_EDGE);

        GenerateWorld(GameState);

        UpdateCameraToWorldTarget(&GameState->Camera, GameState->World, GameState->PlayerEntity->Pos);

        int GroundPointsWidth = 10;
        int GroundPointsHeight = 15;
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

        entity E = GetTestEntityBlueprint(ENTITY_STATIC, '$', VA_WHITE);
        entity *AddedE = AddEntity(&GameState->World, Vec2I(5, 5), &E);
        Noop;
    }

    // SECTION: First updates

    MemoryArena_Reset(&GameState->TransientArena);
    GameState->Camera.Offset = GetWindowSize() / 2.0f;

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

    CalculateNextDestination(&GameState->World, Vec2I(1, 1), Vec2I(5, 5), &GameState->TransientArena);

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

    if (PlayerTookTurn)
    {
        for (int i = 0; i < GameState->World.EntityUsedCount; i++)
        {
            entity *Entity = GameState->World.Entities + i;
            if (Entity->Type == ENTITY_NPC)
            {
                int ShouldMove = GetRandomValue(0, 12);
            
                if (ShouldMove >= 6)
                {
                    int RandDir = GetRandomValue(0, 4); 

                    vec2i NewEntityP = Entity->Pos;
                    switch (RandDir)
                    {
                        case 0:
                        {
                            NewEntityP += Vec2I(0, -1);
                        } break;
                
                        case 1:
                        {
                            NewEntityP += Vec2I(1, 0);
                        } break;
                                    
                        case 2:
                        {
                            NewEntityP += Vec2I(0, 1);
                        } break;
                        case 3:
                        {
                            NewEntityP += Vec2I(-1, 0);
                        } break;
                
                        default: break;
                    }

                    MoveEntity(&GameState->World, Entity, NewEntityP);
                }
               
            }
        }
    }

    // SECTION: End of frame logic
    
    for (int i = 0; i < GameState->World.EntityUsedCount; i++)
    {
        entity *Entity = GameState->World.Entities + i;

        if (EntityExists(Entity) && Entity->Health <= 0.0f)
        {
            DeleteEntity(&GameState->World, Entity);
        }
    }

    Assert(ValidateEntitySpatialPartition(&GameState->World));

    // SECTION: Render
    
    DrawGround(GameState);
    
    BeginDraw();
    {
        BeginCameraMode(&GameState->Camera);
        {
            for (int i = 0; i < GameState->World.Width * GameState->World.Height; i++)
            {
                vec2i WorldP = IdxToXY(i, GameState->World.Width);
                rect Dest = GetWorldDestRect(GameState->World, WorldP);

                entity *Entity = GameState->World.SpatialEntities[i];
                if (Entity)
                {
                    if (Entity->IsTex)
                    {
                        DrawTexture(Entity->Tex, Dest, VA_WHITE);
                    }
                    else
                    {
                        rect Source = GetGlyphSourceRect(GameState->GlyphAtlas, Entity->Glyph);
                        DrawTexture(GameState->GlyphAtlas.T, Dest, Source, Entity->Color);
                    }
                }
            }
        }
        EndCameraMode();

        DrawTexture(GameState->RTexUI.Texture, GameState->uiRect, VA_WHITE);
    }
    EndDraw();

    SavSwapBuffers();
}
