// Fill out your copyright notice in the Description page of Project Settings.


#include "CA_FloorGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACA_FloorGenerator::ACA_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACA_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateModule();
	
}

void ACA_FloorGenerator::GenerateModule()
{
	InitializeMap();
	RunSimulation();
	EnsureConnectivity();
	SpawnGeometry();
	CreateDoors(DefaultDoorCount);
}

void ACA_FloorGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // No per-frame logic needed for now
}

void ACA_FloorGenerator::InitializeMap()
{
	const int32 NumCells = MapWidth * MapHeight;
	CurrentMap.SetNum(NumCells);
	NextMap.SetNum(NumCells);

	for(int32 y = 0; y < MapHeight; ++y)
	{
		for(int32 x = 0; x < MapWidth; ++x)
		{
			bool bIsWall = false;

			//Force borders to be walls so the cave is closed
			if (x == 0 || y == 0 || x == MapWidth - 1 || y == MapHeight - 1)
			{
				bIsWall = true;
			}
			else
			{
				const int32 Rand = FMath::RandRange(0, 100);
				bIsWall = (Rand < InitWallChance);
			}

			CurrentMap[Index(x, y)] = bIsWall;
		}
		
	}
}

void ACA_FloorGenerator::RunSimulation()
{
	for(int32 i = 0; i < SimulationSteps; ++i)
	{
		StepSimulation();
		CurrentMap = NextMap;
	}
}

void ACA_FloorGenerator::StepSimulation()
{
	for (int32 y = 0; y < MapHeight; ++y)
	{
		for(int32 x = 0; x < MapWidth; ++x)
		{
			const int32 Neighbors = CountWallNeighbors(x, y);
			const bool bCurrentWall = CurrentMap[Index(x, y)];

			bool bNewWall = bCurrentWall;

			if (bCurrentWall)
			{
				//Too few wall neighbors => becomes floor
				if (Neighbors < DeathLimit)
				{
					bNewWall = false;
				}
				else
				{
					bNewWall = true;
				}
			}
			else
			{
				//Emough wall neighbors => becomes wall
				if (Neighbors > BirthLimit)
				{
					bNewWall = true;
				}
				else
				{
					bNewWall = false;
				}
			}

			NextMap[Index(x, y)] = bNewWall;
		}
	}
}

int32 ACA_FloorGenerator::CountWallNeighbors(int32 X, int32 Y) const
{
	int32 Count = 0;

	for (int32 ny = Y - 1; ny <= Y + 1; ++ny)
	{
		for (int32 nx = X - 1; nx <= X + 1; ++nx)
		{
			//Skip self
			if (nx == X && ny == Y) continue;

			//Treat out of bounds as wall to help close cave
			if (nx < 0 || ny < 0 || nx >= MapWidth || ny >= MapHeight)
			{
				Count++;
			}
			else
			{
				if (CurrentMap[Index(nx, ny)])
				{
					Count++;
				}
			}
		}
	}
	
	return Count;
}

void ACA_FloorGenerator::SpawnGeometry()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("CA_FloorGenerator: FloorMesh is null"));
	}

	const float BasePlaneSize = 100.f;

	//Track all wall actors by grid
	WallActorMap.Init(nullptr, MapWidth * MapHeight);

	for (int32 y = 0; y < MapHeight; ++y)
	{
		for (int32 x = 0; x < MapWidth; ++x)
		{
			const bool bIsWall = CurrentMap[Index(x, y)];

			//Spawn floor wheere there is no wall
			if (!bIsWall && FloorMesh)
			{
				const FVector WorldPos = GetActorLocation() + FVector(x * TileSize, y * TileSize, FloorZ);

				AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(WorldPos, FRotator::ZeroRotator);
				if (!FloorActor) continue;

				UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent();
				if (!MeshComp)
				{
					FloorActor->Destroy();
					continue;
				}

				MeshComp->SetStaticMesh(FloorMesh);

				//Center the tile
				const float ScaleFactor = TileSize / BasePlaneSize;
				FloorActor->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, 1.f));
				FloorActor->SetMobility(EComponentMobility::Static);
			}

			//Spawn wall mesh where there's a wall
			if (bIsWall && WallMesh)
			{
				const FVector WallPos = GetActorLocation() + FVector(x * TileSize, y * TileSize, FloorZ);

				AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, FRotator::ZeroRotator);
				if (!WallActor) continue;

				UStaticMeshComponent* MeshComp = WallActor->GetStaticMeshComponent();
				if (!MeshComp)
				{
					WallActor->Destroy();
					continue;
				}

				MeshComp->SetStaticMesh(WallMesh);

				//Assume WallMesh is 100x100x100 cube, scale to TileSize and WallHeight
				const float XYScale = TileSize / BasePlaneSize;
				const float ZScale = WallHeight / BasePlaneSize;

				WallActor->SetActorScale3D(FVector(XYScale, XYScale, ZScale));
				WallActor->SetMobility(EComponentMobility::Static);

				//Remember this wall actor at (x, y)
				WallActorMap[Index(x, y)] = WallActor;

				// Count neighboring floor cells, which is when CurrentMap == false
				int32 FloorNeighbors = 0;
				FIntPoint InteriorFloorCell(-1, -1);

				// Check 4-connected neighbours
				const int32 DX[4] = { 1, -1, 0,  0 };
				const int32 DY[4] = { 0,  0, 1, -1 };

				for (int32 i = 0; i < 4; ++i)
				{
					const int32 NX = x + DX[i];
					const int32 NY = y + DY[i];

					if (NX < 0 || NX >= MapWidth || NY < 0 || NY >= MapHeight)
						continue;

					// floor == !CurrentMap
					if (!CurrentMap[Index(NX, NY)])
					{
						++FloorNeighbors;
						InteriorFloorCell = FIntPoint(NX, NY);
					}
				}

				//Outer walls only have one floor neighbour.
				if (FloorNeighbors == 1)
				{
					FDungeonWallSegment Seg;
					Seg.Cell = InteriorFloorCell;
					Seg.WallCell = FIntPoint(x, y);
					Seg.Direction = 0;
					Seg.WallActor = WallActor;
					WallSegments.Add(Seg);
				}
			}
		}
	}
}

