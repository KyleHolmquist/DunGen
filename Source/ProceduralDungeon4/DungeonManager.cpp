// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonManager.h"
#include "Engine/World.h"
#include "CA_FloorGenerator.h"
#include "BSP_FloorGenerator.h"
#include "Holmquist_FloorGenerator.h"
#include "Walk_FloorGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "FloorTile.h"
#include "WallTile.h"
#include "Treasure.h"
#include "BreakableActor.h"
#include "Enemy.h"
#include "Prop.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FloorGeneratorBase.h"
#include "Components/CapsuleComponent.h"
#include "Weapon.h"
#include "PortalManager.h"

// -- Forward declarations --
static FVector ComputeOutwardFromBounds2D(AFloorGeneratorBase* Module, const FVector& DoorWorld);
static bool SnapModuleDoorToTarget(AFloorGeneratorBase* ModuleB, int32 DoorBIndex, const FVector& TargetDoorWorldLocation, const FVector& TargetDoorForward2D);

ADungeonManager::ADungeonManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void ADungeonManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	InitializeDungeonLevelParams();
	DungeonModule = SpawnDungeonModule();
	if (!DungeonModule)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager::BeginPlay - DungeonModule is null."));
    	return;
	}
	
	DungeonModule->SetFloorTile(SelectedFloorTileClass);
	DungeonModule->SetWallTile(SelectedWallTileClass);
	DungeonModule->GenerateModule();
	ExteriorDoors = DungeonModule->GetGeneratedDoorLocations();
	if (ExteriorDoors.Num() > 0)
	{
		int DoorChoice = FMath::RandRange(0, ExteriorDoors.Num() - 1);
			FVector SelectedDoorLocation = ExteriorDoors[DoorChoice];
			//Spawn a portal at one of the ExteriorDoors
			if (PortalManager)
			{
				PortalManager->SpawnAwayPortal(SelectedDoorLocation, FRotator::ZeroRotator);
			}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager::BeginPlay - ExteriorDoors is empty."));
	}
	
	EmptyLocations = DungeonModule->GetGeneratedEmptyLocations();
	PopulateDungeon();

	//Start async wait until both modules finish initializing
	FTimerHandle Timer;
	World->GetTimerManager().SetTimer
	(
		Timer,
		FTimerDelegate::CreateUObject(this, &ADungeonManager::TryConnectModules),
		0.1f,      // wait 0.1 second
		false
	);

	//Old BeginPlay() code

	/*
	const FTransform FirstLocation(GetActorRotation(), GetActorLocation());

	//Spawn the first module at the manager's location
	FirstModule = SpawnModule(FirstModuleClass, GetActorLocation(), FirstModuleDoors);
	if (!FirstModule) return;
	


	

	if (!FirstModuleClass || !SecondModuleClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager Module classes not set"));
		return;
	}
	
	FirstModule = SpawnConfiguredModule(FirstModuleClass, FirstLocation, MapWidth, MapHeight, FirstModuleDoors);
	if (ABSP_FloorGenerator* BSP = Cast<ABSP_FloorGenerator>(FirstModule))
	{
		BSP->MinLeafSize = 8;
		BSP->MaxDepth = 8;
	}
	if (ACA_FloorGenerator* CA = Cast<ACA_FloorGenerator>(FirstModule))
	{
		CA->SimulationSteps = 5;
		CA->BirthLimit = 4;
		CA->DeathLimit = 3;
	}

	//Get the first module's bounds to decide where to place module 2
	//const FBox FirstBounds = FirstModule->GetComponentsBoundingBox(true);
	const FBox FirstBounds = FirstModule->GetModuleBoundsWorld(0.f, 0.f);

	//Leave MinGapBetweenModules size between the two modules
	const float Gap = FMath::Max(ModuleSeparation, MinGapBetweenModules);

	UE_LOG(LogTemp, Log, TEXT("FirstBounds Min=%s Max=%s Center=%s Gap=%.1f"),
		*FirstBounds.Min.ToString(),
		*FirstBounds.Max.ToString(),
		*FirstBounds.GetCenter().ToString(),
		Gap);

	//Simple separation along +X, large enough to not overlap
	FVector SecondPosition = GetActorLocation();
	SecondPosition.X = FirstBounds.Max.X + Gap;
	SecondPosition.Y = FirstModule->GetActorLocation().Y;
	SecondPosition.Z = GetActorLocation().Z;
	const FTransform SecondLocation(GetActorRotation(), SecondPosition);

	//Spawmn the second module at the location
	// SecondModule = SpawnModule(SecondModuleClass, SecondLocation, SecondModuleDoors);
	// if (!SecondModule) return;
	SecondModule = SpawnConfiguredModule(SecondModuleClass, SecondLocation, MapWidth, MapHeight, SecondModuleDoors);
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
	if (AHolmquist_FloorGenerator* Holmquist = Cast<AHolmquist_FloorGenerator>(SecondModule))
	{
		Holmquist-> NumTiles = HolmquistTiles;
	}
	*/
	// FString PlayerName = TEXT("Airsto");
	// FString SelectedThemeText = TEXT("Castle");

	// FString SelectedTreasureText;
	// if (SelectedTreasureClass)
	// {
	// 	const ATreasure* TreasureCDO = SelectedTreasureClass->GetDefaultObject<ATreasure>();
	// 	if (TreasureCDO)
	// 	{
	// 		SelectedTreasureText = TreasureCDO->GetDisplayName();
	// 	}
	// }

	// FText QuestText = GenerateQuestText(QuestAdjectivesTable, PlayerName, SelectedThemeText, SelectedTreasureText);

	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, QuestText.ToString());
	// }
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

	Module->SetDungeonManager(this);
	UE_LOG(LogTemp, Warning, TEXT("Finished ModuleSetDungeonManager"));

	//Module->GenerateModule();

	return Module;
}

