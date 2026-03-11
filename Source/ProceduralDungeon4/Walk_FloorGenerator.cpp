// Fill out your copyright notice in the Description page of Project Settings.


#include "Walk_FloorGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AWalk_FloorGenerator::AWalk_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AWalk_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWalk_FloorGenerator::GenerateModule()
{
	GeneratedEmptyLocations.Reset();
	GenerateMap();
	SpawnGeometry();

	const int32 DoorCount = (DesiredExteriorDoors > 0) ? DesiredExteriorDoors : DefaultDoorCount;

	CreateDoors(DoorCount);

	bHasFinishedGenerating = true;
}

// Called every frame
void AWalk_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWalk_FloorGenerator::GenerateMap()
{
	const int32 NumCells = MapWidth * MapHeight;
	if (NumCells <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Walk_FloorGenerator: invalid map size."));
		return;
	}

	InitializeMap();
	RunRandomWalk();
}

void AWalk_FloorGenerator::InitializeMap()
{
	const int32 NumCells = MapWidth * MapHeight;
	Map.SetNum(NumCells);

	//Start with all walls
	for (int32 i = 0; i < NumCells; ++i)
	{
		//Wall
		Map[i] = true;
	}

	//RNG Setup
	FRandomStream Rng;
	if (Seed >= 0)
	{
		Rng.Initialize(Seed);
	}
	else
	{
		//Non-deterministic RNG
		Rng.GenerateNewSeed();
	}
}

void AWalk_FloorGenerator::RunRandomWalk()
{
	if (MapWidth <= 2 || MapHeight <= 2) return;

	int32 X, Y;

	if (bStartInCenter)
	{
		X = MapWidth / 2;
		Y = MapHeight / 2;
	}
	else
	{
		X = FMath::RandRange(1, MapWidth - 2);
		Y = FMath::RandRange(1, MapHeight - 2);
	}

	//Floor
	Map[Index(X, Y)] = false;

	for (int32 Step = 0; Step < NumSteps; ++Step)
	{
		int32 Dir = FMath::RandRange(0, 3);

		switch(Dir)
		{
			case 0:
				X++;
				break;
			case 1:
				X--;
				break;
			case 2:
				Y++;
				break;
			case 3:
				Y--;
				break;
			default:
				break;
		}

		//Keep within bound with 1-cell border of walls
		X = FMath::Clamp(X, 1, MapWidth - 2);
		Y = FMath::Clamp(Y, 1, MapHeight - 2);

		//Carve floor
		Map[Index(X, Y)] = false;
	}
}

void AWalk_FloorGenerator::SpawnGeometry()
{
	UWorld* World = GetWorld();
	if(!World) return;

	if (!FloorTileClass && !WallTileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Walk_FloorGenerator: no tiles assigned"));
		return;
	}

	const float BasePlaneSize = 100.f;

	for (int32 y = 0; y < MapHeight; ++y)
	{
		for (int32 x = 0; x < MapWidth; ++x)
		{
			const bool bIsWall = Map[Index(x, y)];

			const FVector CellWorld = GetActorLocation() + FVector(x * TileSize, y * TileSize, 0.f);

			//Floor
			if (!bIsWall && FloorTileClass)
			{
				const FVector Pos = CellWorld + FVector(0.f, 0.f, FloorZ);

				FActorSpawnParameters Params;
				Params.Owner = this;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FTransform SpawnTransform(FRotator::ZeroRotator, Pos);

				AFloorTile* FloorActor = World->SpawnActor<AFloorTile>(FloorTileClass, Pos, FRotator::ZeroRotator, Params);

				if (!FloorActor) continue;

				UStaticMeshComponent* MeshComp = FloorActor->GetItemMesh();
				if(!MeshComp)
				{
					UE_LOG(LogTemp, Warning, TEXT("No Mesh Component!"));
					FloorActor->Destroy();
					continue;
				}

				MeshComp->SetMobility(EComponentMobility::Movable);
				FloorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

				//MeshComp->SetStaticMesh(FloorMesh);

				const float Scale = TileSize / BasePlaneSize;
				FloorActor->SetActorScale3D(FVector(Scale, Scale, 1.f));
				GeneratedEmptyLocations.Add(Pos);
			}
			//Walls
			else if (bIsWall && WallTileClass)
			{
				if(!HasFloorNeighbor(x, y)) continue;

				const FVector Pos = CellWorld + FVector(0.f, 0.f, FloorZ + WallHeight * 0.f);

				FActorSpawnParameters Params;
				Params.Owner = this;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FTransform SpawnTransform(FRotator::ZeroRotator, Pos);

				AWallTile* WallActor = World->SpawnActor<AWallTile>(WallTileClass, SpawnTransform, Params);

				if (!WallActor) continue;

				UStaticMeshComponent* MeshComp = WallActor->GetItemMesh();
				if (!MeshComp)
				{
					WallActor->Destroy();
					continue;
				}

				MeshComp->SetMobility(EComponentMobility::Movable);
				WallActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

				//MeshComp->SetStaticMesh(WallMesh);

				const float XYScale = TileSize / BasePlaneSize;
				const float ZScale = WallHeight / BasePlaneSize;
				WallActor->SetActorScale3D(FVector(XYScale, XYScale, ZScale));
				
				//Check how many neighbors the floor cell has, to determine if it's an outer wall cell or not
				int32 FloorNeighbors = 0;
				FIntPoint InteriorFloorCell(-1, -1);

				if (x + 1 < MapWidth  && !Map[Index(x + 1, y)])
				{
					++FloorNeighbors;
					InteriorFloorCell = FIntPoint(x + 1, y);
				}
				if (x - 1 >= 0        && !Map[Index(x - 1, y)])
				{
					++FloorNeighbors;
					InteriorFloorCell = FIntPoint(x - 1, y);
				}
				if (y + 1 < MapHeight && !Map[Index(x, y + 1)])
				{
					++FloorNeighbors;
					InteriorFloorCell = FIntPoint(x, y + 1);
				}
				if (y - 1 >= 0        && !Map[Index(x, y - 1)])
				{
					++FloorNeighbors;
					InteriorFloorCell = FIntPoint(x, y - 1);
				}

				//Outer walls only have 1 neighbor
				if (FloorNeighbors == 1)
				{
					FDungeonWallSegment Seg;
					Seg.Cell = FIntPoint(x, y);
					Seg.Direction = 0;
					Seg.WallActor = WallActor;
					WallSegments.Add(Seg);
				}
				
			}
		}
	}

}

