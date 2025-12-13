// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonManager.h"
#include "Engine/World.h"
#include "CA_FloorGenerator.h"
#include "BSP_FloorGenerator.h"
#include "Holmquist_FloorGenerator.h"
#include "Walk_FloorGenerator.h"
#include "Engine/StaticMeshActor.h"
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

	const FTransform FirstLocation(GetActorRotation(), GetActorLocation());

	// //Spawn the first module at the manager's location
	// FirstModule = SpawnModule(FirstModuleClass, GetActorLocation(), FirstModuleDoors);
	// if (!FirstModule) return;
	FirstModule = SpawnConfiguredModule(FirstModuleClass, FirstLocation, 40, 40, 3);
	if (ABSP_FloorGenerator* BSP = Cast<ABSP_FloorGenerator>(FirstModule))
	{
		BSP->MinLeafSize = 8;
		BSP->MaxDepth = 5;
	}
	if (ACA_FloorGenerator* CA = Cast<ACA_FloorGenerator>(FirstModule))
	{
		CA->SimulationSteps = 5;
		CA->BirthLimit = 4;
		CA->DeathLimit = 3;
	}

	//Get the first module's bounds to decide where to place module 2
	const FBox FirstBounds = FirstModule->GetComponentsBoundingBox(true);

	//Leave MinGapBetweenModules size between the two modules
	const float Gap = FMath::Max(ModuleSeparation, MinGapBetweenModules);

	//Simple separation along +X, large enough to not overlap
	FVector SecondPosition = GetActorLocation();
	SecondPosition.X = FirstBounds.Max.X + Gap;
	SecondPosition.Y = FirstBounds.Min.Y;
	SecondPosition.Z = GetActorLocation().Z;
	const FTransform SecondLocation(GetActorRotation(), SecondPosition);

	//Spawmn the second module at the location
	// SecondModule = SpawnModule(SecondModuleClass, SecondLocation, SecondModuleDoors);
	// if (!SecondModule) return;
	SecondModule = SpawnConfiguredModule(SecondModuleClass, SecondLocation, 40, 40, 3);
	if (ABSP_FloorGenerator* BSP = Cast<ABSP_FloorGenerator>(SecondModule))
	{
		BSP->MinLeafSize = 8;
		BSP->MaxDepth = 5;
	}
	if (ACA_FloorGenerator* CA = Cast<ACA_FloorGenerator>(SecondModule))
	{
		CA->SimulationSteps = 5;
		CA->BirthLimit = 4;
		CA->DeathLimit = 3;
	}

	//Start async wait until both modules finish initializing
	FTimerHandle Timer;
	World->GetTimerManager().SetTimer
	(
		Timer,
		FTimerDelegate::CreateUObject(this, &ADungeonManager::TryConnectModules),
		0.1f,      // wait 0.1 second
		false
	);
	
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

	Module->bGenerateOnBeginPlay = false;

	//Tell the module how many exterior doors are desired
	Module->DesiredExteriorDoors = DesiredDoors;

	Module->GenerateModule();

	return Module;
}

void ADungeonManager::ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B)
{
	if (!A || !B) return;

	FExteriorDoor DoorA, DoorB;
	if (!AlignSecondModuleToFirst(A, B, DoorA, DoorB))
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - Failed to align modules"));
		return;
	}

	//Build the corridor between aligned world door locations
	const float CorridorTileSize = 200.f;
	BuildTiledCorridor(DoorA.Location, DoorB.Location, CorridorTileSize, A->GetActorLocation().Z);


}

void ADungeonManager::TryConnectModules()
{
	if (!FirstModule || !SecondModule) return;

	//Wait until both generators finish building themselves
	if (!FirstModule->bHasFinishedGenerating || !SecondModule->bHasFinishedGenerating)
	{
		//Try again next tick
		GetWorld()->GetTimerManager().SetTimerForNextTick
		(
			FTimerDelegate::CreateUObject(this, &ADungeonManager::TryConnectModules)
		);
		return;
	}

	//Now it's safe to connect doors
	ConnectModulesWithCorridor(FirstModule, SecondModule);
}

