// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonWallSegment.h"
#include "FloorGeneratorBase.h"
#include "Walk_FloorGenerator.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API AWalk_FloorGenerator : public AFloorGeneratorBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWalk_FloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void GenerateModule() override;

	// //Grid Size
    // UPROPERTY(EditAnywhere, Category = "Walker")
	// int32 MapWidth = 60;
	
    // UPROPERTY(EditAnywhere, Category = "Walker")
	// int32 MapHeight = 40;

	//Number of random walk steps to take
    UPROPERTY(EditAnywhere, Category = "Walker")
	int32 NumSteps = 1000;

	//Bool to start in center or random start
    UPROPERTY(EditAnywhere, Category = "Walker")
	bool bStartInCenter = true;

	// //Size of a tile in world units
    // UPROPERTY(EditAnywhere, Category = "Walker")
	// float TileSize = 100.f;

	//Mesh to use for floor cells
    UPROPERTY(EditAnywhere, Category = "Walker")
	UStaticMesh* FloorMesh;

    UPROPERTY(EditAnywhere, Category = "Walker")
	float FloorZ = 0.f;

	//All spawned Wall segments
	UPROPERTY()
	TArray<FDungeonWallSegment> WallSegments;

	//Random seed for reproducibility
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 Seed = 12345;

	// -- Walls --

	//Wall mesh
    UPROPERTY(EditAnywhere, Category = "Walker")
	UStaticMesh* WallMesh;

	//Height offsets

    UPROPERTY(EditAnywhere, Category = "Walker")
	float WallHeight = 200.f;

	// -- Doors --
	
	//How many doors to carve out
	UPROPERTY(EditAnywhere, Category = "Doors")
	int32 DefaultDoorCount = 3;

	//Mesh for the Doors
	UPROPERTY(EditAnywhere, Category = "Doors")
	UStaticMesh* DoorMesh = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//true = wall, false= floor
	TArray<bool> Map;

	FORCEINLINE int32 Index(int32 X, int32 Y) const
	{
		return Y * MapWidth + X;
	};

	void GenerateMap();
	void InitializeMap();
	void RunRandomWalk();
	void SpawnGeometry();
	void CreateDoors(int32 DoorCount);

	bool HasFloorNeighbor(int32 X, int32 Y) const;

	bool IsValidPortalCandidate(int32 WallX, int32 WallY) const;

	FRotator GetPortalFacingRotation(int32 WallX, int32 WallY) const;

};
