// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonWallSegment.h"
#include "Holmquist_FloorGenerator.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API AHolmquist_FloorGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHolmquist_FloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ---- Config ----

	// -- Floor --

	//Dimensions of the grid in cells
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 GridWidth = 20;
	
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 GridHeight = 20;

	//Target number of floor tiles to carve
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int NumTiles = 50;

	//Size of each tile in world units
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	float TileSize = 400.f;

	//Random seed for reproducibility
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 Seed = 12345;
	
	//Vertical offset for the floor tiles
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	float FloorZ = 0.f;

	//Mesh for the floor
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	UStaticMesh* FloorMesh;

	// -- Walls --

	//Height of the walls in world units
	UPROPERTY(EditAnywhere, Category = "Walls")
	float WallHeight = 300.f;
	
	//Tickness of wall segments in world units
	UPROPERTY(EditAnywhere, Category = "Walls")
	float WallThickness = 40.f;

	//Mesh for the Walls
	UPROPERTY(EditAnywhere, Category = "Walls")
	UStaticMesh* WallMesh;

	// -- Doors --
	
	//How many doors to carve out
	UPROPERTY(EditAnywhere, Category = "Doors")
	int32 DefaultDoorCount = 3;

	//Mesh for the Doors
	UPROPERTY(EditAnywhere, Category = "Doors")
	UStaticMesh* DoorMesh = nullptr;

	//---- Internal Data ----

	//Logical grid - true = floor, false = empty
	TArray<bool> Grid;

	//List of floor cells to grow from
	TArray<FIntPoint> Frontier;

	//All spawned Wall segments
	UPROPERTY()
	TArray<FDungeonWallSegment> WallSegments;

	FORCEINLINE int32 Index(int32 X, int32 Y) const
	{
		return Y * GridWidth + X;
	}

	//---- Pipeline ----

	//Fills the Grid[]
	void GenerateRoomLayout();

	//Spawns floor meshes from the Grid[]
	void SpawnFloorTiles();

	//Spawn the doors
	void CreateDoors(int32 DoorCount);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
