// Fill out your copyright notice in the Description page of Project Settings.


#include "BSP_FloorGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"

enum EWallDir : uint8
{
	Bottom = 0,
	Top = 1,
	Left = 2,
	Right = 3
};


// Sets default values
ABSP_FloorGenerator::ABSP_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABSP_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateBSP();

	SpawnFloorPlanes();

	//CreateDoors(DefaultDoorCount);
	
}

void ABSP_FloorGenerator::GenerateBSP()
{
	LeafRegions.Empty();

	FBSPLeaf Root(FIntPoint(0, 0), MapSize);
	SplitSpace(Root, 0);

}

void ABSP_FloorGenerator::SplitSpace(const FBSPLeaf& Region, int32 Depth)
{
	const int32 Width = Region.Width();
	const int32 Height = Region.Height();

	//Stop if too small or depth reached
	if (Depth >= MaxDepth || Width <= MinLeafSize * 2 && Height <= MinLeafSize * 2)
	{
		LeafRegions.Add(Region);
		return;
	}

	//Decide whether to split vertically or horizontally
	bool bSplitVertically;

	//If one dimension is much larger, favor splitting that axis
	if (Width > Height)
	{
		bSplitVertically = true;
	}
	else if (Height > Width)
	{
		bSplitVertically = false;
	}
	else
	{
		//50/50 when equal
		bSplitVertically = FMath::RandBool();
	}

	//If the chosen axis is too small to split, try another axis
	if (bSplitVertically && Width < MinLeafSize * 2)
	{
		bSplitVertically = false;
	}
	else if (!bSplitVertically && Height < MinLeafSize * 2)
	{
		bSplitVertically = true;
	}

	//If it still can't be split, this is a leaf
	if (bSplitVertically && Width < MinLeafSize * 2 ||
	!bSplitVertically && Height < MinLeafSize * 2)
	{
		LeafRegions.Add(Region);
		return;
	}

	if (bSplitVertically)
	{
		//Vertical split: X Axis
		const int32 SplitMin = Region.Min.X + MinLeafSize;
		const int32 SplitMax = Region.Max.X - MinLeafSize;

		if (SplitMin >= SplitMax)
		{
			LeafRegions.Add(Region);
			return;
		}

		const int32 SplitX = FMath::RandRange(SplitMin, SplitMax);

		FBSPLeaf Left(FIntPoint(Region.Min.X, Region.Min.Y), FIntPoint(SplitX, Region.Max.Y));
		FBSPLeaf Right(FIntPoint(SplitX, Region.Min.Y), FIntPoint(Region.Max.X, Region.Max.Y));

		SplitSpace(Left, Depth + 1);
		SplitSpace(Right, Depth + 1);
	}
	else
	{
		//Horizontal Split: Y Axis
		const int32 SplitMin = Region.Min.Y + MinLeafSize;
		const int32 SplitMax = Region.Max.Y - MinLeafSize;

		if (SplitMin >= SplitMax)
		{
			LeafRegions.Add(Region);
			return;
		}

		const int32 SplitY = FMath::RandRange(SplitMin, SplitMax);

		FBSPLeaf Bottom(FIntPoint(Region.Min.X, Region.Min.Y), FIntPoint(Region.Max.X, SplitY));
		FBSPLeaf Top(FIntPoint(Region.Min.X, SplitY), FIntPoint(Region.Max.X, Region.Max.Y));

		SplitSpace(Bottom, Depth + 1);
		SplitSpace(Top, Depth + 1);
	}
}

