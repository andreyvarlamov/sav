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

struct path_node
{
    vec2i P;
    path_node *Parent;
    f32 AccumCost;
    // f32 SqDistToDest;
};

static_g memory_arena *WorkArena;

enum { PATH_NODE_MAX = 256 };

static_g vec2i VisitedPs[PATH_NODE_MAX];
static_g int VisitedPCount;
static_g path_node *SortedPathNodes[PATH_NODE_MAX];
static_g int SortedPathNodeCount;

void
AddPathNode(vec2i P, path_node *Parent, f32 Cost)
{
    // NOTE: Parent can be null, only if this is the first point in path processed
    Assert(Parent || SortedPathNodeCount == 0);

    // NOTE: If the node for this position already exists, check if the new path to the node is lower cost,
    //       and if so replace it.
    for (int i = 0; i < SortedPathNodeCount; i++)
    {
        path_node *N = SortedPathNodes[i];
        if (N->P == P)
        {
            f32 NewCost = Parent->AccumCost + Cost;
            if (NewCost < N->AccumCost)
            {
                N->Parent = Parent;
                N->AccumCost = NewCost;
            }
            return;
        }
    }

    Assert(SortedPathNodeCount < PATH_NODE_MAX);

    // NOTE: For a yet untraversed P, add a new node and calculate its cost based on its parent's cost
    path_node *NewNode = MemoryArena_PushStruct(WorkArena, path_node);
    NewNode->P = P;
    NewNode->Parent = Parent;
    NewNode->AccumCost = Parent ? (Parent->AccumCost + Cost) : Cost;

    // NOTE: Find a place in the sorted path nodes where this node with a given cost should be
    int NewNodeI;
    for (NewNodeI = SortedPathNodeCount - 1; NewNodeI >= 0; NewNodeI--)
    {
        path_node *N = SortedPathNodes[NewNodeI];

        if (N->AccumCost < NewNode->AccumCost)
        {
            break;
        }
    }
    NewNodeI++;

    // NOTE: Shift all nodes that are greater cost than current one to the right
    for (int i = SortedPathNodeCount - 1; i >= NewNodeI; i--)
    {
        SortedPathNodes[i+1] = SortedPathNodes[i];
    }
    SortedPathNodeCount++;

    // NOTE: Store the pointer to the node in the sorted path nodes
    SortedPathNodes[NewNodeI] = NewNode;
}

b32
NextPathNode(path_node **PathNode)
{
    if (SortedPathNodeCount > 0)
    {
        // NOTE: If there are any path nodes in the priority queue, return the lowest cost one and
        //       shift the rest to the left
        path_node *NextPathNode = SortedPathNodes[0];
        for (int i = 0; i < SortedPathNodeCount - 1; i++)
        {
            SortedPathNodes[i] = SortedPathNodes[i+1];
        }
        SortedPathNodeCount--;

        if (VisitedPCount < PATH_NODE_MAX)
        {
            // NOTE: Set the node position as already visited, so we don't return to it from other tiles
            VisitedPs[VisitedPCount++] = NextPathNode->P;
        }
        else InvalidCodePath;
        

        *PathNode = NextPathNode;
        return true;
    }
    else
    {
        return false;
    }
}

static_g int GenIndex;
static_g int GenMax;

void
CalculateNextDestination(world *World, vec2i A, vec2i B, memory_arena *TransientArena, int _GenMax)
{
    GenIndex = 0;
    GenMax = _GenMax;
    VisitedPCount = 0;
    SortedPathNodeCount = 0;

    WorkArena = TransientArena;
    MemoryArena_Freeze(WorkArena);

    if (GenIndex >= GenMax)
    {
        goto path_end;
    }
    GenIndex++;

    // NOTE: 1. Dijkstra
    AddPathNode(A, NULL, 0);

    path_node *CurrentPathNode = NULL;
    while (NextPathNode(&CurrentPathNode))
    {
        if (CurrentPathNode->P == B)
        {
            break;
        }
        
        for (int Dir = 0; Dir < 8; Dir++)
        {
            vec2i D = DIRECTIONS[Dir];

            vec2i TestP = CurrentPathNode->P + D;

            b32 AlreadyVisited = false;
            for (int i = 0; i < VisitedPCount; i++)
            {
                if (VisitedPs[i] == TestP)
                {
                    AlreadyVisited = true;
                    break;
                }
            }

            if (!AlreadyVisited)
            {
                collision_info Col = CheckCollisions(World, TestP);

                if (!Col.Collided)
                {
                    // NOTE: Cost for diagonals is sqrt(2)
                    AddPathNode(TestP, CurrentPathNode, (Dir % 2 == 0) ? 1.0f : 1.414f);
                }
            }
        }

        if (GenIndex >= GenMax)
        {
            for (int i = 0; i < SortedPathNodeCount; i++)
            {
                DrawRect(World, SortedPathNodes[i]->P, ColorAlpha(VA_BLACK, (u8) (((1.0f - (f32) i / SortedPathNodeCount)) * 255.0f)));
            }
            
            int I = 0;
            path_node *N = CurrentPathNode;
            while (N)
            {
                DrawRect(World, N->P, I == 0 ? ColorAlpha(VA_GREEN, 200) : ColorAlpha(VA_BLUE, 100));
                I++;
                N = N->Parent;
            }

            goto path_end;
        }
        GenIndex++;

        Noop;
    }
    Assert(CurrentPathNode);
    path_node *Destination = CurrentPathNode;

    // NOTE: 2. Trace the path back to start
    path_node *NodeAfterFirst = CurrentPathNode;
    while (CurrentPathNode->Parent)
    {
        NodeAfterFirst = CurrentPathNode;
        CurrentPathNode = CurrentPathNode->Parent;
        DrawRect(World, CurrentPathNode->P, ColorAlpha(VA_RED, 200));
    }
    DrawRect(World, Destination->P, ColorAlpha(VA_GREEN, 200));

    // NOTE: 3. Return the next step
    vec2i NextDest = NodeAfterFirst->P;

 path_end:
    MemoryArena_Unfreeze(WorkArena);
    
    // return NextDest;
}
