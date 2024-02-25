#include <cstring> // memset

static_g vec2i DIRECTIONS[] = {
    Vec2I( 0, -1),
    Vec2I( 1, -1),
    Vec2I( 1,  0),
    Vec2I( 1,  1),
    Vec2I( 0,  1),
    Vec2I(-1,  1),
    Vec2I(-1,  0),
    Vec2I(-1, -1)
};

// SECTION: Entity management and spatial partition

entity *
GetEntitiesAt(world *World, vec2i P)
{
    if(IsPInBounds(World, P))
    {
        int WorldI = XYToIdx(P, World->Width);
        return World->SpatialEntities[WorldI];
    }

    return NULL;
}

void
GetAllCharacterEntities(world *World, memory_arena *TrArena, entity ***CharEntities, int *Count)
{
    *CharEntities = MemoryArena_PushArray(TrArena, World->EntityUsedCount, entity *);
    *Count = 0;
    
    for (int i = 0; i < World->EntityUsedCount; i++)
    {
        entity *Entity = World->Entities + i;
        if (Entity->Type == ENTITY_NPC || Entity->Type == ENTITY_PLAYER)
        {
            (*CharEntities)[(*Count)++] = Entity;
        }
    }
    
    MemoryArena_ResizePreviousPushArray(TrArena, *Count, entity *);
}

collision_info
CheckCollisions(world *World, vec2i P)
{
    collision_info CI;
    
    if (IsPInBounds(World, P))
    {
        entity *HeadEntity = GetEntitiesAt(World, P);

        b32 FoundBlocking = false;

        entity *Entity;
        for (Entity = HeadEntity; Entity; Entity = Entity->Next)
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

        CI.Collided = FoundBlocking;
        CI.Entity = Entity;
    }
    else
    {
        CI.Collided = true;
        CI.Entity = NULL;
        return CI;
    }

    return CI;
}

