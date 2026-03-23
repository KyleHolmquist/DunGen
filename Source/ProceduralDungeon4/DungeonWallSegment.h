#pragma once

#include "CoreMinimal.h"
#include "WallTile.h"
#include "DungeonWallSegment.generated.h"

class AStaticMeshActor;

USTRUCT()
struct PROCEDURALDUNGEON4_API FDungeonWallSegment
{
    GENERATED_BODY()

    FDungeonWallSegment()
        : Cell(FIntPoint::ZeroValue)
        , Direction(0)
        , WallActor(nullptr)
        , WallCell(FIntPoint(-1, -1))
    {
    }

    // Grid cell that owns this wall (the floor cell)
    UPROPERTY()
    FIntPoint Cell;

    // 0 = East, 1 = West, 2 = North, 3 = South
    UPROPERTY()
    uint8 Direction = 0;

    // The actual spawned wall actor
    UPROPERTY()
    TWeakObjectPtr<AWallTile> WallActor;

    //Grid coordinates of the wall tile
    FIntPoint WallCell = FIntPoint(-1, -1);
};