AFloorGeneratorBase* ADungeonManager::SpawnConfiguredModule(
        TSubclassOf<AFloorGeneratorBase> ModuleClass,
        const FTransform& SpawnTransform,
        int32 InMapWidth,
        int32 InMapHeight,
        int32 InDoorCount)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//Deferred spawn so BeginPlay doesn't run until FinishSpawning() runs
	Params.bDeferConstruction = true;

	AFloorGeneratorBase* Module = World->SpawnActorDeferred<AFloorGeneratorBase>(ModuleClass, SpawnTransform, this);

	if (!Module) return nullptr;

	//Construction parameters to set before BeginPlay
	Module->MapWidth = InMapWidth;
	Module->MapHeight = InMapHeight;
	Module->DesiredExteriorDoors = InDoorCount;

	//Run ConstructionScript + BeginPlay using the defined values
	Module->FinishSpawning(SpawnTransform);

	return Module;
}

static float NormalizeAngleDegrees(float Degrees)
{
	Degrees = FMath::Fmod(Degrees, 360.f);
	if (Degrees > 180.f) Degrees -= 360.f;
	if (Degrees < -180.f) Degrees += 360.f;
	return Degrees;
}

bool ADungeonManager::AlignSecondModuleToFirst(AFloorGeneratorBase* ModuleA, AFloorGeneratorBase* ModuleB, FExteriorDoor& OutDoorA, FExteriorDoor& OutDoorB)
{
	if (!ModuleA || !ModuleB) return false;

	//Get doors in world space
	const TArray<FExteriorDoor> DoorsA = ModuleA->GetExteriorDoorsWorld();
	const TArray<FExteriorDoor> DoorsB = ModuleB->GetExteriorDoorsWorld();

	if (DoorsA.Num() == 0 || DoorsB.Num() == 0) return false;

	//Choose the closest pair by 2D distance
	float BestDistSq = TNumericLimits<float>::Max();
	int32 BestIndiceA = 0, BestIndiceB = 0;

	for(int32 i = 0; i < DoorsA.Num(); ++i)
	{
		for (int32 j = 0; j < DoorsB.Num(); ++j)
		{
			const float DistSq = FVector::DistSquared2D(DoorsA[i].Location, DoorsB[j].Location);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestIndiceA = i;
				BestIndiceB = j;
			}
		}
	}

	if (!DoorsA.IsValidIndex(BestIndiceA) || !DoorsB.IsValidIndex(BestIndiceB))
	{
		UE_LOG(LogTemp, Warning, TEXT("AlignSecondModuleToFirst: Invalid door indices A=%d B=%d"), BestIndiceA, BestIndiceB);
		return false;
	}

	OutDoorA = DoorsA[BestIndiceA];
	OutDoorB = DoorsB[BestIndiceB];

	//Make DoorB face opposite DoorA through Yaw alignment
	const float YawA = OutDoorA.Rotation.Yaw;
	const float YawB = OutDoorB.Rotation.Yaw;
	const float DesiredYawB = YawA + 180.f;

	const float DeltaYaw = NormalizeAngleDegrees(DesiredYawB - YawB);

	FRotator NewRot = ModuleB->GetActorRotation();
	NewRot.Yaw = NormalizeAngleDegrees(NewRot.Yaw + DeltaYaw);
	ModuleB->SetActorRotation(NewRot);

	//Ensure DoorB forward vector opposes DoorA forward vector
	{
		const FVector ForwardA = OutDoorA.Rotation.Vector().GetSafeNormal2D();
		const FVector ForwardB = OutDoorB.Rotation.Vector().GetSafeNormal2D();

		if (FVector::DotProduct(ForwardA, ForwardB) > -0.99f)
		{
			//Door still aren't opposite, so rotate 180 more
			FRotator Rotator = ModuleB->GetActorRotation();
			Rotator.Yaw += 180.f;
			ModuleB->SetActorRotation(Rotator);
		}
	}

	//Push ModuleB away until it no longer overlaps ModuleA
	{
		const float Padding = 200.f;
		const int32 MaxPushSteps = 50;
		const float PushStep = 200.f;

		for (int32 Step = 0; Step < MaxPushSteps; ++Step)
		{
			const FBox BoundsA = ModuleA->GetComponentsBoundingBox(true);
			const FBox BoundsB = ModuleB->GetComponentsBoundingBox(true);

			if (!BoundsA.Intersect(BoundsB)) break;

			FVector PushDir = (ModuleB->GetActorLocation() - ModuleA->GetActorLocation());
			PushDir.Z = 0.f;
			PushDir = PushDir.GetSafeNormal();

			if (PushDir.IsNearlyZero())
			{
				PushDir = FVector(1.f, 0.f, 0.f);
			}

			FVector PushOffset = PushDir * PushStep;
			ModuleB->SetActorLocation(ModuleB->GetActorLocation() + PushOffset);
		}
	}

	//Re-get DoorB after rotation so it's correct in world space
	{
		const TArray<FExteriorDoor> DoorsB2 = ModuleB->GetExteriorDoorsWorld();
		OutDoorB = DoorsB2.IsValidIndex(BestIndiceB) ? DoorsB2[BestIndiceB] : OutDoorB;
	}

	//Ensure doors align properly along DoorA's forward axis
	{
		FVector DoorAForward = OutDoorA.Rotation.Vector();
		DoorAForward.Z = 0.f;
		DoorAForward.Normalize();

		const FVector DoorALocation = OutDoorA.Location;
		const FVector DoorBLocation = OutDoorB.Location;

		//Vector from A to B.
		const FVector ABVector = DoorBLocation - DoorALocation;

		//Decompose ABVector into parallel + perpendicular
		const float Along = FVector::DotProduct(ABVector, DoorAForward);
		const FVector Parallel = DoorAForward * Along;
		const FVector Perpendicular = ABVector - Parallel;

		//Remove sideways offset so doors lie on the same axis
		ModuleB->SetActorLocation(ModuleB->GetActorLocation() - Perpendicular);
	}
	

	//Translate the module so the doors meet
	{
		const float DoorGap = 0.f;

		const FVector DoorAForward = OutDoorA.Rotation.Vector();
		const FVector TargetDoorBLocation = OutDoorA.Location + (DoorAForward * DoorGap);

		UE_LOG(LogTemp, Log, TEXT("DoorA Loc=%s Rot=%s Fwd=%s"),
			*OutDoorA.Location.ToString(),
			*OutDoorA.Rotation.ToString(),
			*OutDoorA.Rotation.Vector().ToString());

		UE_LOG(LogTemp, Log, TEXT("DoorB Loc=%s Rot=%s Fwd=%s"),
			*OutDoorB.Location.ToString(),
			*OutDoorB.Rotation.ToString(),
			*OutDoorB.Rotation.Vector().ToString());

		const FVector Delta = TargetDoorBLocation - OutDoorB.Location;

		//Move the module so DoorB lands at TargetDoorBLocation
		ModuleB->SetActorLocation(ModuleB->GetActorLocation() + Delta);
	}

	//Overlap insurance
	{
		const int32 MaxPushSteps = 50;
		const float PushStep = 100.f;

		for (int32 Step = 0; Step < MaxPushSteps; ++Step)
		{
			const FBox BoundsA = ModuleA->GetComponentsBoundingBox(true);
			const FBox BoundsB = ModuleB->GetComponentsBoundingBox(true);

			if (!BoundsA.Intersect(BoundsB)) break;

			FVector PushDir = (ModuleB->GetActorLocation() - ModuleA->GetActorLocation());
			PushDir.Z = 0.f;
			if (!PushDir.Normalize())
			{
				PushDir = FVector(1.f, 0.f, 0.f);
			}

			ModuleB->SetActorLocation(ModuleB->GetActorLocation() + PushDir * PushStep);
		}
	}

	//Get DoorB again so the caller gets the final correct position
	{
		const TArray<FExteriorDoor> DoorsB3 = ModuleB->GetExteriorDoorsWorld();
		OutDoorB = DoorsB3.IsValidIndex(BestIndiceB) ? DoorsB3[BestIndiceB] : OutDoorB;
	}

	// // --- Solve ModuleB transform so DoorB matches DoorA (opposite facing + gap) ---

	// //Define Corridor gap between door centers
	// const float DoorGap = 400.f;

	// //Use Yaw
	// const FRotator DoorARot = OutDoorA.Rotation;
	// const FVector  DoorAForward = DoorARot.Vector().GetSafeNormal2D();

	// const FVector DesiredDoorBLocation = OutDoorA.Location + DoorAForward * DoorGap;
	// const FRotator DesiredDoorBRotation = FRotator(0.f, NormalizeAngleDegrees(DoorARot.Yaw + 180.f), 0.f);

	// //Rotate ModuleB so DoorB faces DesiredDoorBRotation
	// {
	// 	//Refresh DoorB before computing delta yaw
	// 	const TArray<FExteriorDoor> DoorsB_Pre = ModuleB->GetExteriorDoorsWorld();
	// 	FExteriorDoor DoorB_Pre = DoorsB_Pre.IsValidIndex(BestIndiceB) ? DoorsB_Pre[BestIndiceB] : OutDoorB;

	// 	const float CurrentYawB = DoorB_Pre.Rotation.Yaw;
	// 	const float DeltaYaw = NormalizeAngleDegrees(DesiredDoorBRotation.Yaw - CurrentYawB);

	// 	FRotator NewRot = ModuleB->GetActorRotation();
	// 	NewRot.Yaw = NormalizeAngleDegrees(NewRot.Yaw + DeltaYaw);
	// 	ModuleB->SetActorRotation(NewRot);
	// }

	// //Translate ModuleB so DoorB lands on DesiredDoorBLocation
	// {
	// 	const TArray<FExteriorDoor> DoorsB_PostRot = ModuleB->GetExteriorDoorsWorld();
	// 	FExteriorDoor DoorB_PostRot = DoorsB_PostRot.IsValidIndex(BestIndiceB) ? DoorsB_PostRot[BestIndiceB] : OutDoorB;

	// 	const FVector Delta = DesiredDoorBLocation - DoorB_PostRot.Location;
	// 	ModuleB->SetActorLocation(ModuleB->GetActorLocation() + Delta);
	// }

	// //Refresh the outputs so the caller gets the correct final values
	// {
	// 	const TArray<FExteriorDoor> DoorsB_Final = ModuleB->GetExteriorDoorsWorld();
	// 	OutDoorB = DoorsB_Final.IsValidIndex(BestIndiceB) ? DoorsB_Final[BestIndiceB] : OutDoorB;
	// }

	return true;

}

