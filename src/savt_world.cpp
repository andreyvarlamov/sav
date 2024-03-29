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
    if(IsPValid(P, World))
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
    
    if (IsPValid(P, World))
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
    if (IsPValid(P, World))
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

    Assert(IsPValid(Pos, World));

    entity *HeadEntity = World->SpatialEntities[WorldI];
    Entity->Next = HeadEntity;

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

void EntityTurnQueueInsert(world *World, entity *Entity, int NewCostOwed);

entity *
AddEntity(world *World, vec2i Pos, entity *CopyEntity, memory_arena *WorldArena)
{
    entity *Entity = FindNextFreeEntitySlot(World);

    *Entity = *CopyEntity;
    
    b32 NeedFOV = Entity->Type == ENTITY_PLAYER;
    if (NeedFOV && Entity->FieldOfView == NULL)
    {
        // TODO: Allocate only for the entity max range rect
        Entity->FieldOfView = MemoryArena_PushArray(WorldArena, World->Width * World->Height, u8);
    }
    
    Entity->Pos = Pos;
    Entity->DebugID = World->EntityCurrentDebugID++;

    AddEntityToSpatial(World, Pos, Entity);

    if (Entity->ActionCost > 0)
    {
        EntityTurnQueueInsert(World, Entity, 0);
    }

    return Entity;
}

void
EntityAttacksEntity(entity *Attacker, entity *Defender, world *World)
{
    int AttackRoll = RollDice(1, 20);

    b32 AttackConnects = AttackRoll + Attacker->Kitrina > Defender->ArmorClass;

    if (AttackConnects)
    {
        int DamageValue = RollDice(1, Attacker->Damage) + Max(0, (Attacker->Haima - 5) / 2);
        Defender->Health -= DamageValue;

        if (Defender->Health > 0)
        {
            LogEntityAction(Attacker, World,
                            "%s (%d) hits %s (%d) for %d damage (%d + %d > %d). Health: %d.",
                            Attacker->Name, Attacker->DebugID,
                            Defender->Name, Defender->DebugID,
                            DamageValue, AttackRoll, Attacker->Kitrina, Defender->ArmorClass,
                            Defender->Health);
        }
        else
        {
            LogEntityAction(Attacker, World,
                            "%s (%d) hits %s (%d) for %d damage (%d + %d > %d), killing them.",
                            Attacker->Name, Attacker->DebugID,
                            Defender->Name, Defender->DebugID,
                            DamageValue, AttackRoll, Attacker->Kitrina, Defender->ArmorClass);
        }
    }
    else
    {
        LogEntityAction(Attacker, World,
                        "%s (%d) misses %s (%d) (%d + %d <= %d).",
                        Attacker->Name, Attacker->DebugID,
                        Defender->Name, Defender->DebugID,
                        AttackRoll, Attacker->Kitrina, Defender->ArmorClass);
    }
}

b32
ResolveEntityCollision(entity *ActiveEntity, entity *PassiveEntity, world *World)
{
    switch(ActiveEntity->Type)
    {
        case ENTITY_PLAYER:
        case ENTITY_NPC:
        {
            switch (PassiveEntity->Type)
            {
                case ENTITY_PLAYER:
                case ENTITY_NPC:
                {
                    EntityAttacksEntity(ActiveEntity, PassiveEntity, World);
                    return true;
                } break;

                default: break;
            }
        };
        
        default: break;
    }
    
    return false;
}

