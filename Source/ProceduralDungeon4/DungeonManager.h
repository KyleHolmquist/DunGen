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

	//Desired doors
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 FirstModuleDoors = 2;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 SecondModuleDoors = 2;

	//How far to separate modules so they don't overlap
	UPROPERTY(EditAnywhere, Category="Dungeon")
	float ModuleSeparation = 2000.f;

	//Corridor meshes
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorFloorMesh;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorWallMesh;

private:
	AFloorGeneratorBase* SpawnModule(TSubclassOf<AFloorGeneratorBase> ModuleClass, const FVector& Location, int32 DesiredDoors);

	void ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B);

	void BuildStraightCorridor(const FVector& From, const FVector& To);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
