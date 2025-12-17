// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorGeneratorBase.h"
#include "DungeonManager.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API ADungeonManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Which moduule types to spawn for test
	UPROPERTY(EditAnywhere, Category="Dungeon")
	TSubclassOf<AFloorGeneratorBase> FirstModuleClass;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	TSubclassOf<AFloorGeneratorBase> SecondModuleClass;

	//Tile Size
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 MapWidth = 30; 
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 MapHeight = 30;

	//Holmquist Size
	UPROPERTY(EditAnywhere, Category = "Dungeon");
	int32 HolmquistTiles = 80;

	//Desired doors
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 FirstModuleDoors = 2;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 SecondModuleDoors = 2;

	//How far to separate modules so they don't overlap
	UPROPERTY(EditAnywhere, Category="Dungeon")
	float ModuleSeparation = 400.f;

	UPROPERTY(EditAnywhere, Category="Dungeon")
	float MinGapBetweenModules = 400.f;

	//Corridor meshes
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorFloorMesh;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorWallMesh;

	// -- Corridors --
	bool AlignSecondModuleToFirst(AFloorGeneratorBase* ModuleA, AFloorGeneratorBase* ModuleB, FExteriorDoor& OutDoorA, FExteriorDoor& OutDoorB, int32& OutDoorBIndex);
	void BuildTiledCorridor(const FVector& FromWorld, const FVector& ToWorld, const FVector& GridOrigin, float TileSize, float Z);
	void BuildStraightCorridor(const FVector& FromWorld, const FVector& ToWorld, float TileSize, float Z);
	void BuildCorridor(const FExteriorDoor& Door, int32 NumTiles, float TileSize, float Z);
	bool BuildCorridorFromDoorOnBounds(AFloorGeneratorBase* Module, const FExteriorDoor& Door, int32 NumTiles, float TileSize, float Z, FVector& OutForward, FVector& OutLastTileWorld);

private:

	AFloorGeneratorBase* FirstModule = nullptr;
	AFloorGeneratorBase* SecondModule = nullptr;

	AFloorGeneratorBase* SpawnModule(TSubclassOf<AFloorGeneratorBase> ModuleClass, const FVector& Location, int32 DesiredDoors);
	

    UFUNCTION(BlueprintCallable, Category="Dungeon")
    AFloorGeneratorBase* SpawnConfiguredModule(
        TSubclassOf<AFloorGeneratorBase> ModuleClass,
        const FTransform& SpawnTransform,
        int32 InMapWidth,
        int32 InMapHeight,
        int32 InDoorCount
    );

	void ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B);
	void TryConnectModules();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