b32
MoveEntity(world *World, entity *Entity, vec2i NewP, b32 *Out_TurnUsed)
{
    *Out_TurnUsed = false;
    
    Assert(Entity->Type > 0);
    
    collision_info Col = CheckCollisions(World, NewP);

    if (Col.Collided)
    {
        if (Col.Entity)
        {
            *Out_TurnUsed = ResolveEntityCollision(Entity, Col.Entity, World);
        }

        return false;
    }
    else
    {
        RemoveEntityFromSpatial(World, Entity->Pos, Entity);
        AddEntityToSpatial(World, NewP, Entity);

        Entity->Pos = NewP;
        
#if 0
        TraceLog("%s (%d) moves without hitting anyone", Entity->Name, Entity->DebugID);
#endif
        
        *Out_TurnUsed = true;
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

// SECTION: World gen

struct room
{
    int X;
    int Y;
    int W;
    int H;
};

enum tile_type
{
    TILE_NONE = 0,
    TILE_STONE = 1,
    TILE_GRASS,
    TILE_WATER,
    TILE_COUNT
};

vec2i
GenerateRoomMap(world *World, u8 *GeneratedMap, memory_arena *TrArena)
{
    int TileCount = World->Width * World->Height;

    for (int i = 0; i < TileCount; i++)
    {
        World->Tiles[i] = TILE_WATER;
    }

    int RoomsMax = 50;
    int SizeMin = 6;
    int SizeMax = 20;

    MemoryArena_Freeze(TrArena);

    room *Rooms = MemoryArena_PushArray(TrArena, RoomsMax, room);
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
            b32 GrassRoom = GetRandomValue(0, 2);
            for (int Y = Room.Y; Y < (Room.Y + Room.H); Y++)
            {
                for (int X = Room.X; X < (Room.X + Room.W); X++)
                {
                    int Idx = XYToIdx(X, Y, World->Width);
                    GeneratedMap[Idx] = 1;
                    World->Tiles[Idx] = (u8) (GrassRoom ? TILE_GRASS : TILE_STONE);
                    World->TilesInitialized[Idx] = true;
                }
            }

            Rooms[RoomCount++] = Room;
        }
    }

    MemoryArena_ResizePreviousPushArray(TrArena, RoomCount, room);

    for (int RoomI = 0; RoomI < RoomCount - 1; RoomI++)
    {
        room *Room1 = Rooms + RoomI;
        room *Room2 = Rooms + RoomI + 1;
        
        vec2i Room1Center = Vec2I(Room1->X + Room1->W / 2, Room1->Y + Room1->H / 2);
        vec2i Room2Center = Vec2I(Room2->X + Room2->W / 2, Room2->Y + Room2->H / 2);

        vec2i LeftCenter = Room1Center.X < Room2Center.X ? Room1Center : Room2Center;
        vec2i RightCenter = Room1Center.X >= Room2Center.X ? Room1Center : Room2Center;

        if (LeftCenter.Y < RightCenter.Y)
        {
            for (int X = LeftCenter.X; X <= RightCenter.X; X++)
            {
                int Idx = XYToIdx(X, LeftCenter.Y, World->Width);
                if (GeneratedMap[Idx] == 0)
                {
                    GeneratedMap[Idx] = 1;
                    World->Tiles[Idx] = TILE_STONE;
                    World->TilesInitialized[Idx] = true;
                }
            }

            for (int Y = LeftCenter.Y; Y <= RightCenter.Y; Y++)
            {
                int Idx = XYToIdx(RightCenter.X, Y, World->Width);
                if (GeneratedMap[Idx] == 0)
                {
                    GeneratedMap[Idx] = 1;
                    World->Tiles[Idx] = TILE_STONE;
                    World->TilesInitialized[Idx] = true;
                }
            }
        }
        else
        {
            for (int X = LeftCenter.X; X <= RightCenter.X; X++)
            {
                int Idx = XYToIdx(X, RightCenter.Y, World->Width);
                if (GeneratedMap[Idx] == 0)
                {
                    GeneratedMap[Idx] = 1;
                    World->Tiles[Idx] = TILE_STONE;
                    World->TilesInitialized[Idx] = true;
                }
            }

            for (int Y = RightCenter.Y; Y <= LeftCenter.Y; Y++)
            {
                int Idx = XYToIdx(LeftCenter.X, Y, World->Width);
                if (GeneratedMap[Idx] == 0)
                {
                    GeneratedMap[Idx] = 1;
                    World->Tiles[Idx] = TILE_STONE;
                    World->TilesInitialized[Idx] = true;
                }
            }
        }
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
                World->Tiles[TileI] = TILE_STONE;
                World->TilesInitialized[TileI] = true;
            }
        }
    }

    vec2i Room0Center = Vec2I(Rooms[0].X + Rooms[0].W / 2, Rooms[0].Y + Rooms[0].H / 2);
 
    MemoryArena_Unfreeze(TrArena);

    return Room0Center;
}

#define GENERATED_MAP 1

void
GenerateWorld(game_state *GameState)
{
    world *World = &GameState->World;
    World->Width = 100;
    World->Height = 100;
    World->TilePxW = GameState->GlyphAtlas.GlyphPxW;
    World->TilePxH = GameState->GlyphAtlas.GlyphPxH;

    World->TilesInitialized = MemoryArena_PushArrayAndZero(&GameState->WorldArena, World->Width * World->Height, u8);
    World->Tiles = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, u8);
    World->DarknessLevels = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, u8);
    for (int i = 0; i < World->Width * World->Height; i++)
    {
        World->DarknessLevels[i] = DARKNESS_UNSEEN;
    }

    World->EntityUsedCount = 0;
    World->EntityMaxCount = ENTITY_MAX_COUNT;
    World->Entities = MemoryArena_PushArray(&GameState->WorldArena, World->EntityMaxCount, entity);

    World->SpatialEntities = MemoryArena_PushArray(&GameState->WorldArena, World->Width * World->Height, entity *);

    World->TurnQueueCount = 0;
    World->TurnQueueMax = World->EntityMaxCount;
    World->EntityTurnQueue = MemoryArena_PushArray(&GameState->WorldArena, World->TurnQueueMax, entity_queue_node);

    entity PumiceWall = Template_PumiceWall();

