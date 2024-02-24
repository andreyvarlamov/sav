#ifndef SAVT_GAME_H
#define SAVT_GAME_H

struct glyph_atlas
{
    sav_texture T;
    int GlyphsPerRow;
    int GlyphPxW;
    int GlyphPxH;
};

enum entity_flags
{
    ENTITY_IS_BLOCKING = 0x1,
    ENTITY_IS_OPAQUE = 0x2,
};

enum entity_type
{
    ENTITY_NONE = 0,
    ENTITY_STATIC,
    ENTITY_NPC,
    ENTITY_PLAYER,
    ENTITY_ITEM_PICKUP,
    ENTITY_TYPE_COUNT
};

struct entity
{
    u8 Type;
    u8 Glyph;
    u8 IsTex;
    color Color;
    sav_texture Tex;
    
    vec2i Pos;
    f32 Health;
    f32 MaxHealth;
    u32 Flags;

    entity *Next;
};

enum { ENTITY_MAX_COUNT = 1000 };

struct world
{
    int Width;
    int Height;
    int TilePxW;
    int TilePxH;

    u8 *Tiles;

    entity *Entities;
    entity **SpatialEntities;
    int EntityTightCount;
    int EntityUsedCount;
    int EntityMaxCount;
};

struct collision_info
{
    b32 Collided;
    entity *Entity;
};

struct game_state
{
    b32 IsInitialized;

    memory_arena RootArena;
    memory_arena WorldArena;
    memory_arena ResourceArena;
    memory_arena TrArenaA;
    memory_arena TrArenaB;

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
    sav_render_texture DebugOverlay;

    world World;

    entity *PlayerEntity;

    vec2 *GroundPoints;
    vec2 *GroundRots;
    int GroundPointCount;
};

inline vec2i IdxToXY(int I, int Width) { return Vec2I(I % Width, I / Width); }
inline int XYToIdx(vec2i P, int Width) { return P.Y * Width + P.X; }
inline int XYToIdx(int X, int Y, int Width) { return Y * Width + X; }
inline b32 CheckFlags(u32 Flags, u32 Mask) { return Flags & Mask; }
inline void SetFlags(u32 *Flags, u32 Mask) { *Flags |= Mask; }
inline void ClearFlags(u32 *Flags, u32 Mask) { *Flags &= ~Mask; }
inline b32 EntityExists(entity *Entity) { return Entity->Type > 0; }

inline rect
GetWorldDestRect(world World, vec2i P)
{
    return Rect(P.X * World.TilePxW, P.Y * World.TilePxH, World.TilePxW, World.TilePxH);
}

inline rect
GetGlyphSourceRect(glyph_atlas Atlas, u8 Glyph)
{
    vec2i P = IdxToXY((int) Glyph, Atlas.GlyphsPerRow);
    return Rect(P.X * Atlas.GlyphPxW, P.Y * Atlas.GlyphPxH, Atlas.GlyphPxW, Atlas.GlyphPxH);
}

inline entity
GetTestEntityBlueprint(entity_type Type, u8 Glyph, color Color)
{
    entity Blueprint = {};
    Blueprint.Type = (u8) Type;
    Blueprint.IsTex = false;
    Blueprint.Color = Color;
    Blueprint.Glyph = Glyph;
    Blueprint.Health = Blueprint.MaxHealth = 10.0f;
    return Blueprint;
}

inline void
DrawRect(world *World, vec2i P, color Color)
{
    f32 X = (f32) P.X * World->TilePxW;
    f32 Y = (f32) P.Y * World->TilePxH;
    rect R = Rect(X, Y, (f32) World->TilePxW, (f32) World->TilePxH);
    DrawRect(R, Color);
}

inline vec2i
GetTilePFromPxP(world *World, vec2 PxP)
{
    vec2i TileP;
    TileP.X = (int) (PxP.X / World->TilePxW);
    TileP.Y = (int) (PxP.Y / World->TilePxH);
    return TileP;
}

inline rect
GetWorldCameraRect(camera_2d *Camera)
{
    vec2 WorldMin = CameraScreenToWorld(Camera, Vec2(0));
    vec2 WorldMax = CameraScreenToWorld(Camera, GetWindowSize());
                                        
    return RectMinMax(WorldMin, WorldMax);
}
    

#endif
