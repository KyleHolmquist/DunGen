// Fill out your copyright notice in the Description page of Project Settings.


#include "Holmquist_FloorGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AHolmquist_FloorGenerator::AHolmquist_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AHolmquist_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateModule();
	
}

void AHolmquist_FloorGenerator::GenerateModule()
{
	//Allocate Grid
	GenerateRoomLayout();
	SpawnFloorTiles();

	const int32 DoorCount = (DesiredExteriorDoors > 0) ? DesiredExteriorDoors : DefaultDoorCount;

	CreateDoors(DoorCount);

	bHasFinishedGenerating = true;
}

// Called every frame
void AHolmquist_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHolmquist_FloorGenerator::GenerateRoomLayout()
{
	//Clear state
	const int32 NumCells = MapWidth * MapHeight;
	if (NumCells <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: Invalid grid size"));
		return;
	}

	Grid.Init(false, NumCells);
	Frontier.Reset();

	//RNG Setup
	FRandomStream Rng;
	if (Seed >= 0)
	{
		Rng.Initialize(Seed);
	}
	else
	{
		//Non-deterministic
		Rng.GenerateNewSeed();
	}

	//Choose starting cell at center of grid
	const int32 StartX = MapWidth / 2;
	const int32 StartY = MapHeight / 2;

	Grid[Index(StartX, StartY)] = true;
	Frontier.Add(FIntPoint(StartX, StartY));

	int32 TilesPlaced = 1;
	const int32 TargetTiles = FMath::Clamp(NumTiles, 1, NumCells);

	//Grow the room
	while (TilesPlaced < TargetTiles && Frontier.Num() > 0)
	{
		//Choose a random floor cell to grow from
		const int32 FrontierIndex = Rng.RandRange(0, Frontier.Num() - 1);
		const FIntPoint Cell = Frontier[FrontierIndex];
		
		const int32 X = Cell.X;
		const int32 Y = Cell.Y;

		//Gather empty neighbors around the cell
		TArray<FIntPoint> EmptyNeighbors;

		const int32 DX[4] = {1, -1, 0, 0};
		const int32 DY[4] = {0, 0, 1, -1};

		for (int32 i = 0; i < 4; ++i)
		{
			const int32 NX = X + DX[i];
			const int32 NY = Y + DY[i];

			//Stay inside the grid
			if (NX < 0 || NX >= MapWidth || NY < 0 || NY >= MapHeight) continue;

			if (!Grid[Index(NX, NY)])
			{
				EmptyNeighbors.Add(FIntPoint(NX, NY));
			}
		}

		if (EmptyNeighbors.Num() == 0)
		{
			//This floor cell has no empty neighbors. Remove from Frontier array
			Frontier.RemoveAtSwap(FrontierIndex);
			continue;
		}

		//Choose a random empty neighbor
		const int32 RandEmptyIndex = Rng.RandRange(0, EmptyNeighbors.Num() - 1);
		const FIntPoint NewCell = EmptyNeighbors[RandEmptyIndex];

		//Carve floor
		Grid[Index(NewCell.X, NewCell.Y)] = true;
		Frontier.Add(NewCell);

		++TilesPlaced;
	}

	UE_LOG(LogTemp, Log, TEXT("Holmquist_FloorGenerator: Placed %d floor tiles (target %d)."),
			TilesPlaced, TargetTiles);
}

