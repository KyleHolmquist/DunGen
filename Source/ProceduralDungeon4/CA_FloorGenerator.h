// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonWallSegment.h"
#include "FloorGeneratorBase.h"
#include "CA_FloorGenerator.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API ACA_FloorGenerator : public AFloorGeneratorBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACA_FloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GenerateModule() override;

	// ---- Grid / CA Settings ----

	//Grid size in cells
	UPROPERTY(EditAnywhere, Category = "CA")
	int32 MapWidth = 60;
	
	UPROPERTY(EditAnywhere, Category = "CA")
	int32 MapHeight = 40;
	
	//% chance for a cell to start as a wall (0-100)
	UPROPERTY(EditAnywhere, Category = "CA")
	int32 InitWallChance = 45;

	//How many simulation steps to run
	UPROPERTY(EditAnywhere, Category = "CA")
	int32 SimulationSteps = 5;

	//Classic cave rules:
	//If a cell is wall and has < DeathLimit neighbors => becomes floor
	//if a cell is floor and has > BirthLimit neighbors => becomes wall
	UPROPERTY(EditAnywhere, Category = "CA")
	int32 BirthLimit = 4;
	
	UPROPERTY(EditAnywhere, Category = "CA")
	int32 DeathLimit = 3;
	
	//Size of each cell in world units (cm)
	UPROPERTY(EditAnywhere, Category = "CA")
	float TileSize = 100.f;

	//Mesh used for each floor tile
	UPROPERTY(EditAnywhere, Category = "CA")
	UStaticMesh* FloorMesh;
	
	//Mesh used for walls
	UPROPERTY(EditAnywhere, Category = "CA")
	UStaticMesh* WallMesh;
	
	//Height of floor and walls
	UPROPERTY(EditAnywhere, Category = "CA")
	float FloorZ = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "CA")
	float WallHeight = 200.f;

	//All spawned Wall segments
	UPROPERTY()
	TArray<FDungeonWallSegment> WallSegments;

	// MapWidth * MapHeight array of wall actors, indexed by cell
    UPROPERTY()
    TArray<TWeakObjectPtr<AStaticMeshActor>> WallActorMap;

	// -- Doors --
	
	//How many doors to carve out
	UPROPERTY(EditAnywhere, Category = "Doors")
	int32 DefaultDoorCount = 3;

	//Mesh for the Doors
	UPROPERTY(EditAnywhere, Category = "Doors")
	UStaticMesh* DoorMesh = nullptr;

	//Random seed for reproducibility
	UPROPERTY(EditAnywhere, Category = "Door")
	int32 Seed = 12345;
	
private:
	//Grid: true = wall, false = floor
	TArray<bool> CurrentMap;
	TArray<bool> NextMap;

	FORCEINLINE int32 Index(int32 X, int32 Y) const
	{
		return Y * MapWidth + X;
	}

	void InitializeMap();
	void RunSimulation();
	void StepSimulation();
	int32 CountWallNeighbors(int32 X, int32 Y) const;

	//----Connectivity----
	void EnsureConnectivity();
	void FloodFillRegion(int32 StartX, int32 StartY, int32 RegionId, TArray<int32>& OutLabels, TArray<FIntPoint>& OutCells) const;
	int32 FindClosestPairBetweenRegions(const TArray<FIntPoint>& RegionA, const TArray<FIntPoint>& RegionB, FIntPoint& OutA, FIntPoint& OutB) const;
	void CarveCorridorBetween(const FIntPoint& A, const FIntPoint& B);

	void SpawnGeometry();

	//Spawn the doors
	void CreateDoors(int32 DoorCount);
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