void ACA_FloorGenerator::EnsureConnectivity()
{
	const int32 NumCells = MapWidth * MapHeight;
	if (NumCells == 0) return;

	//Label array: -1=unvisited, >=0=region id
	TArray<int32> Labels;
	Labels.Init(-1, NumCells);

	struct FRegionData { TArray<FIntPoint> Cells; };
	TArray<FRegionData> Regions;

	int32 NextRegionId = 0;

	//Find regions via flood fill on floor cells (CurrentMap = false)
	for(int32 y = 0; y < MapHeight; ++y)
	{
		for(int32 x = 0; x < MapWidth; ++x)
		{
			const int32 Idx = Index(x, y);
			//Wall
			if (CurrentMap[Idx]) continue;
			//Already visited
			if(Labels[Idx] != -1) continue;

			Regions.AddDefaulted();
			FRegionData& Region = Regions.Last();

			FloodFillRegion(x, y, NextRegionId, Labels, Region.Cells);
			++NextRegionId;
		}
	}

	//If there are 0 or 1 regions, nothing to connect
	if (Regions.Num() <= 1) return;

	//Choose the largest region as the main one
	int32 MainRegionIndex = 0;
	int32 MaxSize = Regions[0].Cells.Num();
	for (int32 i = 1; i < Regions.Num(); ++i)
	{
		const int32 Size = Regions[i].Cells.Num();
		if (Size > MaxSize)
		{
			MaxSize = Size;
			MainRegionIndex = i;
		}
	}

	//Grow the set as other regions are connected
	TArray<FIntPoint> MainCells = Regions[MainRegionIndex].Cells;

	//Connect all other regions into the main region
	for (int32 i = 0; i < Regions.Num(); ++i)
	{
		if (i == MainRegionIndex) continue;

		const TArray<FIntPoint>& OtherCells = Regions[i].Cells;

		FIntPoint MainCell;
		FIntPoint OtherCell;

		if (FindClosestPairBetweenRegions(MainCells, OtherCells, MainCell, OtherCell) >= 0)
		{
			CarveCorridorBetween(MainCell, OtherCell);

			//Add OtherCells into main so future regions can connect to them, too
			MainCells.Append(OtherCells);
		}
	}
}

void ACA_FloorGenerator::FloodFillRegion(int32 StartX, int32 StartY, int32 RegionId, TArray<int32>& OutLabels, TArray<FIntPoint>& OutCells) const
{
	TArray<FIntPoint> Stack;
	Stack.Reserve(128);
	Stack.Add(FIntPoint(StartX, StartY));

	while (Stack.Num() > 0)
	{
		const FIntPoint P = Stack.Pop();
		const int32 X = P.X;
		const int32 Y = P.Y;
		const int32 Idx = Index(X, Y);

		if (OutLabels[Idx] != -1) continue;

		if (CurrentMap[Idx]) continue;

		OutLabels[Idx] = RegionId;
		OutCells.Add(P);

		//4-Connected neighbors (NSWE)
		if (X > 0) Stack.Add(FIntPoint(X - 1, Y));
		if (X < MapWidth - 1) Stack.Add(FIntPoint(X + 1, Y));
		if (Y > 0) Stack.Add(FIntPoint(X, Y - 1));
		if (Y < MapHeight - 1) Stack.Add(FIntPoint(X, Y + 1));
	}
}

//Returns the minimum Manhattan distance or -1 if no pair found
int32 ACA_FloorGenerator::FindClosestPairBetweenRegions(const TArray<FIntPoint>& RegionA, const TArray<FIntPoint>& RegionB, FIntPoint& OutA, FIntPoint& OutB) const
{
	int32 BestDist = TNumericLimits<int32>::Max();
	bool bFound = false;

	for(const FIntPoint& A : RegionA)
	{
		for(const FIntPoint& B : RegionB)
		{
			const int32 Dist = FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
			if (Dist < BestDist)
			{
				BestDist = Dist;
				OutA = A;
				OutB = B;
				bFound = true;
			}
		}
	}

	return bFound ? BestDist : -1;
}