static FIntPoint WorldToGrid2D(const FVector& Point, const FVector& Origin, float TileSize)
{
	const float GridPointX = (Point.X - Origin.X) / TileSize;
	const float GridPointY = (Point.Y - Origin.Y) / TileSize;
	return FIntPoint(FMath::RoundToInt(GridPointX), FMath::RoundToInt(GridPointY));
}

static FVector GridToWorld2D(const FIntPoint& GridPoint, const FVector& Origin, float TileSize, float Z)
{
	return Origin + FVector(GridPoint.X * TileSize, GridPoint.Y * TileSize, Z);
}

void ADungeonManager::BuildTiledCorridor(const FVector& FromWorld, const FVector& ToWorld, float TileSize, float Z)
{
	if (!CorridorFloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - CorridorFloorMesh not set"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	//Pick a corridor grid origin, using the Dungeon Manager rigin for consistency
	const FVector Origin = GetActorLocation();

	const FIntPoint PointA = WorldToGrid2D(FromWorld, Origin, TileSize);
	const FIntPoint PointB = WorldToGrid2D(ToWorld, Origin, TileSize);

	auto SpawnFloorTile = [&](const FIntPoint& Cell)
	{
		const FVector Pos = GridToWorld2D(Cell, Origin, TileSize, Z);

		AStaticMeshActor* Tile = World->SpawnActor<AStaticMeshActor>(Pos, FRotator::ZeroRotator);
		if (!Tile) return;

		Tile->SetMobility(EComponentMobility::Movable);
		Tile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

		UStaticMeshComponent* MeshComp = Tile->GetStaticMeshComponent();
		if (!MeshComp)
		{
			Tile->Destroy();
			return;
		}

		MeshComp->SetStaticMesh(CorridorFloorMesh);

		const float BasePlane = 100.f;
		const float Scale = TileSize / BasePlane;
		Tile->SetActorScale3D(FVector(Scale, Scale, 1.f));
	};

	//Walk L-shaped corridor
	FIntPoint Cur = PointA;

	const int32 StepX = (PointB.X >= Cur.X) ? 1 : -1;
	while (Cur.X != PointB.X)
	{
		SpawnFloorTile(Cur);
		Cur.X += StepX;
	}

	const int32 StepY = (PointB.Y >= Cur.Y) ? 1 : -1;
	while (Cur.Y != PointB.Y)
	{
		SpawnFloorTile(Cur);
		Cur.Y += StepY;
	}

	SpawnFloorTile(PointB);
}