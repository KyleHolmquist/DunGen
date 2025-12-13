// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorGeneratorBase.h"
#include "RoomGenerator.generated.h"

class AGridSpace;
class ADungeonRoom;

UCLASS()
class PROCEDURALDUNGEON4_API ARoomGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomGenerator();

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

	UPROPERTY(EditAnywhere, Category="Dungeon")
	float MinGapBetweenModules = 400.f;

	//Corridor meshes
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorFloorMesh;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorWallMesh;

private:

	AFloorGeneratorBase* FirstModule = nullptr;
	AFloorGeneratorBase* SecondModule = nullptr;

	AFloorGeneratorBase* SpawnModule(TSubclassOf<AFloorGeneratorBase> ModuleClass, const FVector& Location);

	void ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