void ADungeonManager::ConnectModulesWithCorridor(AFloorGeneratorBase* ModuleA, AFloorGeneratorBase* ModuleB)
{
    if (!ModuleA || !ModuleB) return;

    FExteriorDoor DoorA, DoorB;
    int32 DoorBIndex = INDEX_NONE;

    if (!AlignSecondModuleToFirst(ModuleA, ModuleB, DoorA, DoorB, DoorBIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("DungeonManager - Failed to align modules"));
        return;
    }

    const float CorridorTileSize = 200.f;
    const int32 NumTiles = 12;

    FVector ForwardA = FVector::ZeroVector;
    FVector LastTile = FVector::ZeroVector;

    if (!BuildCorridorFromDoorOnBounds(
            ModuleA,
            DoorA,
            NumTiles,
            CorridorTileSize,
            ModuleA->GetActorLocation().Z,
            ForwardA,
            LastTile))
    {
        UE_LOG(LogTemp, Warning, TEXT("DungeonManager - Failed to build corridor stub from DoorA"));
        return;
    }

    //Place DoorB one tile beyond the last corridor tile, facing back into the corridor
    const FVector TargetDoorBLocation = LastTile + ForwardA * CorridorTileSize;
    const FVector TargetDoorBForward  = (-ForwardA).GetSafeNormal2D();

    if (!SnapModuleDoorToTarget(ModuleB, DoorBIndex, TargetDoorBLocation, TargetDoorBForward))
    {
        UE_LOG(LogTemp, Warning, TEXT("DungeonManager - Failed to snap ModuleB door to corridor end"));
        return;
    }

    //Refresh DoorB for logging
    const TArray<FExteriorDoor> DoorsBFinal = ModuleB->GetExteriorDoorsWorld();
    if (DoorsBFinal.IsValidIndex(DoorBIndex))
    {
		const FVector FwdB = ComputeOutwardFromBounds2D(ModuleB, DoorsBFinal[DoorBIndex].Location);
		DrawDebugLine(GetWorld(), DoorsBFinal[DoorBIndex].Location, DoorsBFinal[DoorBIndex].Location + FwdB * 800.f, FColor::Red, true, 0.f, 0, 10.f);
        DoorB = DoorsBFinal[DoorBIndex];
    }

    UE_LOG(LogTemp, Log, TEXT("DoorA Loc=%s  ForwardA=%s  LastTile=%s  TargetDoorB=%s  DoorBFinal=%s Rot=%s"),
        *DoorA.Location.ToString(),
        *ForwardA.ToString(),
        *LastTile.ToString(),
        *TargetDoorBLocation.ToString(),
        *DoorB.Location.ToString(),
        *DoorB.Rotation.ToString());
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
	Module->InitMapSize(InMapWidth, InMapHeight);
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

static FVector ComputeOutwardFromBounds2D(AFloorGeneratorBase* Module, const FVector& DoorWorld)
{
    const FBox Box = Module->GetModuleBoundsWorld(0.f, 0.f);

    const float dMinX = FMath::Abs(DoorWorld.X - Box.Min.X);
    const float dMaxX = FMath::Abs(DoorWorld.X - Box.Max.X);
    const float dMinY = FMath::Abs(DoorWorld.Y - Box.Min.Y);
    const float dMaxY = FMath::Abs(DoorWorld.Y - Box.Max.Y);

    FVector Forward(-1, 0, 0);
    float Best = dMinX;

    if (dMaxX < Best) { Best = dMaxX; Forward = FVector( 1, 0, 0); }
    if (dMinY < Best) { Best = dMinY; Forward = FVector( 0,-1, 0); }
    if (dMaxY < Best) { Best = dMaxY; Forward = FVector( 0, 1, 0); }

    return Forward;
}


static bool SnapModuleDoorToTarget(AFloorGeneratorBase* ModuleB, int32 DoorBIndex, const FVector& TargetDoorWorldLocation, const FVector& TargetDoorForward2D)
{
    if (!ModuleB) return false;

    FVector DesiredForward = TargetDoorForward2D;
    DesiredForward.Z = 0.f;
    if (!DesiredForward.Normalize()) return false;

    // -- Rotate module so DoorB faces DesiredForward --
    {
        const TArray<FExteriorDoor> Doors = ModuleB->GetExteriorDoorsWorld();
        if (!Doors.IsValidIndex(DoorBIndex)) return false;

		const FVector CurrentForward = ComputeOutwardFromBounds2D(ModuleB, Doors[DoorBIndex].Location);

        const float CurrentYaw = CurrentForward.Rotation().Yaw;
        const float DesiredYaw = DesiredForward.Rotation().Yaw;

        const float DeltaYaw = NormalizeAngleDegrees(DesiredYaw - CurrentYaw);

        FRotator NewRot = ModuleB->GetActorRotation();
        NewRot.Yaw = NormalizeAngleDegrees(NewRot.Yaw + DeltaYaw);
        ModuleB->SetActorRotation(NewRot);
    }

    // -- Translate module so DoorB lands on target location --
    {
        const TArray<FExteriorDoor> Doors = ModuleB->GetExteriorDoorsWorld();
        if (!Doors.IsValidIndex(DoorBIndex)) return false;

        const FVector Delta = TargetDoorWorldLocation - Doors[DoorBIndex].Location;
        ModuleB->SetActorLocation(ModuleB->GetActorLocation() + Delta);
    }

    return true;
}


bool ADungeonManager::AlignSecondModuleToFirst(AFloorGeneratorBase* ModuleA, AFloorGeneratorBase* ModuleB, FExteriorDoor& OutDoorA, FExteriorDoor& OutDoorB, int32& OutDoorBIndex)
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
	OutDoorBIndex = BestIndiceB;

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

		//Overlap resolution (single-step using deterministic bounds)
	{
		const float Padding = MinGapBetweenModules; // reuse your setting :contentReference[oaicite:5]{index=5}

		const FBox BoundsA = ModuleA->GetModuleBoundsWorld(0.f, 0.f);
		const FBox BoundsB = ModuleB->GetModuleBoundsWorld(0.f, 0.f);

		if (BoundsA.Intersect(BoundsB))
		{
			//Push B away from A along a stable direction.
			FVector PushDir = (ModuleB->GetActorLocation() - ModuleA->GetActorLocation());
			PushDir.Z = 0.f;

			if (!PushDir.Normalize())
			{
				PushDir = FVector(1.f, 0.f, 0.f);
			}

			//Compute a push distance based on overlap of AABB projections.
			//Push enough so the centers separate by at least sum of extents + padding on the dominant axis.
			const FVector CenterA = BoundsA.GetCenter();
			const FVector CenterB = BoundsB.GetCenter();
			const FVector DeltaC  = CenterB - CenterA;

			const FVector ExtA = BoundsA.GetExtent();
			const FVector ExtB = BoundsB.GetExtent();

			//Choose axis with larger penetration potential
			const float NeededX = (ExtA.X + ExtB.X + Padding) - FMath::Abs(DeltaC.X);
			const float NeededY = (ExtA.Y + ExtB.Y + Padding) - FMath::Abs(DeltaC.Y);

			float PushDist = 0.f;
			if (NeededX > NeededY)
			{
				PushDist = FMath::Max(0.f, NeededX);
				PushDir = FVector(FMath::Sign(DeltaC.X) == 0.f ? 1.f : FMath::Sign(DeltaC.X), 0.f, 0.f);
			}
			else
			{
				PushDist = FMath::Max(0.f, NeededY);
				PushDir = FVector(0.f, FMath::Sign(DeltaC.Y) == 0.f ? 1.f : FMath::Sign(DeltaC.Y), 0.f);
			}

			ModuleB->SetActorLocation(ModuleB->GetActorLocation() + PushDir * PushDist);
		}
	}

	//Get DoorB again so the caller gets the final correct position
	{
		const TArray<FExteriorDoor> DoorsB3 = ModuleB->GetExteriorDoorsWorld();
		OutDoorB = DoorsB3.IsValidIndex(BestIndiceB) ? DoorsB3[BestIndiceB] : OutDoorB;
	}

	return true;

}