b32
IsTileOpaque(world *World, vec2i P)
{
    if (IsPInBounds(World, P))
    {
        entity *HeadEntity = GetEntitiesAt(World, P);

        b32 FoundOpaque = false;

        entity *Entity;
        for (Entity = HeadEntity; Entity; Entity = Entity->Next)
        {
            if (EntityExists(Entity) && Entity->Pos == P)
            {
                if (CheckFlags(Entity->Flags, ENTITY_IS_OPAQUE))
                {
                    FoundOpaque = true;
                    break;
                }
            }
        }

        return FoundOpaque;
    }
    else
    {
        return true;
    }
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
    // NOTE: This will hit even if there are spaces based on tight count
    Assert(World->EntityUsedCount < World->EntityMaxCount);

    // TODO: Handle more than a given amount of entities. Need a bucket array or something like that.
    // TODO: Do the assert or allocate another bucket only if entities are tightly packed and it's out of space
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
AddEntity(world *World, vec2i Pos, entity *CopyEntity, memory_arena *WorldArena)
{
    entity *Entity = FindNextFreeEntitySlot(World);

    *Entity = *CopyEntity;
    
    b32 NeedFOV = (Entity->Type == ENTITY_NPC || Entity->Type == ENTITY_PLAYER);
    if (NeedFOV && Entity->FieldOfView == NULL)
    {
        // TODO: Allocate only for the entity max range rect
        Entity->FieldOfView = MemoryArena_PushArray(WorldArena, World->Width * World->Height, u8);
    }
    
    Entity->Pos = Pos;

    AddEntityToSpatial(World, Pos, Entity);

    return Entity;
}

b32
MoveEntity(world *World, entity *Entity, vec2i NewP)
{
    if (Entity->Type > 0)
    {
        collision_info Col = CheckCollisions(World, NewP);

        if (Col.Collided)
        {
            if (Col.Entity)
            {
                Col.Entity->Health -= 3;
                TraceLog("Entity %p hits entity %p. Remaining health: %f", Entity, Col.Entity, Col.Entity->Health);
                if (Col.Entity->Health <= 0.0f)
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

    return false;
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

// SECTION: World gen

struct room
{
    int X;
    int Y;
    int W;
    int H;
};

vec2i
GenerateRoomMap(world *World, u8 *GeneratedMap, memory_arena *WorldArena)
{
    int TileCount = World->Width * World->Height;
    int RoomsMax = 50;
    int SizeMin = 6;
    int SizeMax = 20;

    MemoryArena_Freeze(WorldArena);

    room *Rooms = MemoryArena_PushArray(WorldArena, RoomsMax, room);
    int RoomCount = 0;

    for (int RoomI = 0; RoomI < RoomsMax; RoomI++)
    {
        room Room;
        Room.W = GetRandomValue(SizeMin, SizeMax);
        Room.H = GetRandomValue(SizeMin, SizeMax);
        Room.X = GetRandomValue(2, World->Width - Room.W - 2);
        Room.Y = GetRandomValue(2, World->Height - Room.H - 2);

        b32 Intersects = false;
        for (int Y = Room.Y - 2; Y < (Room.Y + Room.H + 2); Y++)
        {
            for (int X = Room.X - 2; X < (Room.X + Room.W + 2); X++)
            {
                if (GeneratedMap[XYToIdx(X, Y, World->Width)] > 0)
                {
                    Intersects = true;
                    break;
                }
            }
        }

        if (!Intersects)
        {
            for (int Y = Room.Y; Y < (Room.Y + Room.H); Y++)
            {
                for (int X = Room.X; X < (Room.X + Room.W); X++)
                {
                    GeneratedMap[XYToIdx(X, Y, World->Width)] = 1;
                }
            }

            Rooms[RoomCount++] = Room;
        }
    }

    MemoryArena_ResizePreviousPushArray(WorldArena, RoomCount, room);

    for (int RoomI = 0; RoomI < RoomCount - 1; RoomI++)
    {
        int FilledCount = 0;
        for (int i = 0; i < TileCount; i++)
        {
            if (GeneratedMap[i] == 1)
            {
                FilledCount++;
            }
        }

        room *Room1 = Rooms + RoomI;
        room *Room2 = Rooms + RoomI + 1;
        
        vec2i Room1Center = Vec2I(Room1->X + Room1->W / 2, Room1->Y + Room1->H / 2);
        vec2i Room2Center = Vec2I(Room2->X + Room2->W / 2, Room2->Y + Room2->H / 2);

        vec2i LeftCenter = Room1Center.X < Room2Center.X ? Room1Center : Room2Center;
        vec2i RightCenter = Room1Center.X > Room2Center.X ? Room1Center : Room2Center;

        if (LeftCenter.Y < RightCenter.Y)
        {
            for (int X = LeftCenter.X; X <= RightCenter.X; X++)
            {
                GeneratedMap[XYToIdx(X, LeftCenter.Y, World->Width)] = 1;
            }

            for (int Y = LeftCenter.Y; Y <= RightCenter.Y; Y++)
            {
                GeneratedMap[XYToIdx(RightCenter.X, Y, World->Width)] = 1;
            }
        }
        else
        {
            for (int X = LeftCenter.X; X <= RightCenter.X; X++)
            {
                GeneratedMap[XYToIdx(X, RightCenter.Y, World->Width)] = 1;
            }

            for (int Y = RightCenter.Y; Y <= LeftCenter.Y; Y++)
            {
                GeneratedMap[XYToIdx(LeftCenter.X, Y, World->Width)] = 1;
            }
        }
        
        int NextFilledCount = 0;
        for (int i = 0; i < TileCount; i++)
        {
            if (GeneratedMap[i] == 1)
            {
                NextFilledCount++;
            }
        }

        // Assert(FilledCount != NextFilledCount);
    }

    for (int TileI = 0; TileI < TileCount; TileI++)
    {
        if (GeneratedMap[TileI] == 0)
        {
            vec2i Current = IdxToXY(TileI, World->Width);
            b32 FoundRoomGround = false;
            for (int Dir = 0; Dir < 8; Dir++)
            {
                vec2i Neighbor = Current + DIRECTIONS[Dir];

                if (GeneratedMap[XYToIdx(Neighbor, World->Width)] == 1)
                {
                    FoundRoomGround = true;
                    break;
                }
            }

            if (FoundRoomGround)
            {
                GeneratedMap[TileI] = 2;
            }
        }
    }

    vec2i Room0Center = Vec2I(Rooms[0].X + Rooms[0].W / 2, Rooms[0].Y + Rooms[0].Y / 2);
 
    MemoryArena_Unfreeze(WorldArena);

    return Room0Center;
}

void
GenerateWorld(game_state *GameState)
{
    world *World = &GameState->World;
    World->Width = gWorldWidth;
    World->Height = gWorldHeight;
    World->TilePxW = GameState->GlyphAtlas.GlyphPxW;
    World->TilePxH = GameState->GlyphAtlas.GlyphPxH;
    
    World->Tiles = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, u8);
    World->DarknessLevels = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, u8);

    World->EntityUsedCount = 0;
    World->EntityMaxCount = ENTITY_MAX_COUNT;
    World->Entities = MemoryArena_PushArray(&GameState->WorldArena, World->EntityMaxCount, entity);
    World->SpatialEntities = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, entity *);

    u8 *GeneratedMap = MemoryArena_PushArrayAndZero(&GameState->WorldArena, World->Width * World->Height, u8);
    vec2i Room0Center = GenerateRoomMap(&GameState->World, GeneratedMap, &GameState->WorldArena);

    // entity W = GetTestEntityBlueprint(ENTITY_STATIC, '#', VA_WHITE);
    // entity G = GetTestEntityBlueprint(ENTITY_STATIC, '@', VA_BLACK);
    // for (int i = 0; i < World->Width * World->Height; i++)
    // {
    //     if (GeneratedMap[i] == 1)
    //     {
    //         AddEntity(&GameState->World, IdxToXY(i, World->Width), &G, &GameState->WorldArena);
    //     }
    //     if (GeneratedMap[i] == 2)
    //     {
    //         AddEntity(&GameState->World, IdxToXY(i, World->Width), &W, &GameState->WorldArena);
    //     }
    // }
    
    for (int i = 0; i < ArrayCount(gWorldTiles); i++)
    {
        World->Tiles[i] = gWorldTiles[i];
    }

    for (int i = 0; i < World->Width * World->Height; i++)
    {
        World->DarknessLevels[i] = DARKNESS_UNSEEN;
    }

    entity WallBlueprint = {};
    WallBlueprint.Type = ENTITY_STATIC;
    WallBlueprint.Color = VA_DARKGRAY;
    WallBlueprint.Glyph = '#';
    WallBlueprint.Health = WallBlueprint.MaxHealth = 100.0f;
    SetFlags(&WallBlueprint.Flags, ENTITY_IS_BLOCKING | ENTITY_IS_OPAQUE);

    #if 0
    for (int X = 0; X < World->Width; X++)
    {
        AddEntity(World, Vec2I(X, 0), &WallBlueprint, &GameState->WorldArena);
        AddEntity(World, Vec2I(X, World->Height - 1), &WallBlueprint, &GameState->WorldArena);
    }

    for (int Y = 1; Y < World->Height - 1; Y++)
    {
        AddEntity(World, Vec2I(0, Y), &WallBlueprint, &GameState->WorldArena);
        AddEntity(World, Vec2I(World->Width - 1, Y), &WallBlueprint, &GameState->WorldArena);
    }
    #elif 0
    for (int i = 0; i < ArrayCount(gWorldWalls); i++)
    {
        if (gWorldWalls[i] == '#')
        {
            vec2i P = IdxToXY(i, World->Width);
            AddEntity(World, P, &WallBlueprint, &GameState->WorldArena);
        }
    }
    #elif 1
    for (int i = 0; i < World->Width * World->Height; i++)
    {
        if (GeneratedMap[i] == 2)
        {
            AddEntity(&GameState->World, IdxToXY(i, World->Width), &WallBlueprint, &GameState->WorldArena);
        }
    }
    #endif

    entity PlayerBlueprint = {};
    PlayerBlueprint.Type = ENTITY_PLAYER;
    PlayerBlueprint.Color = VA_LIGHTBLUE;
    PlayerBlueprint.Glyph = '@';
    PlayerBlueprint.Health = PlayerBlueprint.MaxHealth = 10000.0f;
    PlayerBlueprint.ViewRange = 7;
    SetFlags(&PlayerBlueprint.Flags, ENTITY_IS_BLOCKING);
    GameState->PlayerEntity = AddEntity(World, Room0Center, &PlayerBlueprint, &GameState->WorldArena);
    
    entity EnemyBlueprint = {};
    EnemyBlueprint.Type = ENTITY_NPC;
    EnemyBlueprint.Color = VA_CORAL;
    EnemyBlueprint.Glyph = 1 + 9*16;
    EnemyBlueprint.Health = EnemyBlueprint.MaxHealth = 10.0f;
    EnemyBlueprint.ViewRange = 5;
    SetFlags(&EnemyBlueprint.Flags, ENTITY_IS_BLOCKING);
    
    int AttemptsToAdd = 50;
    for (int i = 0; i < AttemptsToAdd; i++)
    {
        int X = GetRandomValue(0, World->Width);
        int Y = GetRandomValue(0, World->Height);

        vec2i P = Vec2I(X, Y);
        if (!CheckCollisions(World, P).Collided)
        {
            AddEntity(World, P, &EnemyBlueprint, &GameState->WorldArena);
        }
    }
}

// SECTION: Pathing

enum { OPEN_SET_MAX = 1024, PATH_MAX = 512 };

struct path_state
{
    int *OpenSet;
    int OpenSetCount;

    int *CameFrom;
    f32 *GScores;
    f32 *FScores;
    int MapSize;
};

f32
GetHeuristic(vec2i Start, vec2i End)
{
    #if 1
    // NOTE: Sq Dist. Really unadmissible. Fast but very unoptimal path.
    f32 dX = (f32) (End.X - Start.X);
    f32 dY = (f32) (End.Y - Start.Y);
    return dX*dX + dY*dY;
    #elif 0
    // NOTE: Manhattan. Slightly unadmissible. Fast ish, somewhat optimal path.
    return AbsF((f32) ((End.X - Start.X) + (End.Y - Start.Y)));
    #else
    // NOTE: Euclidian distance. Admissible. Slow calculation, way more iterations and guaranteed optimal path.
    f32 dX = (f32) (End.X - Start.X);
    f32 dY = (f32) (End.Y - Start.Y);
    return SqrtF(dX*dX + dY*dY);
    #endif
}

b32
PopLowestScoreFromOpenSet(path_state *PathState, int *LowestScoreIdx)
{
    // NOTE: One of the first optimizations that could be done here is to convert this to a min heap
    if (PathState->OpenSetCount > 0)
    {
        f32 LowestScore = FLT_MAX;
        int LowestScoreOpenSetI = 0;
        for (int OpenSetI = 0; OpenSetI < PathState->OpenSetCount; OpenSetI++)
        {
            int Idx = PathState->OpenSet[OpenSetI];

            if (PathState->FScores[Idx] < LowestScore)
            {
                LowestScore = PathState->FScores[Idx];
                LowestScoreOpenSetI = OpenSetI;
                *LowestScoreIdx = Idx;
            }
        }

        for (int OpenSetI = LowestScoreOpenSetI; OpenSetI < PathState->OpenSetCount - 1; OpenSetI++)
        {
            PathState->OpenSet[OpenSetI] = PathState->OpenSet[OpenSetI + 1];
        }
        PathState->OpenSetCount--;
        return true;
    }

    return false;
}

b32
IsInOpenSet(path_state *PathState, int Idx)
{
    for (int i = 0; i < PathState->OpenSetCount; i++)
    {
        if (PathState->OpenSet[i] == Idx)
        {
            return true;
        }
    }

    return false;
}

void
AddToOpenSet(path_state *PathState, int Idx)
{
    if (!IsInOpenSet(PathState, Idx))
    {
        PathState->OpenSet[PathState->OpenSetCount++] = Idx;
    }
}

void
GetNeighbors(vec2i Pos, world *World,
             vec2i *Neighbors, b32 *DiagonalNeighbors, int *NeighborCount)
{
    *NeighborCount = 0;
    
    for (int Dir = 0; Dir < 8; Dir++)
    {
        vec2i Neighbor = Pos + DIRECTIONS[Dir];

        if (Neighbor.X >= 0 && Neighbor.X < World->Width &&
            Neighbor.Y >= 0 && Neighbor.Y < World->Height)
        {
            DiagonalNeighbors[*NeighborCount] = (Dir % 2 == 1);
            Neighbors[*NeighborCount] = Neighbor;
            (*NeighborCount)++;
        }
    }
}

struct path_result
{
    b32 FoundPath;
    vec2i *Path;
    int PathSteps;
};

path_result
CalculatePath(world *World, vec2i Start, vec2i End, memory_arena *TrArena, memory_arena *ResultArena,  int VizGenMax)
{
    int VizGen = 0;
    path_result Result = {};

    MemoryArena_Freeze(TrArena);

#ifdef VIZ
    if (VizGen >= VizGenMax)
    {
        goto routine_end;
    }
    VizGen++;
#endif

    path_state PathState;
    PathState.OpenSet = MemoryArena_PushArray(TrArena, OPEN_SET_MAX, int);
    PathState.OpenSetCount = 0;

    PathState.MapSize = World->Width * World->Height;
    // NOTE: For unbounded maps, or just big maps, these could be hash tables
    PathState.CameFrom = MemoryArena_PushArray(TrArena, PathState.MapSize, int);
    PathState.GScores = MemoryArena_PushArray(TrArena, PathState.MapSize, f32);
    PathState.FScores = MemoryArena_PushArray(TrArena, PathState.MapSize, f32);
    for (int i = 0; i < PathState.MapSize; i++)
    {
        PathState.GScores[i] = FLT_MAX;
        PathState.FScores[i] = FLT_MAX;
    }

    int StartIdx = XYToIdx(Start, World->Width);
    PathState.OpenSet[PathState.OpenSetCount++] = StartIdx;
    PathState.CameFrom[StartIdx] = 0;
    PathState.GScores[StartIdx] = 0;
    PathState.FScores[StartIdx] = GetHeuristic(Start, End);

    int EndIdx = XYToIdx(End, World->Width);

    if (StartIdx == EndIdx)
    {
        Result.FoundPath = true;
        MemoryArena_Unfreeze(TrArena);
        return Result;
    }

    b32 FoundPath = false;
    int CurrentIdx;
    while (PopLowestScoreFromOpenSet(&PathState, &CurrentIdx))
    {
        if (CurrentIdx == EndIdx)
        {
            FoundPath = true;
            break;
        }

        vec2i Neighbors[8];
        b32 DiagonalNeighbors[8];
        int NeighborCount;
        GetNeighbors(IdxToXY(CurrentIdx, World->Width), World, Neighbors, DiagonalNeighbors, &NeighborCount);

        for (int i = 0; i < NeighborCount; i++)
        {
            vec2i Neighbor = Neighbors[i];
            b32 Diagonal = DiagonalNeighbors[i];
            f32 ThisGScore = PathState.GScores[CurrentIdx] + (Diagonal ? 1.414f : 1.0f);
            
            int NeighborIdx = XYToIdx(Neighbor, World->Width);
            if (ThisGScore < PathState.GScores[NeighborIdx])
            {
                // TODO: This can be optimized by caching results of collisions.
                // Right now if a cell gets rejected because there is a collision, it will recheck collisions again
                // from another currentIdx position.
                // In addition, if a cell has already been discovered with a higher GScore, we know that there was no collision,
                // but this is gonna check collisions for that cell again.
                b32 Collided = (NeighborIdx == EndIdx) ? false : CheckCollisions(World, Neighbor).Collided;
                if (!Collided)
                {
                    PathState.CameFrom[NeighborIdx] = CurrentIdx;
                    PathState.GScores[NeighborIdx] = ThisGScore;
                    PathState.FScores[NeighborIdx] = ThisGScore + GetHeuristic(Neighbor, End);

                    AddToOpenSet(&PathState, NeighborIdx);
                }
            }
        }

#ifdef VIZ
        if (VizGen >= VizGenMax)
        {
            int I = 0;
            int PrevIdx = CurrentIdx;
            while (PrevIdx != StartIdx && I < PATH_MAX)
            {
                DrawRect(World, IdxToXY(PrevIdx, World->Width), I == 0 ? ColorAlpha(VA_YELLOW, 200) : ColorAlpha(VA_BLUE, 100));
                I++;
                PrevIdx = PathState.CameFrom[PrevIdx];
            }
            DrawRect(World, Start, ColorAlpha(VA_RED, 200));

            goto routine_end;
        }
        VizGen++;
#endif
    }

    Result.FoundPath = FoundPath;
    
    if (FoundPath)
    {
        Result.Path = MemoryArena_PushArray(ResultArena, PATH_MAX, vec2i);

        int Count = 0;
        // NOTE: Reconstitute path backwards
        int PrevIdx = EndIdx;
        while (PrevIdx != StartIdx && Count < PATH_MAX)
        {
            Result.Path[Count++] = IdxToXY(PrevIdx, World->Width);
            PrevIdx = PathState.CameFrom[PrevIdx];
        }

        Assert(Count < PATH_MAX);

        // NOTE: And then reverse it
        for (int i = 0; i < Count / 2; i++)
        {
            vec2i Temp = Result.Path[i];
            Result.Path[i] = Result.Path[Count - 1  - i];
            Result.Path[Count - 1  - i] = Temp;
        }
        Result.PathSteps = Count;
        MemoryArena_ResizePreviousPushArray(ResultArena, Count, vec2i);
    }

#ifdef VIZ
 routine_end:
#endif
    MemoryArena_Unfreeze(TrArena);

    return Result;
}

// SECTION: Line of sight

void
TraceLineBresenham(world *World, vec2i A, vec2i B, u8 *VisibilityMap, int MaxRangeSq)
{
    int X1 = A.X;
    int Y1 = A.Y;
    int X2 = B.X;
    int Y2 = B.Y;
    
    int DeltaX = X2 - X1;
    int IX = ((DeltaX > 0) - (DeltaX < 0));
    DeltaX = Abs(DeltaX) << 1;

    int DeltaY = Y2 - Y1;
    int IY = ((DeltaY > 0) - (DeltaY < 0));
    DeltaY = Abs(DeltaY) << 1;

    VisibilityMap[XYToIdx(X1, Y1, World->Width)] = 1;

    if (DeltaX >= DeltaY)
    {
        int Error = (DeltaY - (DeltaY >> 1));

        while (X1 != X2)
        {
            if ((Error > 0) || (!Error && (IX > 0)))
            {
                Error -= DeltaX;
                Y1 += IY;
            }

            Error += DeltaY;
            X1 += IX;

            VisibilityMap[XYToIdx(X1, Y1, World->Width)] = 1;

            if ((X1 - A.X)*(X1 - A.X) + (Y1 - A.Y)*(Y1 - A.Y) >= MaxRangeSq)
            {
                break;
            }

            if (IsTileOpaque(World, Vec2I(X1, Y1)))
            {
                break;
            }
        }
    }
    else
    {
        int Error = (DeltaX - (DeltaY >> 1));

        while (Y1 != Y2)
        {
            if ((Error > 0) || (!Error && (IY > 0)))
            {
                Error -= DeltaY;
                X1 += IX;
            }

            Error += DeltaX;
            Y1 += IY;

            VisibilityMap[XYToIdx(X1, Y1, World->Width)] = 1;

            if ((X1 - A.X)*(X1 - A.X) + (Y1 - A.Y)*(Y1 - A.Y) >= MaxRangeSq)
            {
                break;
            }
            
            if (IsTileOpaque(World, Vec2I(X1, Y1)))
            {
                break;
            }
        }
    }

}

void
CalculateLineOfSight(world *World, vec2i Pos, u8 *VisibilityMap, int MaxRange)
{
    int MaxRangeSq = MaxRange*MaxRange;
    
    for (int X = 0; X < World->Width; X++)
    {
        TraceLineBresenham(World, Pos, Vec2I(X, 0), VisibilityMap, MaxRangeSq);
        TraceLineBresenham(World, Pos, Vec2I(X, World->Height - 1), VisibilityMap, MaxRangeSq);
    }

    for (int Y = 0; Y < World->Height; Y++)
    {
        TraceLineBresenham(World, Pos, Vec2I(0, Y), VisibilityMap, MaxRangeSq);
        TraceLineBresenham(World, Pos, Vec2I(World->Width - 1, Y), VisibilityMap, MaxRangeSq);
    }
}

b32
IsInFOV(world *World, u8 *FieldOfVision, vec2i Pos)
{
    int WorldI = XYToIdx(Pos, World->Width);

    return FieldOfVision[WorldI];
}
