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

#if 0
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

#include "savt_ground.cpp"
#include "savt_world.cpp"

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
        GameState->TrArenaA = MemoryArenaNested(&GameState->RootArena, Megabytes(16));
        GameState->TrArenaB = MemoryArenaNested(&GameState->RootArena, Megabytes(16));

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
        GameState->DebugOverlay = SavLoadRenderTexture((int) GetWindowSize().X, (int) GetWindowSize().Y, false);
        
        GameState->Font = SavLoadFont(&GameState->ResourceArena, "res/ProtestStrike-Regular.ttf", 32);
        GameState->GlyphAtlas.T = SavLoadTexture("res/bloody_font.png");
        GameState->GlyphAtlas.GlyphsPerRow = 16;
        GameState->GlyphAtlas.GlyphPxW = 48;
        GameState->GlyphAtlas.GlyphPxH = 72;

        GameState->VigTex = GenerateVignette(&GameState->TrArenaA);
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

    MemoryArena_Reset(&GameState->TrArenaA);
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

    // SECTION: Game logic

    static_p int PathGen = 0;

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

    vec2 MouseP = GetMousePos();
    vec2 MouseWorldPxP = CameraScreenToWorld(&GameState->Camera, MouseP);
    vec2i MouseTileP = GetTilePFromPxP(&GameState->World, MouseWorldPxP);

    BeginTextureMode(GameState->DebugOverlay, Rect(0)); BeginCameraMode(&GameState->Camera); 
    {
        ClearBackground(ColorAlpha(VA_WHITE, 0));

        entity *Entity = GetEntitiesAt(&GameState->World, MouseTileP);
        while (Entity)
        {
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

        u8 *VisibilityTest = MemoryArena_PushArrayAndZero(&GameState->TrArenaA, GameState->World.Width * GameState->World.Height, u8);
        CalculateLineOfSight(&GameState->World, GameState->PlayerEntity->Pos, VisibilityTest, 7);

        for (int i = 0; i < GameState->World.Width * GameState->World.Height; i++)
        {
            if (VisibilityTest[i] == 0)
            {
                DrawRect(&GameState->World, IdxToXY(i, GameState->World.Width), ColorAlpha(VA_BLACK, 240));
            }
        }
    }
    EndCameraMode(); EndTextureMode();

    if (PlayerTookTurn)
    {
        for (int i = 0; i < GameState->World.EntityUsedCount; i++)
        {
            entity *Entity = GameState->World.Entities + i;
            if (Entity->Type == ENTITY_NPC)
            {
#if 0
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
#else
                if (VecLengthSq(GameState->PlayerEntity->Pos - Entity->Pos) < 49)
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
                   &GameState->TrArenaA);

        DrawString(TextFormat("%d, %d", MouseTileP.X, MouseTileP.Y),
                   GameState->Font,
                   GameState->Font->PointSize,
                   10,
                   60,
                   VA_MAROON,
                   true, ColorAlpha(VA_BLACK, 128),
                   &GameState->TrArenaA);
                   
    }
    EndTextureMode();

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

        DrawTexture(GameState->DebugOverlay.Texture, Rect(GetWindowSize()), VA_WHITE);
        
        DrawTexture(GameState->RTexUI.Texture, GameState->uiRect, VA_WHITE);
    }
    EndDraw();

    SavSwapBuffers();
}
