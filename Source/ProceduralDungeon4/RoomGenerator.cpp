// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomGenerator.h"
#include "GridSpace.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProceduralDungeonGameMode.h"
#include "DungeonRoom.h"

// Sets default values
ARoomGenerator::ARoomGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARoomGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!FirstModuleClass || !SecondModuleClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager Module classes not set"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	//Spawn the first module at the manager's location
	FirstModule = SpawnModule(FirstModuleClass, GetActorLocation());
	if (!FirstModule) return;

	// //Get the first module's bounds to decide where to place module 2
	// const FBox FirstBounds = FirstModule->GetComponentsBoundingBox(true);

	// //Leave MinGapBetweenModules size between the two modules
	// const float Gap = FMath::Max(ModuleSeparation, MinGapBetweenModules);

	// //Simple separation along +X, large enough to not overlap
	// FVector SecondLocation = GetActorLocation();
	// SecondLocation.X = FirstBounds.Max.X + Gap;
	// SecondLocation.Y = FirstBounds.Min.Y;
	// SecondLocation.Z = GetActorLocation().Z;

	// //Spawmn the second module at the location
	// SecondModule = SpawnModule(SecondModuleClass, SecondLocation, SecondModuleDoors);
	// if (!SecondModule) return;

	// //Start async wait until both modules finish initializing
	// FTimerHandle Timer;
	// World->GetTimerManager().SetTimer
	// (
	// 	Timer,
	// 	FTimerDelegate::CreateUObject(this, &ADungeonManager::TryConnectModules),
	// 	0.1f,      // wait 0.1 second
	// 	false
	// );
		
}

// Called every frame
void ARoomGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


AFloorGeneratorBase* ARoomGenerator::SpawnModule(TSubclassOf<AFloorGeneratorBase> ModuleClass, const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World || !*ModuleClass)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AFloorGeneratorBase* Module = World->SpawnActor<AFloorGeneratorBase>(ModuleClass, Location, FRotator::ZeroRotator, Params);
	if (!Module) return nullptr;

	return Module;
}

void ARoomGenerator::ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B)
{
	if (!A || !B) return;

	const TArray<FExteriorDoor>& DoorsA = A->GetExteriorDoors();
	const TArray<FExteriorDoor>& DoorsB = B->GetExteriorDoors();

	 UE_LOG(LogTemp, Log, TEXT("DungeonManager: Module A has %d exterior doors, Module B has %d"), DoorsA.Num(), DoorsB.Num());


	if (DoorsA.Num() == 0 || DoorsB.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - One of the modules has no exterior doors"));
		return;
	}

	if (!CorridorFloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dungeon Manager - CorridorFloorMesh not set"));
		return;
	}

	//Find the closest pair of doors
	float BestDistSq = TNumericLimits<float>::Max();
	FVector BestA = DoorsA[0].Location;
	FVector BestB = DoorsB[0].Location;

	for (const FExteriorDoor& DoorA : DoorsA)
	{
		for (const FExteriorDoor& DoorB : DoorsB)
		{
			const float DistSq = FVector::DistSquared2D(DoorA.Location, DoorB.Location);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestA = DoorA.Location;
				BestB = DoorB.Location;
			}
		}
	}
}