bool AWalk_FloorGenerator::HasFloorNeighbor(int32 X, int32 Y) const
{
	//4-connected neighbors NSWE
	const int32 DX[4] = {1, -1, 0, 0};
	const int32 DY[4] = {0, 0, 1, -1};

	for (int32 i = 0; i < 4; ++i)
	{
		int32 NX = X + DX[i];
		int32 NY = Y + DY[i];

		if (NX < 0 || NY < 0 || NX >= MapWidth || NY >= MapHeight)
		{
			continue;
		}

		//false = floor
		if (!Map[Index(NX, NY)])
		{
			return true;
		}
	}

	return false;
}

void AWalk_FloorGenerator::CreateDoors(int32 DoorCount)
{
	if (DoorCount <= 0) return;

	if (WallSegments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Walk_FloorGenerator: No wall segments to carve doors from!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	//Clamp to existing walls
	DoorCount = FMath::Min(DoorCount, WallSegments.Num());

	//Reuse Seed so doors are deterministic relative to layout, but offset so it doesn't affect shape generation
	FRandomStream Rng;
	if (Seed >= 0)
	{
		Rng.Initialize(Seed + 1337);
	}
	else
	{
		Rng.GenerateNewSeed();
	}

	for (int32 d = 0; d < DoorCount && WallSegments.Num() > 0; ++d)
	{
		const int32 Index = Rng.RandRange(0, WallSegments.Num() - 1);
		FDungeonWallSegment Seg = WallSegments[Index];

		AWallTile* WallActor = Seg.WallActor.Get();
		if (!WallActor)
		{
			//Dead pointer, discard and retry
			WallSegments.RemoveAtSwap(Index);
			--d;
			continue;
		}

		const FTransform WallTransform = WallActor->GetActorTransform();

		//Record the door so DungeonManager can use it
		// FExteriorDoor DoorInfo;
		// DoorInfo.Location = WallTransform.GetLocation();
		// DoorInfo.Rotation = WallTransform.GetRotation().Rotator();
		// ExteriorDoors.Add(DoorInfo);
		AddExteriorDoorWorld(WallTransform.GetLocation(), WallTransform.GetRotation().Rotator());

		//Remove the wall section
		WallActor->Destroy();
		WallSegments.RemoveAtSwap(Index);
		//Spawn a door mesh
		if (WallTileClass)
		{

			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (WallTileClass)
			{
				AWallTile* DoorActor = World->SpawnActor<AWallTile>(WallTileClass, WallTransform.GetLocation(), WallTransform.GetRotation().Rotator(), Params);

				if (DoorActor)
				{

					if (UStaticMeshComponent* DoorComp = DoorActor->GetItemMesh())
					{
						DoorActor->SetActorScale3D(WallTransform.GetScale3D());

						DoorComp->SetMobility(EComponentMobility::Movable);
						DoorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

						GeneratedDoorActors.Add(DoorActor);
					}
					else
					{
						DoorActor->Destroy();
					}
				}

			}
		}
	}
}