#if (GENERATED_MAP == 1)
    
    u8 *GeneratedEntityMap = MemoryArena_PushArrayAndZero(&GameState->TrArenaA, World->Width * World->Height, u8);
    vec2i Room0Center = GenerateRoomMap(World, GeneratedEntityMap, &GameState->TrArenaA);

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

    // NOTE: Add walls
    for (int i = 0; i < World->Width * World->Height; i++)
    {
        if (GeneratedEntityMap[i] == 2)
        {
            AddEntity(&GameState->World, IdxToXY(i, World->Width), &PumiceWall, &GameState->WorldArena);
        }
    }

#else

    vec2i Room0Center = Vec2I(5, 5);

    for (int i = 0; i < World->Width * World->Height; i++)
    {
        World->TilesInitialized[i] = true;
        World->Tiles[i] = TILE_STONE;
    }

    for (int X = 0; X < World->Width; X++)
    {
        AddEntity(World, Vec2I(X, 0), &PumiceWall, &GameState->WorldArena);
        AddEntity(World, Vec2I(X, World->Height - 1), &PumiceWall, &GameState->WorldArena);
    }

    for (int Y = 1; Y < World->Height - 1; Y++)
    {
        AddEntity(World, Vec2I(0, Y), &PumiceWall, &GameState->WorldArena);
        AddEntity(World, Vec2I(World->Width - 1, Y), &PumiceWall, &GameState->WorldArena);
    }
    
#endif

    entity Player = Template_Player();
    World->PlayerEntity = AddEntity(World, Room0Center, &Player, &GameState->WorldArena);

    entity AetherFly = Template_AetherFly();
#if 1
    int EnemyCount = 0;
    int AttemptCount = 0;
    int EnemiesToAdd = 150;
    int MaxAttempts = 500;
    while (EnemyCount < EnemiesToAdd && AttemptCount < MaxAttempts)
    {
        int X = GetRandomValue(0, World->Width);
        int Y = GetRandomValue(0, World->Height);

        vec2i P = Vec2I(X, Y);
        if (!CheckCollisions(World, P).Collided)
        {
            AddEntity(World, P, &AetherFly, &GameState->WorldArena);
            EnemyCount++;
        }
        AttemptCount++;
    }
#else
    int EnemyCount = 0;
    int AttemptCount = 0;
    int EnemiesToAdd = 5;
    int MaxAttempts = 500;
    while (EnemyCount < EnemiesToAdd && AttemptCount < MaxAttempts)
    {
        int X = GetRandomValue(World->PlayerEntity->Pos.X - 10, World->PlayerEntity->Pos.X + 10);
        int Y = GetRandomValue(World->PlayerEntity->Pos.Y - 10, World->PlayerEntity->Pos.Y + 10);

        vec2i P = Vec2I(X, Y);
        if (!CheckCollisions(World, P).Collided)
        {
            AddEntity(World, P, &AetherFly, &GameState->WorldArena);
            EnemyCount++;
        }
        AttemptCount++;
    }
#endif

    TraceLog("Generated world. Added %d enemies in %d attempts.", EnemyCount, AttemptCount);
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
CalculateFOV(world *World, vec2i Pos, u8 *VisibilityMap, int MaxRange)
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

void
CalculateExhaustiveFOV(world *World, vec2i Pos, u8 *VisibilityMap, int MaxRange)
{
    int MaxRangeSq = MaxRange*MaxRange;

    int StartX = Max(Pos.X - MaxRange, 0);
    int EndX = Min(Pos.X + MaxRange, World->Width - 1);
    int StartY = Max(Pos.Y - MaxRange, 0);
    int EndY = Min(Pos.Y + MaxRange, World->Height - 1);
    
    for (int Y = StartY; Y <= EndY; Y++)
    {
        for (int X = StartX; X <= EndX; X++)
        {
            if (!VisibilityMap[XYToIdx(X, Y, World->Width)])
            {
                TraceLineBresenham(World, Pos, Vec2I(X, Y), VisibilityMap, MaxRangeSq);
            }
        }
    }
}