void AHolmquist_FloorGenerator::SpawnFloorTiles()
{
	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: FloorMesh not assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const float BaseSize = 100.f;
	const float HalfTile = TileSize * 0.5f;

	//---- Floors and Edge Walls ----

	for (int32 y = 0; y < MapHeight; ++y)
	{
		for (int32 x = 0; x < MapWidth; ++x)
		{
			const bool bIsFloor = Grid[Index(x, y)];
			const FVector TileCenter = (GetActorLocation() + FVector(x * TileSize, y * TileSize, 0.f));

			//---- Floor ----
			if (bIsFloor && FloorMesh)
			{
				const FVector FloorPos = TileCenter + FVector(0.f, 0.f, FloorZ);
				AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(FloorPos, FRotator::ZeroRotator);
				if (!FloorActor) continue;

				FloorActor->SetMobility(EComponentMobility::Movable);
				FloorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

				UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent();
				if (!MeshComp)
				{
					FloorActor->Destroy();
					continue;
				}
				
				MeshComp->SetStaticMesh(FloorMesh);

				const float FloorScale = TileSize / BaseSize;
				FloorActor->SetActorScale3D(FVector(FloorScale, FloorScale, 1.f));
			}

			//---- Walls around Floor ----
			if (!bIsFloor || !WallMesh)
			{
				continue;
			}

			// Helper lambda to ask "is there floor at (NX, NY)?"
			auto HasFloorAt = [&](int32 NX, int32 NY) -> bool
			{
				if (NX < 0 || NX >= MapWidth || NY < 0 || NY >= MapHeight)
				{
					return false;
				}
				return Grid[Index(NX, NY)];
			};

			// EAST (+X) edge  → vertical wall (length along Y)
			{
				const bool bHasEastFloor = HasFloorAt(x + 1, y);
				if (!bHasEastFloor)
				{
					const FVector WallPos = TileCenter + FVector(HalfTile, 0.f, FloorZ + WallHeight * 0.f);
					const FRotator Rot(0.f, 90.f, 0.f);

					AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
					if (WallActor)
					{
						WallActor->SetMobility(EComponentMobility::Movable);
						WallActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

						UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
						if (!WComp)
						{
							WallActor->Destroy();
						}
						else
						{
							WComp->SetStaticMesh(WallMesh);

							const float ScaleX = TileSize      / BaseSize; // length
							const float ScaleY = WallThickness / BaseSize; // thickness
							const float ScaleZ = WallHeight    / BaseSize; // height

							WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));

							FDungeonWallSegment Seg;
							Seg.Cell = FIntPoint(x, y);
							Seg.Direction = 0;
							Seg.WallActor = WallActor;
							WallSegments.Add(Seg);
						}
					}
				}

				// WEST (-X) edge → vertical wall
				{
					const bool bHasWestFloor = HasFloorAt(x - 1, y);
					if (!bHasWestFloor)
					{
						const FVector WallPos = TileCenter + FVector(-HalfTile, 0.f, FloorZ + WallHeight * 0.f);
						const FRotator Rot(0.f, 90.f, 0.f);

						AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
						if (WallActor)
						{
							WallActor->SetMobility(EComponentMobility::Movable);
							WallActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

							UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
							if (!WComp)
							{
								WallActor->Destroy();
							}
							else
							{
								WComp->SetStaticMesh(WallMesh);

								const float ScaleX = TileSize      / BaseSize;
								const float ScaleY = WallThickness / BaseSize;
								const float ScaleZ = WallHeight    / BaseSize;

								WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));

							FDungeonWallSegment Seg;
							Seg.Cell = FIntPoint(x, y);
							Seg.Direction = 1;
							Seg.WallActor = WallActor;
							WallSegments.Add(Seg);
							}
						}
					}
				}

				// NORTH (+Y) edge → horizontal wall (length along X)
				{
					const bool bHasNorthFloor = HasFloorAt(x, y + 1);
					if (!bHasNorthFloor)
					{
						const FVector WallPos = TileCenter + FVector(0.f, HalfTile, FloorZ + WallHeight * 0.f);
						const FRotator Rot(0.f, 0.f, 0.f);

						AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
						if (WallActor)
						{
							WallActor->SetMobility(EComponentMobility::Movable);
							WallActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

							UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
							if (!WComp)
							{
								WallActor->Destroy();
							}
							else
							{
								WComp->SetStaticMesh(WallMesh);

								const float ScaleX = TileSize      / BaseSize;
								const float ScaleY = WallThickness / BaseSize;
								const float ScaleZ = WallHeight    / BaseSize;

								WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));

							FDungeonWallSegment Seg;
							Seg.Cell = FIntPoint(x, y);
							Seg.Direction = 2;
							Seg.WallActor = WallActor;
							WallSegments.Add(Seg);
							}
						}
					}
				}

				// SOUTH (-Y) edge → horizontal wall
				{
					const bool bHasSouthFloor = HasFloorAt(x, y - 1);
					if (!bHasSouthFloor)
					{
						const FVector WallPos = TileCenter + FVector(0.f, -HalfTile, FloorZ + WallHeight * 0.f);
						const FRotator Rot(0.f, 0.f, 0.f);

						AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
						if (WallActor)
						{
							WallActor->SetMobility(EComponentMobility::Movable);
							WallActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

							UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
							if (!WComp)
							{
								WallActor->Destroy();
							}
							else
							{
								WComp->SetStaticMesh(WallMesh);

								const float ScaleX = TileSize      / BaseSize;
								const float ScaleY = WallThickness / BaseSize;
								const float ScaleZ = WallHeight    / BaseSize;

								WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));

							FDungeonWallSegment Seg;
							Seg.Cell = FIntPoint(x, y);
							Seg.Direction = 3;
							Seg.WallActor = WallActor;
							WallSegments.Add(Seg);
							}
						}
					}
				}
			}
		}
	}
}

void AHolmquist_FloorGenerator::CreateDoors(int32 DoorCount)
{
	if (DoorCount <= 0) return;

	if (WallSegments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: No wall segments to carve doors from!"));
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

		AStaticMeshActor* WallActor = Seg.WallActor.Get();
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
		if (DoorMesh)
		{
			AStaticMeshActor* DoorActor = World->SpawnActor<AStaticMeshActor>(WallTransform.GetLocation(), WallTransform.GetRotation().Rotator());

			if (DoorActor)
			{
				DoorActor->SetMobility(EComponentMobility::Movable);
				DoorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

				if (UStaticMeshComponent* DoorComp = DoorActor->GetStaticMeshComponent())
				{
					DoorComp->SetStaticMesh(DoorMesh);
					DoorActor->SetActorScale3D(WallTransform.GetScale3D());

					//Record the Door's info to ExteriorDoors array
					FExteriorDoor Door;
					Door.Location = DoorActor->GetActorLocation();
					Door.Rotation = DoorActor->GetActorRotation();
					ExteriorDoors.Add(Door);
				}
				else
				{
					DoorActor->Destroy();
				}
			}
		}
	}
}