static FIntPoint WorldToGrid2D(const FVector& Point, const FVector& Origin, float TileSize)
{
	const float GridPointX = (Point.X - Origin.X) / TileSize;
	const float GridPointY = (Point.Y - Origin.Y) / TileSize;
	return FIntPoint(FMath::FloorToInt(GridPointX + 0.5f), FMath::FloorToInt(GridPointY + 0.5f));
}

static FVector GridToWorld2D(const FIntPoint& GridPoint, const FVector& Origin, float TileSize, float Z)
{
	return Origin + FVector(GridPoint.X * TileSize, GridPoint.Y * TileSize, Z);
}

void ADungeonManager::BuildTiledCorridor(const FVector& FromWorld, const FVector& ToWorld, const FVector& GridOrigin, float TileSize, float Z)
{
	if (!CorridorFloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - CorridorFloorMesh not set"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Origin = GridOrigin;
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

void ADungeonManager::BuildStraightCorridor(const FVector& FromWorld, const FVector& ToWorld, float TileSize, float Z)
{
	if (!CorridorFloorMesh) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector Delta = ToWorld - FromWorld;
    Delta.Z = 0.f;

    const float Dist = Delta.Size();
    if (Dist < KINDA_SMALL_NUMBER) return;

    const FVector Dir = Delta / Dist;

    //Rotate tiles to face along the corridor direction
    const float Yaw = Dir.Rotation().Yaw;
    const FRotator TileRot(0.f, Yaw, 0.f);

    const int32 Steps = FMath::Max(1, FMath::RoundToInt(Dist / TileSize));

    for (int32 i = 0; i <= Steps; ++i)
    {
        const FVector Pos = FromWorld + Dir * (i * TileSize);
        const FVector SpawnPos(Pos.X, Pos.Y, Z);

        AStaticMeshActor* Tile = World->SpawnActor<AStaticMeshActor>(SpawnPos, TileRot);
        if (!Tile) continue;

        UStaticMeshComponent* MeshComp = Tile->GetStaticMeshComponent();
        if (!MeshComp) { Tile->Destroy(); continue; }

        //Mobility before SetStaticMesh to avoid warnings
        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(CorridorFloorMesh);

        Tile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

        const float BasePlane = 100.f;
        const float Scale = TileSize / BasePlane;
        Tile->SetActorScale3D(FVector(Scale, Scale, 1.f));
    }
}

void ADungeonManager::BuildCorridor(const FExteriorDoor& Door, int32 NumTiles, float TileSize, float Z)
{
	if (!CorridorFloorMesh) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector Forward = Door.Rotation.Vector();
	Forward.Z = 0.f;
	if (!Forward.Normalize())
	{
		Forward = FVector(1.f, 0.f, 0.f);
	}

	//Start one tile outside the door
	const FVector Start = Door.Location + Forward * TileSize;

	const float Yaw = Forward.Rotation().Yaw;
	const FRotator TileRot(0.f, Yaw, 0.f);

	for (int32 i = 0; i < NumTiles ; ++i)
	{
		const FVector Pos = Start + Forward * (i * TileSize);
		const FVector SpawnPos(Pos.X, Pos.Y, Z);

		AStaticMeshActor* Tile = World->SpawnActor<AStaticMeshActor>(SpawnPos, TileRot);
		if (!Tile) continue;

		UStaticMeshComponent* MeshComp = Tile->GetStaticMeshComponent();
		if (!MeshComp)
		{
			Tile->Destroy();
			continue;
		}

		//Mobility before SetStaticMesh
		MeshComp->SetMobility(EComponentMobility::Movable);
		MeshComp->SetStaticMesh(CorridorFloorMesh);

		Tile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

		const float BasePlane = 100.f;
		const float Scale = TileSize / BasePlane;
		Tile->SetActorScale3D(FVector(Scale, Scale, 1.f));
	}

	DrawDebugSphere(GetWorld(), Door.Location, 80.f, 16, FColor::Green, true, 0.f, 0, 4.f);

	FVector Fwd = Door.Rotation.Vector();
	Fwd.Z = 0.f; Fwd.Normalize();
	DrawDebugLine(GetWorld(), Door.Location, Door.Location + Fwd * 500.f, FColor::Blue, true, 10.f, 0, 8.f);
}

bool ADungeonManager::BuildCorridorFromDoorOnBounds(AFloorGeneratorBase* Module, const FExteriorDoor& Door, int32 NumTiles, float TileSize, float Z, FVector& OutForward, FVector& OutLastTileWorld)
{
    OutForward = FVector::ZeroVector;
    OutLastTileWorld = Door.Location;

    if (!Module || !CorridorFloorMesh) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    const FBox Box = Module->GetModuleBoundsWorld(0.f, 0.f);

    //Choose outward direction by nearest bounds side
    const float dMinX = FMath::Abs(Door.Location.X - Box.Min.X);
    const float dMaxX = FMath::Abs(Door.Location.X - Box.Max.X);
    const float dMinY = FMath::Abs(Door.Location.Y - Box.Min.Y);
    const float dMaxY = FMath::Abs(Door.Location.Y - Box.Max.Y);

    FVector Forward(-1, 0, 0);
    float Best = dMinX;

    if (dMaxX < Best) { Best = dMaxX; Forward = FVector( 1, 0, 0); }
    if (dMinY < Best) { Best = dMinY; Forward = FVector( 0,-1, 0); }
    if (dMaxY < Best) { Best = dMaxY; Forward = FVector( 0, 1, 0); }

    OutForward = Forward;

    //Start one tile outside the door
    const FVector Start = Door.Location + Forward * TileSize;

    //The last tile center
    OutLastTileWorld = Start + Forward * ((FMath::Max(NumTiles, 1) - 1) * TileSize);

    //Persistent debug
    DrawDebugSphere(World, Door.Location, 80.f, 16, FColor::Green, true, 0.f, 0, 4.f);
    DrawDebugLine(World, Door.Location, Door.Location + Forward * 800.f, FColor::Blue, true, 0.f, 0, 10.f);

    for (int32 i = 0; i < NumTiles; ++i)
    {
        const FVector Pos = Start + Forward * (i * TileSize);
        const FVector SpawnPos(Pos.X, Pos.Y, Z);

        AStaticMeshActor* Tile = World->SpawnActor<AStaticMeshActor>(SpawnPos, Forward.Rotation());
        if (!Tile) continue;

        UStaticMeshComponent* MeshComp = Tile->GetStaticMeshComponent();
        if (!MeshComp) { Tile->Destroy(); continue; }

        MeshComp->SetMobility(EComponentMobility::Movable);
        MeshComp->SetStaticMesh(CorridorFloorMesh);

        const float BasePlane = 100.f;
        const float Scale = TileSize / BasePlane;
        Tile->SetActorScale3D(FVector(Scale, Scale, 1.f));

        Tile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    }

    return true;
}

void ADungeonManager::SetDungeonTheme()
{
	DungeonTheme = GetRandomDungeonTheme();	
}

void ADungeonManager::SetFloorGenerator()
{
	SelectedFloorGeneratorClass = GetRandomFloorGenerator();	
}

void ADungeonManager::SetFloorTile()
{
	SelectedFloorTileClass = GetRandomFloorTile();	
}

void ADungeonManager::SetWallTile()
{
	SelectedWallTileClass = GetRandomWallTile();	
}

void ADungeonManager::SetTreasure()
{
	SelectedTreasureClass = GetRandomTreasure();	
}

void ADungeonManager::SetBreakable()
{
	SelectedBreakableClass = GetRandomBreakable();	
}

void ADungeonManager::SetEnemy()
{
	SelectedEnemyClass = GetRandomEnemy();	
}

void ADungeonManager::SetProp()
{
	SelectedPropClass = GetRandomProp();	
}

void ADungeonManager::InitializeDungeonLevelParams()
{
	SetDungeonTheme();
	SetFloorGenerator();
	SetFloorTile();
	SetWallTile();
	SetDungeonSize(AccruedWisdom);
}

void ADungeonManager::SetDungeonSize(int WisdomAmount)
{
	DungeonSize = WisdomAmount * 20;
}

AFloorGeneratorBase* ADungeonManager::SpawnDungeonModule()
{
	FTransform SpawnTransform = FTransform::Identity;

	//Create the FloorGenerator using SpawnActorDeferred to allow other setup actions to occur
	AFloorGeneratorBase* FloorGenerator = GetWorld()->SpawnActorDeferred<AFloorGeneratorBase>(
		SelectedFloorGeneratorClass,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!FloorGenerator) return nullptr;
	
	//Configure before construction
	FloorGenerator->SetMapSize(MapWidth, MapHeight);
	FloorGenerator->SetFloorTile(SelectedFloorTileClass);
	FloorGenerator->SetWallTile(SelectedWallTileClass);
	FloorGenerator->SetDungeonManager(this);

	UGameplayStatics::FinishSpawningActor(FloorGenerator, SpawnTransform);

	//Populate DungneonModule variable and Call GenerateModule after the config is set and the actor is finished spawning
	DungeonModule = FloorGenerator;
	//DungeonModule->GenerateModule();

	return DungeonModule;
}

void ADungeonManager::PopulateDungeon()
{
	TotalEmptySpaces = EmptyLocations.Num();
	SetTreasure();
	SpawnEnemies();
	SpawnBreakables();
	SpawnProps();

	//Set Delay
	GetWorldTimerManager().SetTimer(
    PatrolDelayHandle,
    this,
    &ADungeonManager::StartPatrollingDelayed,
    5.0f,
    false
	);
}

bool ADungeonManager::TryPopRandomEmptyLocation(FVector& OutLocation)
{
	if (EmptyLocations.Num() == 0) return false;

	const int32 Index = FMath::RandRange(0, EmptyLocations.Num() - 1);
	OutLocation = EmptyLocations[Index];
	EmptyLocations.RemoveAtSwap(Index);
	return true;
}

void ADungeonManager::SpawnBreakables()
{
	//Check to see if the EmptyLocations array is empty
	if (EmptyLocations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - EmptyLocations array is... EMPTY!!"));
		return;
	}
	SetNumBreakablesInLevel();

	//Spawn Breakable actors
	for (int i = BreakableQuantityInLevel; i > 0; --i)
	{
		FVector Cell;
		if (!TryPopRandomEmptyLocation(Cell))
		{
			UE_LOG(LogTemp, Warning, TEXT("DungeonManager - ran out of EmptyLocations while spawning breakables."));
			return;
		}

		//Move the Breakable up a bit so it doesn't fall through the floor
		const FVector Location = FVector(Cell.X, Cell.Y, Cell.Z + 10.f);

		//Create a random float for the object's yaw rotation
		float RandomYaw = FMath::RandRange(0.f, 360.f);
		//Create the Rotator
		FRotator Rotation =  FRotator(0.f, RandomYaw, 0.f);
		//Declare the FTransform
		const FTransform SpawnTransform(Rotation, Location);

		SetBreakable();

		//Create the BreakableActor using SpawnActorDeferred to allow other setup actions to occur
		ABreakableActor* Breakable = GetWorld()->SpawnActorDeferred<ABreakableActor>(
			SelectedBreakableClass,
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (!Breakable) return;

		SpawnedActors.Add(Breakable);

		Breakable->SetTreasureClass(SelectedTreasureClass);

		UGameplayStatics::FinishSpawningActor(Breakable, SpawnTransform);
	}
}

void ADungeonManager::SpawnProps()
{	
	//Check to see if the EmptyLocations array is empty
	if (EmptyLocations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - EmptyLocations array is... EMPTY!!"));
		return;
	}

	SetNumPropsInLevel();

	//Spawn Props
	for (int i = PropQuantityInLevel; i > 0; --i)
	{
		FVector Cell;
		if (!TryPopRandomEmptyLocation(Cell))
		{
			UE_LOG(LogTemp, Warning, TEXT("DungeonManager - ran out of EmptyLocations while spawning props."));
			return;
		}

		//Move the Breakable up a bit so it doesn't fall through the floor
		const FVector Location = FVector(Cell.X, Cell.Y, Cell.Z + 10.f);
		//Create a random float for the object's z rotation
		float RandomYaw = FMath::RandRange(0.f, 360.f);
		//Create the Rotator
		FRotator Rotation =  FRotator(0.f, RandomYaw, 0.f);
		//Declare the FTransform
		const FTransform SpawnTransform(FRotator::ZeroRotator, Location);

		SetProp();

		//Create the Prop using SpawnActorDeferred to allow other setup actions to occur
		AProp* Prop = GetWorld()->SpawnActorDeferred<AProp>(
			SelectedPropClass,
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (!Prop) return;

		SpawnedActors.Add(Prop);

		UGameplayStatics::FinishSpawningActor(Prop, SpawnTransform);
	}
}

void ADungeonManager::SpawnEnemies()
{
	
	FEnemyConfig EnemyConfig;

	//Check to see if the EmptyLocations array is empty
	if (EmptyLocations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager - EmptyLocations array is... EMPTY!!"));
		return;
	}

	SetNumEnemiesInLevel();
	UE_LOG(LogTemp, Warning, TEXT("NumEnemiesInLevel = %i"), EnemyQuantityInLevel);

	//Spawn Enemy actors
	for (int i = EnemyQuantityInLevel; i > 0; --i)
	{
		//Set up Enemy variables before spawning it.
		SetEnemy();
		EnemyConfig.EnemyClass = SelectedEnemyClass;
		EnemyConfig.MaxHealth = SetEnemyHealth();
		EnemyConfig.WisdomAmount = SetEnemyWisdom();

		FVector Cell;
		if (!TryPopRandomEmptyLocation(Cell))
		{
			UE_LOG(LogTemp, Warning, TEXT("DungeonManager - ran out of EmptyLocations while spawning enemies."));
			return;
		}

		//Set the Enemy's location
		const FVector Location = FVector(Cell.X, Cell.Y, 10.f);

		//Create a random float for the object's z rotation
		const float RandomYaw = FMath::RandRange(0.f, 360.f);
		//Create the Rotator
		const FRotator Rotation = FRotator(0.f, RandomYaw, 0.f );

		//Declare the FTransform
		FTransform SpawnTransform(Rotation, Location);

		// Force floor Z
		FVector Loc = SpawnTransform.GetLocation();
		Loc.Z = 0.f;
		SpawnTransform.SetLocation(Loc);

		//Create the Enemy using SpawnActorDeferred to allow other setup actions to occur
		AEnemy* Enemy = GetWorld()->SpawnActorDeferred<AEnemy>(
			SelectedEnemyClass,
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (!Enemy) continue;
		
		//Configure before construction
		Enemy->Initialize(EnemyConfig);
		//SetEnemyPatrolPoints(Enemy);
		SetEnemyAttackTimer(Enemy);
		SetEnemyWeaponDamage(Enemy);
		SpawnedEnemies.Add(Enemy);

		//Raise the enemy up a bit so the capsule rests on the floor plane
		float HalfHeight = 0.f;
		if (UCapsuleComponent* Capsule = Enemy->GetCapsuleComponent())
		{
			HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		//Adjust SpawnTransform Z
		FVector Adjusted = SpawnTransform.GetLocation();
		Adjusted.Z = 0.f + HalfHeight;
		SpawnTransform.SetLocation(Adjusted);

		UGameplayStatics::FinishSpawningActor(Enemy, SpawnTransform);

		// //Start patrolling after spawn has finished
		// Enemy->StartPatrolling();
	}
}

void ADungeonManager::SetNumBreakablesInLevel()
{
	BreakableQuantityInLevel = TotalEmptySpaces / 30;
}

void ADungeonManager::SetNumPropsInLevel()
{
	PropQuantityInLevel = TotalEmptySpaces / 7;
}

void ADungeonManager::SetNumEnemiesInLevel()
{
	EnemyQuantityInLevel = 1+ (TotalEmptySpaces / 50) + (AccruedWisdom / 10);
}

int ADungeonManager::SetEnemyHealth()
{
	return EnemyHealth = FMath::RandRange(25, 100);
}

int ADungeonManager::SetEnemyWisdom()
{
	return EnemyWisdom = EnemyHealth / 5;
}

void ADungeonManager::SetEnemyPatrolPoints(AEnemy* Enemy)
{
	if (!Enemy) return;
	if (SpawnedActors.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough spawned actors. Only %i"), SpawnedActors.Num());
		return;
	}
	TArray<AActor*> SpawnedActorsToChooseFrom = SpawnedActors;

	int RandIndex1 = FMath::RandRange(0, SpawnedActorsToChooseFrom.Num() - 1);
	AActor* PatrolActor1 = SpawnedActorsToChooseFrom[RandIndex1];
	SpawnedActorsToChooseFrom.RemoveAtSwap(RandIndex1);

	int RandIndex2 = FMath::RandRange(0, SpawnedActorsToChooseFrom.Num() - 1);
	AActor* PatrolActor2 = SpawnedActorsToChooseFrom[RandIndex2];
	SpawnedActorsToChooseFrom.RemoveAtSwap(RandIndex2);

	Enemy->SetPatrolPoints(PatrolActor1, PatrolActor2);
}

void ADungeonManager::StartPatrollingDelayed()
{
	for (AEnemy* Enemy : SpawnedEnemies)
	{
		SetEnemyPatrolPoints(Enemy);
		Enemy->StartPatrolling();
	}
}

void ADungeonManager::SetEnemyAttackTimer(AEnemy* Enemy)
{
	float TimerMin = 1.f - (AccruedWisdom / 100.f);
	float TimerMax = 1.5f - (AccruedWisdom / 200.f);
	Enemy->SetAttackTimer(TimerMin, TimerMax);
}

void ADungeonManager::SetEnemyWeaponDamage(AEnemy* Enemy)
{
	if (!Enemy) return;

	AWeapon* Weapon = Enemy->GetEquippedWeapon();
	if (!Weapon) return;

	float NewDamage = Weapon->GetWeaponDamage();
	NewDamage += (AccruedWisdom * 0.01f);

	Weapon->SetWeaponDamage(NewDamage);
}

EDungeonTheme ADungeonManager::GetRandomDungeonTheme() const
{
	if (DungeonThemes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonManager::GetRandomDungeonTheme - DungeonThemes is empty."));
        return EDungeonTheme::EDT_Dungeon;
	}

	return DungeonThemes[FMath::RandRange(0, DungeonThemes.Num() - 1)];	
}

TSubclassOf<AFloorGeneratorBase> ADungeonManager::GetRandomFloorGenerator()
{
	if (!bThemedFloorGeneratorsPopulated)
	{
		for (TSubclassOf<AFloorGeneratorBase> FloorGenerator : FloorGenerators)
		{
			if (!FloorGenerator) continue;

			//Class Default Object
			const AFloorGeneratorBase* CDO = FloorGenerator->GetDefaultObject<AFloorGeneratorBase>();
			if (!CDO) continue;

			if (CDO->GetThemes().Contains(DungeonTheme))
			{
				ThemedFloorGenerators.Add(FloorGenerator);
			}
		}
		
		bThemedFloorGeneratorsPopulated = true;
	}
	
	if (ThemedFloorGenerators.Num() == 0)
	{
		return nullptr;
	}

	return ThemedFloorGenerators[FMath::RandRange(0, ThemedFloorGenerators.Num() - 1)];	
}

TSubclassOf<AFloorTile> ADungeonManager::GetRandomFloorTile()
{
	if (!bThemedFloorTilesPopulated)
	{
		for (TSubclassOf<AFloorTile> Tile : FloorTiles)
		{
			if (!Tile) continue;

			//Class Default Object
			const AFloorTile* CDO = Tile->GetDefaultObject<AFloorTile>();
			if (!CDO) continue;

			if (CDO->GetThemes().Contains(DungeonTheme))
			{
				ThemedFloorTiles.Add(Tile);
			}
		}
		
		bThemedFloorTilesPopulated = true;
	}
	
	if (ThemedFloorTiles.Num() == 0)
	{
		return nullptr;
	}

	return ThemedFloorTiles[FMath::RandRange(0, ThemedFloorTiles.Num() - 1)];	
}

TSubclassOf<AWallTile> ADungeonManager::GetRandomWallTile()
{
	if (!bThemedWallTilesPopulated)
	{
		for (TSubclassOf<AWallTile> Tile : WallTiles)
		{
			if (!Tile) continue;

			//Class Default Object
			const AWallTile* CDO = Tile->GetDefaultObject<AWallTile>();
			if (!CDO) continue;

			if (CDO->GetThemes().Contains(DungeonTheme))
			{
				ThemedWallTiles.Add(Tile);
			}
		}
		bThemedWallTilesPopulated = true;
	}

	if (ThemedWallTiles.Num() == 0)
	{
		return nullptr;
	}

	return ThemedWallTiles[FMath::RandRange(0, ThemedWallTiles.Num() - 1)];
}

TSubclassOf<ATreasure> ADungeonManager::GetRandomTreasure()
{
	if (!bThemedTreasuresPopulated)
	{
		for (TSubclassOf<ATreasure> Treasure : Treasures)
		{
			if (!Treasure) continue;

			//Class Default Object
			const ATreasure* CDO = Treasure->GetDefaultObject<ATreasure>();
			if (!CDO) continue;

			ThemedTreasures.Add(Treasure);
		}
		bThemedTreasuresPopulated = true;
	}

	if (ThemedTreasures.Num() == 0)
	{
		return nullptr;
	}

	return ThemedTreasures[FMath::RandRange(0, ThemedTreasures.Num() - 1)];
}

TSubclassOf<ABreakableActor> ADungeonManager::GetRandomBreakable()
{
	if (!bThemedBreakablesPopulated)
	{
		for (TSubclassOf<ABreakableActor> Breakable : Breakables)
		{
			if (!Breakable) continue;

			//Class Default Object
			const ABreakableActor* CDO = Breakable->GetDefaultObject<ABreakableActor>();
			if (!CDO) continue;

			if (CDO->GetThemes().Contains(DungeonTheme))
			{
				ThemedBreakables.Add(Breakable);
			}
		}
		bThemedBreakablesPopulated = true;
	}
	
	if (ThemedBreakables.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Themedbreakables.num = 0"));
		return nullptr;
	}

	return ThemedBreakables[FMath::RandRange(0, ThemedBreakables.Num() - 1)];
}

TSubclassOf<AEnemy> ADungeonManager::GetRandomEnemy()
{
	if (!bThemedEnemiesPopulated)
	{
		for (TSubclassOf<AEnemy> Enemy : Enemies)
		{
			if (!Enemy) continue;

			//Class Default Object
			const AEnemy* CDO = Enemy->GetDefaultObject<AEnemy>();
			if (!CDO) continue;

			if (CDO->GetThemes().Contains(DungeonTheme))
			{
				ThemedEnemies.Add(Enemy);
			}
		}
		bThemedEnemiesPopulated = true;
	}
	
	if (ThemedEnemies.Num() == 0)
	{
		return nullptr;
	}

	return ThemedEnemies[FMath::RandRange(0, ThemedEnemies.Num() - 1)];
}

TSubclassOf<AProp> ADungeonManager::GetRandomProp()
{
	if (!bThemedPropsPopulated)
	{
		for (TSubclassOf<AProp> Prop : Props)
		{
			if (!Prop) continue;

			//Class Default Object
			const AProp* CDO = Prop->GetDefaultObject<AProp>();
			if (!CDO) continue;

			if (CDO->GetThemes().Contains(DungeonTheme))
			{
				ThemedProps.Add(Prop);
			}
		}
		bThemedPropsPopulated = true;
	}
	
	if (ThemedProps.Num() == 0)
	{
		return nullptr;
	}

	return ThemedProps[FMath::RandRange(0, ThemedProps.Num() - 1)];
}

FString ADungeonManager::GetSelectedThemeText() const
{
	switch(DungeonTheme)
	{
		case EDungeonTheme::EDT_Castle:
			return TEXT("Castle");
		case EDungeonTheme::EDT_Fort:
			return TEXT("Fort");
		case EDungeonTheme::EDT_Cave:
			return TEXT("Cave");
		case EDungeonTheme::EDT_Dungeon:
			return TEXT("Dungeon");
		case EDungeonTheme::EDT_Mansion:
			return TEXT("Mansion");
		case EDungeonTheme::EDT_Crypt:
			return TEXT("Crypt");
		case EDungeonTheme::EDT_Temple:
			return TEXT("Temple");
		case EDungeonTheme::EDT_Ruins:
			return TEXT("Ruins");
		case EDungeonTheme::EDT_Palace:
			return TEXT("Palace");
		case EDungeonTheme::EDT_City_Streets:
			return TEXT("City Streets");
		default:
			return TEXT("Dungeon");
	}
}

FString ADungeonManager::GetSelectedTreasureText() const
{
	if (SelectedTreasureClass)
	{
		const ATreasure* TreasureCDO = SelectedTreasureClass->GetDefaultObject<ATreasure>();
		if (TreasureCDO)
		{
			return TreasureCDO->GetDisplayName();
		}
	}

	return TEXT("Treasure");
}