b32
IsInLineOfSight(world *World, vec2i Start, vec2i End, int MaxRange)
{
    if (VecLengthSq(End - Start) <= MaxRange*MaxRange)
    {
        int CurrentX = Start.X;
        int CurrentY = Start.Y;
        int EndX = End.X;
        int EndY = End.Y;
    
        int DeltaX = EndX - CurrentX;
        int IX = ((DeltaX > 0) - (DeltaX < 0));
        DeltaX = Abs(DeltaX) << 1;

        int DeltaY = EndY - CurrentY;
        int IY = ((DeltaY > 0) - (DeltaY < 0));
        DeltaY = Abs(DeltaY) << 1;

        if (DeltaX >= DeltaY)
        {
            int Error = (DeltaY - (DeltaY >> 1));
            while (CurrentX != EndX)
            {
                if ((Error > 0) || (!Error && (IX > 0)))
                {
                    Error -= DeltaX;
                    CurrentY += IY;
                }

                Error += DeltaY;
                CurrentX += IX;

                vec2i TestPos = Vec2I(CurrentX, CurrentY);
                if (TestPos == End) return true;
                if (IsTileOpaque(World, TestPos)) break;
            }
        }
        else
        {
            int Error = (DeltaX - (DeltaY >> 1));
            while (CurrentY != EndY)
            {
                if ((Error > 0) || (!Error && (IY > 0)))
                {
                    Error -= DeltaY;
                    CurrentX += IX;
                }

                Error += DeltaX;
                CurrentY += IY;

                vec2i TestPos = Vec2I(CurrentX, CurrentY);
                if (TestPos == End) return true;
                if (IsTileOpaque(World, TestPos)) break;
            }
        }
    }

    return false;
}

// SECTION: Entity Turn Queue

inline entity_queue_node
MakeEntityQueueNode(entity *Entity, int Cost)
{
    entity_queue_node Node;
    Node.Entity = Entity;
    Node.LeftoverCost = Cost;
    return Node;
}

inline entity *
EntityTurnQueuePeek(world *World)
{
    return World->EntityTurnQueue->Entity;
}

entity *
EntityTurnQueuePop(world *World)
{
    entity_queue_node *TopNode = World->EntityTurnQueue;

    entity *TopEntity = TopNode->Entity;
    int CostToConsume = TopNode->LeftoverCost;

    for (int I = 0; I < World->TurnQueueCount - 1; I++)
    {
        World->EntityTurnQueue[I] = World->EntityTurnQueue[I + 1];
        World->EntityTurnQueue[I].LeftoverCost -= CostToConsume;
    }
    World->TurnQueueCount--;

    return TopEntity;
}

void
EntityTurnQueueInsert(world *World, entity *Entity, int NewCostOwed)
{
    Assert(World->TurnQueueCount < World->TurnQueueMax);
    int InsertI;
    for (InsertI = World->TurnQueueCount - 1; InsertI >= 0; InsertI--)
    {
        if (World->EntityTurnQueue[InsertI].LeftoverCost <= NewCostOwed)
        {
            break;
        }
    }
    InsertI++;

    for (int ShiftI = World->TurnQueueCount - 1; ShiftI >= InsertI; ShiftI--)
    {
        World->EntityTurnQueue[ShiftI + 1] = World->EntityTurnQueue[ShiftI];
    }
    World->TurnQueueCount++;

    Assert(InsertI >= 0 && InsertI < World->TurnQueueMax);

    World->EntityTurnQueue[InsertI] = MakeEntityQueueNode(Entity, NewCostOwed);
}

int
EntityTurnQueuePopAndReinsert(world *World, int NewCostOwed)
{
    entity *Entity = World->EntityTurnQueue->Entity;
    int CostToConsume = World->EntityTurnQueue->LeftoverCost;
    
    int InsertI;
    for (InsertI = World->TurnQueueCount - 1; InsertI > 0; InsertI--)
    {
        World->EntityTurnQueue[InsertI].LeftoverCost -= CostToConsume;
        if (World->EntityTurnQueue[InsertI].LeftoverCost <= NewCostOwed)
        {
            // NOTE: Set cost back up, because it will be subtracted again in the next for loop
            World->EntityTurnQueue[InsertI].LeftoverCost += CostToConsume;
            break;
        }
    }

    for (int I = 0; I < InsertI; I++)
    {
        World->EntityTurnQueue[I] = World->EntityTurnQueue[I + 1];
        World->EntityTurnQueue[I].LeftoverCost -= CostToConsume;
    }

    Assert(InsertI >= 0 && InsertI < World->TurnQueueMax);

    World->EntityTurnQueue[InsertI] = MakeEntityQueueNode(Entity, NewCostOwed);

    return CostToConsume;
}

void
EntityTurnQueueDelete(world *World, entity *Entity)
{
    int ToDeleteI;
    for (ToDeleteI = 0; ToDeleteI < World->TurnQueueCount; ToDeleteI++)
    {
        if (World->EntityTurnQueue[ToDeleteI].Entity == Entity)
        {
            break;
        }
    }

    for (int ShiftI = ToDeleteI; ShiftI < World->TurnQueueCount - 1; ShiftI++)
    {
        World->EntityTurnQueue[ShiftI] = World->EntityTurnQueue[ShiftI + 1];
    }
    World->TurnQueueCount--;
}