void ABSP_FloorGenerator::SpawnFloorPlanes()
{
	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: FloorMesh is null"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

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

	//Assume the plane and cube meshes are 100x100 units. Adjust if need be
	const float BaseMeshSize = 100.f;

	//Split a wall into two segments with a door-sized gap in the center
	auto CarveDoorInWall = [&](AStaticMeshActor* WallActor, bool bHorizontal, float WallLength)
	{
		if (!WallActor || !WallMesh) return;

		UWorld* LocalWorld = GetWorld();
		if (!LocalWorld) return;

		//Clamp door width so it  fits
		float DoorWorldWidth = FMath::Clamp(DoorWidth, 0.f, WallLength - 10.f);
		if (DoorWorldWidth <= 0.f) return;

		const float DoorHalf   = DoorWorldWidth * 0.5f;
		const float HalfLength = WallLength * 0.5f;

		//Lengths of the left/right (or bottom/top) wall pieces
		const float SideLength = (WallLength - DoorWorldWidth) * 0.5f;
		if (SideLength <= 0.f)
		{
			//Wall is too short to carve a proper door
			return;
		}

		const FVector WallCenter = WallActor->GetActorLocation();
		const FRotator WallRot   = WallActor->GetActorRotation();

		//Compute centers for the two wall segments
		FVector CenterA = WallCenter;
		FVector CenterB = WallCenter;

		if (bHorizontal)
		{
			//Wall runs along X (bottom/top)
			CenterA.X -= (DoorHalf + SideLength * 0.5f);
			CenterB.X += (DoorHalf + SideLength * 0.5f);
		}
		else
		{
			//Wall runs along Y (left/right)
			CenterA.Y -= (DoorHalf + SideLength * 0.5f);
			CenterB.Y += (DoorHalf + SideLength * 0.5f);
		}

		auto SpawnWallSegment = [&](const FVector& Center)
		{
			AStaticMeshActor* Segment = LocalWorld->SpawnActor<AStaticMeshActor>(Center, WallRot);
			if (!Segment) return;

			if (UStaticMeshComponent* WComp = Segment->GetStaticMeshComponent())
			{
				WComp->SetStaticMesh(WallMesh);

				const float SegmentX = bHorizontal ? SideLength : WallThickness;
				const float SegmentY = bHorizontal ? WallThickness : SideLength;
				const float ScaleX = SegmentX / BaseMeshSize;
				const float ScaleY = SegmentY / BaseMeshSize;
				const float ScaleZ = WallHeight / BaseMeshSize;

				Segment->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
				Segment->SetMobility(EComponentMobility::Static);
			}
			else
			{
				Segment->Destroy();
			}
		};

		//Destroy the original long wall
		WallActor->Destroy();

		//Spawn the two shorter segments
		SpawnWallSegment(CenterA);
		SpawnWallSegment(CenterB);

		//Spawn a door mesh in the middle of the gap (optional)
		if (DoorMesh)
		{
			AStaticMeshActor* DoorActor = LocalWorld->SpawnActor<AStaticMeshActor>(WallCenter, WallRot);
			if (DoorActor)
			{
				if (UStaticMeshComponent* DoorComp = DoorActor->GetStaticMeshComponent())
				{
					DoorComp->SetStaticMesh(DoorMesh);

					const float DoorX = bHorizontal ? DoorWorldWidth   : WallThickness;
					const float DoorY = bHorizontal ? WallThickness    : DoorWorldWidth;
					const float ScaleX = DoorX / BaseMeshSize;
					const float ScaleY = DoorY / BaseMeshSize;
					const float ScaleZ = WallHeight / BaseMeshSize;

					DoorActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
					DoorActor->SetMobility(EComponentMobility::Static);
				}
				else
				{
					DoorActor->Destroy();
				}
			}
		}
	};

	for (const FBSPLeaf& Leaf : LeafRegions)
	{
		AStaticMeshActor* BottomWallActor = nullptr;
		AStaticMeshActor* TopWallActor = nullptr;
		AStaticMeshActor* LeftWallActor = nullptr;
		AStaticMeshActor* RightWallActor = nullptr;

		const int32 LeafW = Leaf.Width();
		const int32 LeafH = Leaf.Height();

		if (LeafW <= 0 || LeafH <= 0)
		{
			continue;
		}

		//Random padding inside the leaf so rooms don't fill entire region
		const int32 PadMin = RoomPaddingMin;
		const int32 PadMax = RoomPaddingMax;

		int32 PadLeft = FMath::RandRange(PadMin, PadMax);
		int32 PadRight = FMath::RandRange(PadMin, PadMax);
		int32 PadBottom = FMath::RandRange(PadMin, PadMax);
		int32 PadTop = FMath::RandRange(PadMin, PadMax);

		//Clamp padding so room doesn't invert
		PadLeft = FMath::Clamp(PadLeft, 0, LeafW - 1);
		PadRight = FMath::Clamp(PadRight, 0, LeafW - 1 - PadLeft);
		PadBottom = FMath::Clamp(PadBottom, 0, LeafH - 1);
		PadTop = FMath::Clamp(PadTop, 0, LeafH - 1 - PadBottom);

		const int32 RoomMinX = Leaf.Min.X + PadLeft;
		const int32 RoomMaxX = Leaf.Max.X - PadRight;
		const int32 RoomMinY = Leaf.Min.Y + PadBottom;
		const int32 RoomMaxY = Leaf.Max.Y - PadTop;

		const int32 RoomW = RoomMaxX - RoomMinX;
		const int32 RoomH = RoomMaxY - RoomMinY;

		if (RoomW <= 0 || RoomH <= 0)
		{
			continue;
		}

		const float RoomWorldWidth = RoomW * TileSize;
		const float RoomWorldHeight = RoomH * TileSize;

		//Center of this room in world space
		const FVector RoomCenter(
			(RoomMinX + RoomW * 0.5f) * TileSize,
			(RoomMinY + RoomH * 0.5f) * TileSize,
			FloorZ
		);

		const FVector WorldLocation = GetActorLocation() + RoomCenter;

		//Scale so that a 100x100 plane becomes WorldWidth x WorldHeight
		const FVector FloorScale(
			RoomWorldWidth / BaseMeshSize,
			RoomWorldHeight / BaseMeshSize,
			1.f
		);

		//---- Floor ---- 

		AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(WorldLocation, FRotator::ZeroRotator);
		if (!FloorActor) continue;

		if (UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent())
		{
			MeshComp->SetStaticMesh(FloorMesh);
			FloorActor->SetActorScale3D(FloorScale);
			FloorActor->SetMobility(EComponentMobility::Static);
		}
		else
		{
			FloorActor->Destroy();
			continue;
		}

		//---- Walls ----

		if (!WallMesh) continue;

		//Bottom Wall
		{

			const FVector LocalPos(
				(RoomMinX + RoomW * 0.5f) * TileSize,
				RoomMinY * TileSize,
				FloorZ + WallHeight * 0.f
			);
			const FVector WorldPos = GetActorLocation() + LocalPos;

			AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WorldPos, FRotator::ZeroRotator);
				if (WallActor)
				{
					if (UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent())
					{
						WComp->SetStaticMesh(WallMesh);

						const float ScaleX = RoomWorldWidth / BaseMeshSize;
						const float ScaleY = WallThickness / BaseMeshSize;
						const float ScaleZ = WallHeight / BaseMeshSize;

						WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
						WallActor->SetMobility(EComponentMobility::Static);
						BottomWallActor = WallActor;

						FDungeonWallSegment Seg;
						Seg.Cell = FIntPoint(-1, -1);
						Seg.Direction = 0;
						Seg.WallActor = WallActor;
						WallSegments.Add(Seg);
					}
					else
					{
						WallActor->Destroy();
					}
				}
		}

		//Top Wall
		{
			const FVector LocalPos(
				(RoomMinX + RoomW * 0.5f) * TileSize,
				RoomMaxY * TileSize,
				FloorZ + WallHeight * 0.f
			);
			const FVector WorldPos = GetActorLocation() + LocalPos;

			AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WorldPos, FRotator::ZeroRotator);
			if (!WallActor) continue;

			if (UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent())
			{
				WComp->SetStaticMesh(WallMesh);

				const float ScaleX = RoomWorldWidth / BaseMeshSize;
				const float ScaleY = WallThickness / BaseMeshSize;
				const float ScaleZ = WallHeight / BaseMeshSize;

				WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
				WallActor->SetMobility(EComponentMobility::Static);
				BottomWallActor = WallActor;

				FDungeonWallSegment Seg;
				Seg.Cell = FIntPoint(-1, -1);
				Seg.Direction = 1;
				Seg.WallActor = WallActor;
				WallSegments.Add(Seg);
			}
			else
			{
				WallActor->Destroy();
			}

		}

		//Left Wall
		{
			const FVector LocalPos(
				RoomMinX  * TileSize,
				(RoomMinY + RoomH * 0.5f) * TileSize,
				FloorZ + WallHeight * 0.f
			);
			const FVector WorldPos = GetActorLocation() + LocalPos;

			//Yaw 90 so mesh's x-axis points along world y
			const FRotator Rot(0.f, 90.f, 0.f);

			AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WorldPos, Rot);

			if (!WallActor) continue;

			if (UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent())
			{
				WComp->SetStaticMesh(WallMesh);

				const float ScaleX = RoomWorldHeight / BaseMeshSize;
				const float ScaleY = WallThickness / BaseMeshSize;
				const float ScaleZ = WallHeight / BaseMeshSize;

				WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
				WallActor->SetMobility(EComponentMobility::Static);
				BottomWallActor = WallActor;

				FDungeonWallSegment Seg;
				Seg.Cell = FIntPoint(-1, -1);
				Seg.Direction = 2;
				Seg.WallActor = WallActor;
				WallSegments.Add(Seg);
			}
			else
			{
				WallActor->Destroy();
			}
		}

		//Right Wall
		{
			const FVector LocalPos(
				RoomMaxX * TileSize,
				(RoomMinY + RoomH * 0.5f) * TileSize,
				FloorZ + WallHeight * 0.f
			);
			const FVector WorldPos = GetActorLocation() + LocalPos;

			const FRotator Rot(0.f, 90.f, 0.f);

			AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WorldPos, Rot);
			if(!WallActor) continue;

			if (UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent())
			{
				WComp->SetStaticMesh(WallMesh);

				const float ScaleX = RoomWorldHeight / BaseMeshSize;
				const float ScaleY = WallThickness / BaseMeshSize;
				const float ScaleZ = WallHeight / BaseMeshSize;

				WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
				WallActor->SetMobility(EComponentMobility::Static);
				BottomWallActor = WallActor;

				FDungeonWallSegment Seg;
				Seg.Cell = FIntPoint(-1, -1);
				Seg.Direction = 3;
				Seg.WallActor = WallActor;
				WallSegments.Add(Seg);
			}
			else
			{
				WallActor->Destroy();
			}
		}

		// -- Choose a wall and carve a door --

		TArray<TPair<AStaticMeshActor*, bool>> CandidateWalls;
        if (BottomWallActor) CandidateWalls.Add({BottomWallActor, true});  // horizontal
        if (TopWallActor)    CandidateWalls.Add({TopWallActor,    true});
        if (LeftWallActor)   CandidateWalls.Add({LeftWallActor,   false}); // vertical
        if (RightWallActor)  CandidateWalls.Add({RightWallActor,  false});

        if (CandidateWalls.Num() > 0 && DoorWidth > 0.f)
        {
            const int32 ChoiceIdx = Rng.RandRange(0, CandidateWalls.Num() - 1);
            AStaticMeshActor* ChosenWall = CandidateWalls[ChoiceIdx].Key;
            const bool bHorizontal = CandidateWalls[ChoiceIdx].Value;

            const float WallLength = bHorizontal ? RoomWorldWidth : RoomWorldHeight;
            CarveDoorInWall(ChosenWall, bHorizontal, WallLength);
        }

	}
}

void ABSP_FloorGenerator::CreateDoors(int32 DoorCount)
{
	if (DoorCount <= 0) return;

	if (WallSegments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: No wall segments to carve doors from!"));
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

		//Remove the wall section
		WallActor->Destroy();
		WallSegments.RemoveAtSwap(Index);

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
				}
				else
				{
					DoorActor->Destroy();
				}
			}
		}
	}
}

// Called every frame
void ABSP_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

