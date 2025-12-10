// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonWallSegment.h"
#include "FloorGeneratorBase.h"
#include "BSP_FloorGenerator.generated.h"

USTRUCT(BlueprintType)
struct FBSPLeaf
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Max;

	FBSPLeaf()
		: Min(FIntPoint::ZeroValue)
		, Max(FIntPoint::ZeroValue)
		{}

	FBSPLeaf(FIntPoint InMin, FIntPoint InMax)
	: Min(InMin), Max(InMax)
	{}

	int32 Width() const { return Max.X - Min.X; }
	int32 Height() const { return Max.Y - Min.Y; }
};


UCLASS()
class PROCEDURALDUNGEON4_API ABSP_FloorGenerator : public AFloorGeneratorBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABSP_FloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Size of the whole map in grid cells
	UPROPERTY(EditAnywhere, Category = "BSP")
	FIntPoint MapSize = FIntPoint(40, 40);

	//Minimum leaf size in cells
	UPROPERTY(EditAnywhere, Category = "BSP")
	int32 MinLeafSize = 8;

	//Max recursion depth
	UPROPERTY(EditAnywhere, Category = "BSP")
	int32 MaxDepth = 5;

	//Size of one grid cell in world units (cm)
	UPROPERTY(EditAnywhere, Category = "BSP")
	float TileSize = 100.f;

	//Plane mesh to use for each room floor
	UPROPERTY(EditAnywhere, Category = "BSP")
	UStaticMesh* FloorMesh;

	//Z Offset for the floor
	UPROPERTY(EditAnywhere, Category = "BSP")
	float FloorZ = 0.f;

	// How much to shrink rooms inside each leaf (in grid cells)
	UPROPERTY(EditAnywhere, Category = "Rooms")
	int32 RoomPaddingMin = 1;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	int32 RoomPaddingMax = 3;

	// ---- Walls ----

	//Mesh used for walls
	UPROPERTY(EditAnywhere, Category = "Walls")
	UStaticMesh* WallMesh;
	
	//Height of walls
	UPROPERTY(EditAnywhere, Category = "Walls")
	float WallHeight = 300.f;

	//Thickness of the walls
	UPROPERTY(EditAnywhere, Category = "Walls")
	float WallThickness = 50.f;

	//Length of a single wall segment in grid cells
	UPROPERTY(EditAnywhere, Category = "Walls")
	int32 WallSegmentCells = 1;

	//All spawned Wall segments
	UPROPERTY()
	TArray<FDungeonWallSegment> WallSegments;

	// Which BSP room (leaf) each interior grid cell belongs to
    UPROPERTY()
    TMap<FIntPoint, int32> CellToRoomIndex;

	// -- Doors --
	
	//How many doors to carve out
	UPROPERTY(EditAnywhere, Category = "Doors")
	int32 DefaultDoorCount = 2;

	//How many doors each room should have
	UPROPERTY(EditAnywhere, Category = "Doors")
	int32 DoorsPerRoom = 2;

	//Mesh for the Doors
	UPROPERTY(EditAnywhere, Category = "Doors")
	UStaticMesh* DoorMesh = nullptr;

	//Width of a door opening in world units
	UPROPERTY(EditAnywhere, Category = "Doors")
	float DoorWidth = 200.f;

	//Random seed for reproducibility
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 Seed = 12345;


private:
	//All leaf regions after BSP split
	
	UPROPERTY()
	TArray<FBSPLeaf> LeafRegions;

	void GenerateBSP();
	void SplitSpace(const FBSPLeaf& Region, int32 Depth);

	void SpawnFloorPlanes();

	//Spawn both interior and exterior doors
	void CreateDoors(int32 DoorCount);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
