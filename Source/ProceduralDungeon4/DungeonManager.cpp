// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ADungeonManager::ADungeonManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ADungeonManager::BeginPlay()
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
	AFloorGeneratorBase* FirstModule = SpawnModule(FirstModuleClass, GetActorLocation(), FirstModuleDoors);
	if (!FirstModule) return;

	//Get the first module's bounds to decide where to place module 2
	FBox FirstBounds = FirstModule->GetComponentsBoundingBox(true);
	const FVector FirstSize = FirstBounds.GetSize();

	//Simple separation along +X, large enough to not overlap
	FVector SecondLocation = GetActorLocation();
	SecondLocation.X += FirstSize.X + ModuleSeparation;

	//Spawmn the second module at the location
	AFloorGeneratorBase* SecondModule = SpawnModule(SecondModuleClass, SecondLocation, SecondModuleDoors);
	if (!SecondModule) return;

	//Connect closest exterior doors
	ConnectModulesWithCorridor(FirstModule, SecondModule);
	
}

// Called every frame
void ADungeonManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AFloorGeneratorBase* ADungeonManager::SpawnModule(TSubclassOf<AFloorGeneratorBase> ModuleClass, const FVector& Location, int32 DesiredDoors)
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

	//Tell the module how many exterior doors are desired
	Module->DesiredExteriorDoors = DesiredDoors;

	//Build the module
	Module->GenerateModule();

	return Module;
}

void ADungeonManager::ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B)
{
	if (!A || !B) return;

	const TArray<FExteriorDoor>& DoorsA = A->GetExteriorDoors();
	const TArray<FExteriorDoor>& DoorsB = B->GetExteriorDoors();

	if (DoorsA.Num() == 0 || DoorsB.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - One of the modules has no exterior doors"));
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

	BuildStraightCorridor(BestA, BestB);
}

void ADungeonManager::BuildStraightCorridor(const FVector& From, const FVector& To)
{
	if (!CorridorFloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dungeon Manager - CorridorFloorMesh not set"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	FVector Start = From;
	FVector End = To;
	Start.Z = End.Z = From.Z;

	const FVector Delta = End - Start;
	const float Length = Delta.Size2D();
	if (Length < KINDA_SMALL_NUMBER) return;

	const FVector Dir = FVector(Delta.X, Delta.Y, 0.f).GetSafeNormal();

	//Use Floor tiles of roughly TileSize length
	const float Step = 200.f;
	const int32 NumSegments = FMath::CeilToInt(Length / Step);

	for (int32 i = 0; i <= NumSegments; ++i)
	{
		float T = (NumSegments == 0) ? 0.f : (float)i / (float)NumSegments;
		FVector Pos = FMath::Lerp(Start, End, T);

		FTransform Tr;
		Tr.SetLocation(Pos);
		Tr.SetRotation(Dir.ToOrientationQuat());

		AActor* Segment = World->SpawnActor<AActor>(AActor::StaticClass(), Tr);

		if (Segment)
		{
			//Give it a StatiMeshComponent using the corridor mesh
			UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Segment);
			MeshComp->SetStaticMesh(CorridorFloorMesh);
			MeshComp->SetupAttachment(Segment->GetRootComponent());
			MeshComp->RegisterComponent();
		}
	}
}