void ACA_FloorGenerator::CarveCorridorBetween(const FIntPoint& A, const FIntPoint& B)
{
	FIntPoint Current = A;

	//First walk in X, then in Y for a simple L-shaped corridor
	int32 StepX = (B.X > Current.X) ? 1 : -1;
	int32 StepY = (B.Y > Current.Y) ? 1 : -1;

	while (Current.X != B.X)
	{
		const int32 Idx = Index(Current.X, Current.Y);
		//Floor
		CurrentMap[Idx] = false;

		Current.X += StepX;
	}

	while(Current.Y != B.Y)
	{
		const int32 Idx = Index(Current.X, Current.Y);
		//Floor
		CurrentMap[Idx] = false;

		Current.Y += StepY;
	}

	//Make sure the destination cell is also Floor
	const int32 EndIdx = Index(B.X, B.Y);
	CurrentMap[EndIdx] = false;
}

void ACA_FloorGenerator::CreateDoors(int32 DoorCount)
{
	if (DoorCount <= 0) return;

	if (WallSegments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CA_FloorGenerator: No wall segments to carve doors from!"));
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
		const int32 SegIndex = Rng.RandRange(0, WallSegments.Num() - 1);
		FDungeonWallSegment Seg = WallSegments[SegIndex];

		AStaticMeshActor* WallActor = Seg.WallActor.Get();
		if (!WallActor)
		{
			//Dead pointer, discard and retry
			WallSegments.RemoveAtSwap(SegIndex);
			--d;
			continue;
		}

		const FTransform WallTransform = WallActor->GetActorTransform();

		//Grid coordinates
		//On Inside
		const FIntPoint FloorCell = Seg.Cell;
		//On Boundary
		const FIntPoint WallCell = Seg.WallCell;

		//Clear this wall from an actor map and destroy it
		const int32 WallIdx = Index(WallCell.X, WallCell.Y);
		if (WallActorMap.IsValidIndex(WallIdx))
		{
			WallActorMap[WallIdx] = nullptr;
		}

		//Remove the wall section
		WallActor->Destroy();
		WallSegments.RemoveAtSwap(SegIndex);

		// -- Carve a straight corridor from the wall cell to the nearest edge --

		//Direction from interior floor cell to wall cell
		int32 DirX = Seg.WallCell.X - FloorCell.X;
		int32 DirY = Seg.WallCell.Y - FloorCell.Y;
		DirX = FMath::Clamp(DirX, -1, 1);
		DirY = FMath::Clamp(DirY, -1, 1);

		//Only carve a corridor if it's a valid cardinal direction
		const bool bHasDirection = !(DirX == 0 && DirY == 0);

		if (bHasDirection)
		{
			int32 CX = Seg.WallCell.X;
			int32 CY = Seg.WallCell.Y;

			//Spawn a floor mesh for the corridor
			while (CX >= 0 && CX < MapWidth && CY >= 0 && CY < MapHeight)
			{
				const int32 CellIdx = Index(CX, CY);

				//Carve this cell to floor in the logical map
				CurrentMap[CellIdx] = false;

				//Remove existing wall actor at this cell if any
				if (WallActorMap.IsValidIndex(CellIdx) && WallActorMap[CellIdx].IsValid())
				{
					WallActorMap[CellIdx]->Destroy();
					WallActorMap[CellIdx] = nullptr;
				}

				//Spawn a floor mesh along the corridor if there is one
				if (FloorMesh)
				{
					const float BasePlaneSize = 100.f;
					const float ScaleFactor = TileSize / BasePlaneSize;

					const FVector FloorPos = GetActorLocation() + FVector(CX * TileSize, CY * TileSize, FloorZ);

					AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(FloorPos, FRotator::ZeroRotator);

					if (FloorActor)
					{
						if (UStaticMeshComponent* FloorComp = FloorActor->GetStaticMeshComponent())
						{
							FloorComp->SetStaticMesh(FloorMesh);
							FloorActor->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, 1.f));
							FloorActor->SetMobility(EComponentMobility::Static);
						}
						else
						{
							FloorActor->Destroy();
						}
					}
				}

				// Stop carving if border is reached
				if (CX == 0 || CX == MapWidth - 1 || CY == 0 || CY == MapHeight - 1)
				{
					break;
				}

				CX += DirX;
				CY += DirY;
			}
		}

		//Spawn a door mesh
		if (DoorMesh)
		{
			AStaticMeshActor* DoorActor = World->SpawnActor<AStaticMeshActor>(WallTransform.GetLocation(), WallTransform.GetRotation().Rotator());

			if (DoorActor)
			{
				if (UStaticMeshComponent* DoorComp = DoorActor->GetStaticMeshComponent())
				{
					DoorComp->SetStaticMesh(DoorMesh);
					DoorActor->SetActorScale3D(WallTransform.GetScale3D());
					DoorActor->SetMobility(EComponentMobility::Static);

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