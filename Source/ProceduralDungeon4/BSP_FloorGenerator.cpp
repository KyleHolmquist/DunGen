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


ABSP_FloorGenerator::ABSP_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABSP_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABSP_FloorGenerator::GenerateModule()
{
	GeneratedEmptyLocations.Reset();
	GeneratedWallActors.Reset();
	GenerateBSP();
	SpawnFloorPlanes();

	const int32 DoorCount = (DesiredExteriorDoors > 0) ? DesiredExteriorDoors : DefaultDoorCount;

	CreateDoors(DoorCount);

	bHasFinishedGenerating = true;
}

void ABSP_FloorGenerator::GenerateBSP()
{
	LeafRegions.Empty();

	FBSPLeaf BSPRoot(FIntPoint(0, 0), MapSize);
	SplitSpace(BSPRoot, 0);

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
		//Horizontal Split on the Y Axis
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
    if (!FloorTileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator - FloorTileClass is null"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    //RNG for room padding
    FRandomStream Rng;
    if (Seed >= 0) Rng.Initialize(Seed);
    else Rng.GenerateNewSeed();

	//Both plane and cube are 100x100
    const float BaseMeshSize = 100.f; 

    WallSegments.Empty();
    CellToRoomIndex.Empty();

    //Iterate by index so RoomIndex can be used in CellToRoomIndex
    for (int32 RoomIndex = 0; RoomIndex < LeafRegions.Num(); ++RoomIndex)
    {
        const FBSPLeaf& Leaf = LeafRegions[RoomIndex];

        const int32 LeafW = Leaf.Width();
        const int32 LeafH = Leaf.Height();
        if (LeafW <= 0 || LeafH <= 0) continue;

        // ---- Room padding inside leaf ----
        const int32 PadMin = RoomPaddingMin;
        const int32 PadMax = RoomPaddingMax;

        int32 PadLeft   = FMath::RandRange(PadMin, PadMax);
        int32 PadRight  = FMath::RandRange(PadMin, PadMax);
        int32 PadBottom = FMath::RandRange(PadMin, PadMax);
        int32 PadTop    = FMath::RandRange(PadMin, PadMax);

        PadLeft   = FMath::Clamp(PadLeft,   0, LeafW - 1);
        PadRight  = FMath::Clamp(PadRight,  0, LeafW - 1 - PadLeft);
        PadBottom = FMath::Clamp(PadBottom, 0, LeafH - 1);
        PadTop    = FMath::Clamp(PadTop,    0, LeafH - 1 - PadBottom);

        const int32 RoomMinX = Leaf.Min.X + PadLeft;
        const int32 RoomMaxX = Leaf.Max.X - PadRight;
        const int32 RoomMinY = Leaf.Min.Y + PadBottom;
        const int32 RoomMaxY = Leaf.Max.Y - PadTop;

        const int32 RoomW = RoomMaxX - RoomMinX;
        const int32 RoomH = RoomMaxY - RoomMinY;
        if (RoomW <= 0 || RoomH <= 0) continue;

        //Register the interior cells for this room
        for (int32 Y = RoomMinY; Y < RoomMaxY; ++Y)
        {
            for (int32 X = RoomMinX; X < RoomMaxX; ++X)
            {
                CellToRoomIndex.Add(FIntPoint(X, Y), RoomIndex);
            }
        }

        const float RoomWorldWidth  = RoomW * TileSize;
        const float RoomWorldHeight = RoomH * TileSize;

        //Center in world space
        const FVector RoomCenter((RoomMinX + RoomW * 0.5f) * TileSize, (RoomMinY + RoomH * 0.5f) * TileSize, FloorZ);

        const FVector FloorLocation = GetActorLocation() + RoomCenter;
        const FVector FloorScale( RoomWorldWidth  / BaseMeshSize, RoomWorldHeight / BaseMeshSize, 1.f);

        //-- Floor --

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FTransform SpawnTransform(FRotator::ZeroRotator, FloorLocation);

		if (!FloorTileClass) continue;

		AFloorTile* FloorActor = World->SpawnActor<AFloorTile>(FloorTileClass, SpawnTransform, Params);

        if (!FloorActor) continue;

		GeneratedFloorActors.Add(FloorActor);

        if (UStaticMeshComponent* MeshComp = FloorActor->GetItemMesh())
        {
            //MeshComp->SetStaticMesh(FloorMesh);
            FloorActor->SetActorScale3D(FloorScale);
		
			MeshComp->SetMobility(EComponentMobility::Movable);
			FloorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        }
        else
        {
			UE_LOG(LogTemp, Warning, TEXT("No Mesh Component!"));
            FloorActor->Destroy();
            continue;
        }

		GeneratedEmptyLocations.Add(FloorLocation);

        //-- Walls as equal-length segments --
        if (!WallTileClass || WallHeight <= 0.f) continue;

        const float HalfThickness = WallThickness * 0.5f;
        const FVector Origin = GetActorLocation();
        const float WallZ = FloorZ + WallHeight * 0.f;

        auto SpawnWallSegment = [&](const FVector& Location, const FRotator& Rot) -> AWallTile*
        {

			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			const FTransform SpawnTransform(Rot, Location);

			if (!WallTileClass) return nullptr;

			AWallTile* Segment = World->SpawnActor<AWallTile>(WallTileClass, SpawnTransform, Params);

			GeneratedWallActors.Add(Segment);

            if (!Segment) return nullptr;

			//GeneratedWallActors.Add(Segment);

            UStaticMeshComponent* WComp = Segment->GetItemMesh();
            if (!WComp)
            {
                Segment->Destroy();
                return nullptr;
            }

            WComp->SetMobility(EComponentMobility::Movable);
			Segment->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

            //WComp->SetStaticMesh(WallMesh);

            const float SegmentLength = TileSize;
			//Length
            const float ScaleX = SegmentLength / BaseMeshSize;
			//Thickness   
            const float ScaleY = WallThickness / BaseMeshSize;
            const float ScaleZ = WallHeight   / BaseMeshSize;

            Segment->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
            return Segment;
        };

        //Bottom & Top walls, which run along X
        for (int32 i = 0; i < RoomW; ++i)
        {
            const int32 TileX = RoomMinX + i;
            const float CenterX = (TileX + 0.5f) * TileSize;

            const float BottomY = RoomMinY * TileSize - HalfThickness;
            const float TopY    = RoomMaxY * TileSize + HalfThickness;

            //Bottom segment, which belongs to the interior cell just above the bottom edge
            {
                const FVector Loc = Origin + FVector(CenterX, BottomY, WallZ);
                const FRotator Rot(0.f, 0.f, 0.f);

                if (AWallTile* Segment = SpawnWallSegment(Loc, Rot))
                {
					Segment->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
                    
					FDungeonWallSegment Seg;
                    Seg.Cell      = FIntPoint(TileX, RoomMinY);
                    Seg.Direction = (uint8)EWallDir::Bottom;
                    Seg.WallActor = Segment;
                    WallSegments.Add(Seg);
                }
            }

            //Top segment, which belongs to the interior cell just below the top edge
            {
                const FVector Loc = Origin + FVector(CenterX, TopY, WallZ);
                const FRotator Rot(0.f, 0.f, 0.f);

                if (AWallTile* Segment = SpawnWallSegment(Loc, Rot))
                {
                    
					FDungeonWallSegment Seg;
                    Seg.Cell      = FIntPoint(TileX, RoomMaxY - 1);
                    Seg.Direction = (uint8)EWallDir::Top;
                    Seg.WallActor = Segment;
                    WallSegments.Add(Seg);
                }
            }
        }

        //Left & Right walls, which run along Y
        for (int32 j = 0; j < RoomH; ++j)
        {
            const int32 TileY = RoomMinY + j;
            const float CenterY = (TileY + 0.5f) * TileSize;

            const float LeftX  = RoomMinX * TileSize - HalfThickness;
            const float RightX = RoomMaxX * TileSize + HalfThickness;

            //Left segment, which belongs to the interior cell just right of left edge
            {
                const FVector Loc = Origin + FVector(LeftX, CenterY, WallZ);
                const FRotator Rot(0.f, 90.f, 0.f);

                if (AWallTile* Segment = SpawnWallSegment(Loc, Rot))
                {
					Segment->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

                    FDungeonWallSegment Seg;
                    Seg.Cell      = FIntPoint(RoomMinX, TileY);
                    Seg.Direction = (uint8)EWallDir::Left;
                    Seg.WallActor = Segment;
                    WallSegments.Add(Seg);
                }
            }

            //Right segment, which belongs to the interior cell just left of right edge
            {
                const FVector Loc = Origin + FVector(RightX, CenterY, WallZ);
                const FRotator Rot(0.f, 90.f, 0.f);

                if (AWallTile* Segment = SpawnWallSegment(Loc, Rot))
                {
					Segment->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

                    FDungeonWallSegment Seg;
                    Seg.Cell      = FIntPoint(RoomMaxX - 1, TileY);
                    Seg.Direction = (uint8)EWallDir::Right;
                    Seg.WallActor = Segment;
                    WallSegments.Add(Seg);
                }
            }
        }
    }
}


// void ABSP_FloorGenerator::CreateDoors(int32 ExteriorDoorCount)
// {
// 	if (WallSegments.Num() == 0) return;

// 	UWorld* World = GetWorld();
// 	if (!World) return;

// 	const int32 RoomCount = LeafRegions.Num();
// 	if (RoomCount == 0) return;

// 	//How many wall segments wide a door should be
// 	const int32 DoorSegments = FMath::Max(1, FMath::RoundToInt(DoorWidth / TileSize));

// 	//Deterministic RNG for the doors
// 	FRandomStream Rng;
// 	if (Seed >=0) Rng.Initialize(Seed + 777);
// 	else Rng.GenerateNewSeed();

// 	// Helper: neighbour cell in grid space
//     auto GetNeighborCell = [](const FDungeonWallSegment& Seg) -> FIntPoint
//     {
//         FIntPoint N = Seg.Cell;
//         switch ((EWallDir)Seg.Direction)
//         {
//         case EWallDir::Bottom: N.Y -= 1; break;
//         case EWallDir::Top:    N.Y += 1; break;
//         case EWallDir::Left:   N.X -= 1; break;
//         case EWallDir::Right:  N.X += 1; break;
//         }
//         return N;
//     };

// 	//Check to see if  this wall segment touches any perpendicular wall anywhere along its length
// 	auto SegmentTouchesPerpendicular = [&](int32 Index) -> bool
// 	{
// 		if (!WallSegments.IsValidIndex(Index)) return false;
// 		const FDungeonWallSegment& Seg = WallSegments[Index];
// 		if (!Seg.WallActor.IsValid()) return false;

// 		AWallTile* ThisActor = Seg.WallActor.Get();
// 		const FVector ThisLoc = ThisActor->GetActorLocation();

// 		const bool bThisHorizontal =
// 			(Seg.Direction == EWallDir::Bottom || Seg.Direction == EWallDir::Top);

// 		const float HalfLen = TileSize * 0.5f;

// 		for (int32 j = 0; j < WallSegments.Num(); ++j)
// 		{
// 			if (j == Index) continue;

// 			const FDungeonWallSegment& OtherSeg = WallSegments[j];
// 			if (!OtherSeg.WallActor.IsValid()) continue;

// 			const bool bOtherHorizontal =
// 				(OtherSeg.Direction == EWallDir::Bottom || OtherSeg.Direction == EWallDir::Top);

// 			//Only care about perpendicular walls
// 			if (bThisHorizontal == bOtherHorizontal)
// 				continue;

// 			AWallTile* OtherActor = OtherSeg.WallActor.Get();
// 			const FVector OtherLoc = OtherActor->GetActorLocation();

// 			if (bThisHorizontal)
// 			{
// 				//Horizontal extends along X, other is vertical along Y
// 				const bool bXOverlap =
// 					OtherLoc.X >= ThisLoc.X - HalfLen && OtherLoc.X <= ThisLoc.X + HalfLen;
// 				const bool bYOverlap =
// 					ThisLoc.Y >= OtherLoc.Y - HalfLen && ThisLoc.Y <= OtherLoc.Y + HalfLen;

// 				if (bXOverlap && bYOverlap)
// 					return true;
// 			}
// 			else
// 			{
// 				//This is vertical, other is horizontal
// 				const bool bXOverlap =
// 					ThisLoc.X >= OtherLoc.X - HalfLen && ThisLoc.X <= OtherLoc.X + HalfLen;
// 				const bool bYOverlap =
// 					OtherLoc.Y >= ThisLoc.Y - HalfLen && OtherLoc.Y <= ThisLoc.Y + HalfLen;

// 				if (bXOverlap && bYOverlap)
// 					return true;
// 			}
// 		}

// 		return false;
// 	};

// 	//Helper that returns indices for a contiguous run of segments along the same wall.
// 	//BaseIndex is the start of the segment. Walk forward along the wall direction and
// 	//Try to collect DoorSegments total. Returns false if unable to.
// 	auto CollectAlongWallSegmentIndices = [&](int32 BaseIndex, TArray<int32>& OutIndices) -> bool
// 	{
// 		OutIndices.Reset();

// 		if (!WallSegments.IsValidIndex(BaseIndex))
// 			return false;

// 		FDungeonWallSegment& BaseSeg = WallSegments[BaseIndex];
// 		if (!BaseSeg.WallActor.IsValid())
// 			return false;

// 		OutIndices.Add(BaseIndex);

// 		//Tangent direction along the wall
// 		FIntPoint Tangent(0, 0);
// 		switch ((EWallDir)BaseSeg.Direction)
// 		{
// 		case EWallDir::Bottom:
// 		//Increase X
// 		case EWallDir::Top:   Tangent = FIntPoint(1, 0); break;
// 		case EWallDir::Left:
// 		//Increase Y
// 		case EWallDir::Right: Tangent = FIntPoint(0, 1); break;
// 		}

// 		FIntPoint CurrentCell = BaseSeg.Cell;

// 		for (int32 s = 1; s < DoorSegments; ++s)
// 		{
// 			CurrentCell += Tangent;
// 			bool bFound = false;

// 			for (int32 i = 0; i < WallSegments.Num(); ++i)
// 			{
// 				if (i == BaseIndex) continue;

// 				FDungeonWallSegment& Other = WallSegments[i];
// 				if (!Other.WallActor.IsValid()) continue;

// 				if (Other.Cell == CurrentCell && Other.Direction == BaseSeg.Direction)
// 				{
// 					OutIndices.Add(i);
// 					bFound = true;
// 					break;
// 				}
// 			}

// 			if (!bFound)
// 			{
// 				//Not enough contiguous segments to make a wide door
// 				return false;
// 			}

// 			//Reject door locations where any segment in the run touches a perpendicular wall
// 			for (int32 idx : OutIndices)
// 			{
// 				if (SegmentTouchesPerpendicular(idx))
// 				return false;
// 			}
// 		}

// 		return true;
// 	};

//     //Helper to destroy this segment and its opposite twin, additional door-width segments along the wall,
// 	//And eacj of their opposite twins, if they exist.
//    auto DestroySegmentAndOpposite = [&](int32 SegIndex)
// 	{
// 		TArray<int32> DoorSegIndices;
// 		if (!CollectAlongWallSegmentIndices(SegIndex, DoorSegIndices))
// 			return; 

// 		for (int32 Index : DoorSegIndices)
// 		{
// 			if (!WallSegments.IsValidIndex(Index)) continue;

// 			FDungeonWallSegment& Seg = WallSegments[Index];
// 			AWallTile* Actor = Seg.WallActor.Get();
// 			if (Actor) Actor->Destroy();
// 			Seg.WallActor = nullptr;

// 			//Destroy the opposite twin segment on the other side of the wall
// 			FIntPoint NeighborCell = GetNeighborCell(Seg);
// 			uint8 OppDir = 0;
// 			switch ((EWallDir)Seg.Direction)
// 			{
// 			case EWallDir::Bottom: OppDir = (uint8)EWallDir::Top;    break;
// 			case EWallDir::Top:    OppDir = (uint8)EWallDir::Bottom; break;
// 			case EWallDir::Left:   OppDir = (uint8)EWallDir::Right;  break;
// 			case EWallDir::Right:  OppDir = (uint8)EWallDir::Left;   break;
// 			}

// 			for (int32 j = 0; j < WallSegments.Num(); ++j)
// 			{
// 				if (j == Index) continue;
// 				FDungeonWallSegment& Other = WallSegments[j];
// 				if (!Other.WallActor.IsValid()) continue;

// 				if (Other.Cell == NeighborCell && Other.Direction == OppDir)
// 				{
// 					if (AWallTile* OtherActor = Other.WallActor.Get())
// 					{
// 						OtherActor->Destroy();
// 					}
// 					Other.WallActor = nullptr;
// 					break;
// 				}
// 			}
// 		}
// 	};

//     // -- Classify interior vs exterior segments --
//     TArray<TArray<int32>> InteriorWallsPerRoom;
//     InteriorWallsPerRoom.SetNum(RoomCount);

//     TArray<int32> ExteriorWalls;

//     for (int32 i = 0; i < WallSegments.Num(); ++i)
//     {
//         FDungeonWallSegment& Seg = WallSegments[i];
//         if (!Seg.WallActor.IsValid()) continue;

//         int32* RoomA = CellToRoomIndex.Find(Seg.Cell);
//         if (!RoomA) continue;

//         FIntPoint NeighborCell = GetNeighborCell(Seg);
//         int32* RoomB = CellToRoomIndex.Find(NeighborCell);

//         TArray<int32> DummyDoorSegs;

// 		if (RoomB && *RoomB != *RoomA)
// 		{
// 			//Only consider this as a candidate if it can fit a full-width door
// 			if (CollectAlongWallSegmentIndices(i, DummyDoorSegs))
// 			{
// 				InteriorWallsPerRoom[*RoomA].Add(i);
// 				InteriorWallsPerRoom[*RoomB].Add(i);
// 			}
// 		}
// 		else if (!RoomB)
// 		{
// 			// Exterior wall, only consider a candidate if it can fit a full-width door
// 			if (CollectAlongWallSegmentIndices(i, DummyDoorSegs))
// 			{
// 				ExteriorWalls.Add(i);
// 			}
// 		}

//     }

//     // -- Interior doors. Ensure each room has doors --
//     for (int32 RoomIndex = 0; RoomIndex < RoomCount; ++RoomIndex)
//     {
//         auto& Candidates = InteriorWallsPerRoom[RoomIndex];
//         if (Candidates.Num() == 0) continue;

// 		const int32 DoorsThisRoom = FMath::RandRange(DoorsPerRoomMin, DoorsPerRoomMax);
//         const int32 NumDoorsForRoom = FMath::Min(DoorsThisRoom, Candidates.Num());

//         for (int32 d = 0; d < NumDoorsForRoom; ++d)
//         {
//             const int32 PickIdx   = Rng.RandRange(0, Candidates.Num() - 1);
//             const int32 SegIndex  = Candidates[PickIdx];
//             Candidates.RemoveAtSwap(PickIdx);

//             if (!WallSegments.IsValidIndex(SegIndex)) continue;
//             FDungeonWallSegment& Seg = WallSegments[SegIndex];
//             if (!Seg.WallActor.IsValid()) continue;

//             const FTransform T = Seg.WallActor->GetActorTransform();
//             DestroySegmentAndOpposite(SegIndex);

//             //Door mesh
//             if (DoorMesh)
//             {

// 				FActorSpawnParameters Params;
// 				Params.Owner = this;
// 				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

// 				if (!WallTileClass) continue;

// 				AWallTile* DoorActor = World->SpawnActor<AWallTile>(WallTileClass, T.GetLocation(), T.GetRotation().Rotator(), Params);

//                 if (DoorActor)
//                 {
// 					DoorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

//                     if (UStaticMeshComponent* DoorComp = DoorActor->GetItemMesh())
//                     {
// 						DoorComp->SetMobility(EComponentMobility::Movable);

// 						FVector DoorScale = T.GetScale3D();
// 						//Stretch the door mesh to cover all segments
// 						DoorScale.X *= DoorSegments;
//                         DoorActor->SetActorScale3D(DoorScale);

// 						GeneratedDoorActors.Add(DoorActor);

//                     }
//                     else
//                     {
//                         DoorActor->Destroy();
//                     }
//                 }
//             }
//         }

// 			auto GetPortalFacingRotationFromSegment = [&](const FDungeonWallSegment& Seg) -> FRotator
// 			{
// 				switch ((EWallDir)Seg.Direction)
// 				{
// 				case EWallDir::Bottom:
// 					// bottom wall: portal faces into room (+Y)
// 					return FRotator(0.f, 90.f, 0.f);

// 				case EWallDir::Top:
// 					// top wall: portal faces into room (-Y)
// 					return FRotator(0.f, -90.f, 0.f);

// 				case EWallDir::Left:
// 					// left wall: portal faces into room (+X)
// 					return FRotator(0.f, 0.f, 0.f);

// 				case EWallDir::Right:
// 					// right wall: portal faces into room (-X)
// 					return FRotator(0.f, 180.f, 0.f);

// 				default:
// 					return FRotator::ZeroRotator;
// 				}
// 			};

// 			auto ConfigurePortalComponentsForWall = [&](AWallTile* WallActor, const FDungeonWallSegment& Seg) -> bool
// 			{
// 				if (!WallActor) return false;

// 				USceneComponent* PortalFacingRoot = WallActor->GetPortalFacingRoot();
// 				USceneComponent* PortalSpawnPoint = WallActor->GetPortalSpawnPoint();

// 				if (!PortalFacingRoot || !PortalSpawnPoint)
// 				{
// 					UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: PortalFacingRoot or PortalSpawnPoint missing on wall tile."));
// 					return false;
// 				}

// 				const FRotator PortalFacingRotation = GetPortalFacingRotationFromSegment(Seg);

// 				// Rotate only the helper parent, never the wall actor itself
// 				PortalFacingRoot->SetRelativeRotation(PortalFacingRotation);

// 				return true;
// 			};


// 			// -- Choose one portal location from exterior wall candidates --
// 			TArray<int32> PortalCandidates;

// 			for (int32 SegIndex : ExteriorWalls)
// 			{
// 				if (!WallSegments.IsValidIndex(SegIndex)) continue;

// 				FDungeonWallSegment& Seg = WallSegments[SegIndex];
// 				if (!Seg.WallActor.IsValid()) continue;

// 				PortalCandidates.Add(SegIndex);
// 			}

// 			if (PortalCandidates.Num() == 0)
// 			{
// 				UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: No valid exterior wall candidates found for portal."));
// 				return;
// 			}

// 			const int32 PortalPickIdx = Rng.RandRange(0, PortalCandidates.Num() - 1);
// 			const int32 ChosenSegIndex = PortalCandidates[PortalPickIdx];

// 			if (!WallSegments.IsValidIndex(ChosenSegIndex)) return;

// 			FDungeonWallSegment& ChosenSeg = WallSegments[ChosenSegIndex];
// 			if (!ChosenSeg.WallActor.IsValid()) return;

// 			AWallTile* ChosenWallActor = ChosenSeg.WallActor.Get();
// 			if (!ChosenWallActor) return;

// 			GeneratedWallActors.Add(ChosenWallActor);

// 			const FVector PortalLocation = ChosenWallActor->GetActorLocation();
// 			const FRotator PortalRotation = GetPortalFacingRotationFromSegment(ChosenSeg);

// 			// Record the portal location for DungeonManager
// 			AddExteriorDoorWorld(PortalLocation, PortalRotation);

// 			UE_LOG(LogTemp, Warning, TEXT("BSP portal chosen at %s facing %s"),
// 				*PortalLocation.ToString(),
// 				*PortalRotation.ToString());


	   
// 	}

//     // -- Exterior doors --
//     //ExteriorDoorCount = FMath::Min(ExteriorDoorCount, ExteriorWalls.Num());
// 	// ExteriorDoorCount = 0;

//     // for (int32 d = 0; d < ExteriorDoorCount && ExteriorWalls.Num() > 0; ++d)
//     // {
//     //     const int32 PickIdx  = Rng.RandRange(0, ExteriorWalls.Num() - 1);
//     //     const int32 SegIndex = ExteriorWalls[PickIdx];
//     //     ExteriorWalls.RemoveAtSwap(PickIdx);

//     //     if (!WallSegments.IsValidIndex(SegIndex)) continue;
//     //     FDungeonWallSegment& Seg = WallSegments[SegIndex];
//     //     if (!Seg.WallActor.IsValid()) continue;

//     //     const FTransform WallTransform = Seg.WallActor->GetActorTransform();
//     //     DestroySegmentAndOpposite(SegIndex);

// 	// 	//Record the door so DungeonManager can use it
// 	// 	// FExteriorDoor DoorInfo;
// 	// 	// DoorInfo.Location = T.GetLocation();
// 	// 	// DoorInfo.Rotation = T.GetRotation().Rotator();
// 	// 	// ExteriorDoors.Add(DoorInfo);
// 	// 	//AddExteriorDoorWorld(WallTransform.GetLocation(), WallTransform.GetRotation().Rotator());

//     //     // if (DoorMesh)
//     //     // {

// 	// 	// 	FActorSpawnParameters Params;
// 	// 	// 	Params.Owner = this;
// 	// 	// 	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

// 	// 	// 	if (!WallTileClass) continue;

// 	// 	// 	AWallTile* DoorActor = World->SpawnActor<AWallTile>(WallTileClass, WallTransform.GetLocation(), WallTransform.GetRotation().Rotator(), Params);
			
//     //     //     if (DoorActor)
//     //     //     {
// 	// 	// 		DoorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

//     //     //         if (UStaticMeshComponent* DoorComp = DoorActor->GetItemMesh())
//     //     //         {
// 	// 	// 			DoorComp->SetMobility(EComponentMobility::Movable);

// 	// 	// 			FVector DoorScale = WallTransform.GetScale3D();
// 	// 	// 			//Stretch the door mesh to cover all segments
// 	// 	// 			DoorScale.X *= DoorSegments;

// 	// 	// 			DoorActor->SetActorScale3D(DoorScale);

// 	// 	// 			GeneratedDoorActors.Add(DoorActor);
                    
//     //     //         }
//     //     //         else
//     //     //         {
//     //     //             DoorActor->Destroy();
//     //     //         }
//     //     //     }

// 	// 	// 	UE_LOG(LogTemp, Warning, TEXT(" created %d exterior doors"), ExteriorDoors.Num());
//     //     // }
//     // }
// }

void ABSP_FloorGenerator::CreateDoors(int32 ExteriorDoorCount)
{
	if (WallSegments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: No wall segments to process in CreateDoors."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: World is null in CreateDoors."));
		return;
	}

	const int32 RoomCount = LeafRegions.Num();
	if (RoomCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: No rooms found in CreateDoors."));
		return;
	}

	const int32 DoorSegments = FMath::Max(1, FMath::RoundToInt(DoorWidth / TileSize));

	FRandomStream Rng;
	if (Seed >= 0)
	{
		Rng.Initialize(Seed + 777);
	}
	else
	{
		Rng.GenerateNewSeed();
	}

	auto GetNeighborCell = [](const FDungeonWallSegment& Seg) -> FIntPoint
	{
		FIntPoint N = Seg.Cell;

		switch ((EWallDir)Seg.Direction)
		{
		case EWallDir::Bottom: N.Y -= 1; break;
		case EWallDir::Top:    N.Y += 1; break;
		case EWallDir::Left:   N.X -= 1; break;
		case EWallDir::Right:  N.X += 1; break;
		default: break;
		}

		return N;
	};

	auto IsHorizontal = [](const FDungeonWallSegment& Seg) -> bool
	{
		return Seg.Direction == EWallDir::Bottom || Seg.Direction == EWallDir::Top;
	};

	auto GetOppositeDirection = [](uint8 Direction) -> uint8
	{
		switch ((EWallDir)Direction)
		{
		case EWallDir::Bottom: return (uint8)EWallDir::Top;
		case EWallDir::Top:    return (uint8)EWallDir::Bottom;
		case EWallDir::Left:   return (uint8)EWallDir::Right;
		case EWallDir::Right:  return (uint8)EWallDir::Left;
		default:               return Direction;
		}
	};

	auto GetPortalFacingRotationFromSegment = [](const FDungeonWallSegment& Seg) -> FRotator
	{
		switch ((EWallDir)Seg.Direction)
		{
		case EWallDir::Bottom: return FRotator(0.f,  90.f, 0.f);
		case EWallDir::Top:    return FRotator(0.f, -90.f, 0.f);
		case EWallDir::Left:   return FRotator(0.f,   0.f, 0.f);
		case EWallDir::Right:  return FRotator(0.f, 180.f, 0.f);
		default:               return FRotator::ZeroRotator;
		}
	};

	auto SegmentTouchesPerpendicular = [&](int32 Index) -> bool
	{
		if (!WallSegments.IsValidIndex(Index))
		{
			return false;
		}

		const FDungeonWallSegment& Seg = WallSegments[Index];
		if (!Seg.WallActor.IsValid())
		{
			return false;
		}

		const bool bThisHorizontal = IsHorizontal(Seg);
		const FVector ThisLoc = Seg.WallActor->GetActorLocation();
		const float HalfLen = TileSize * 0.5f;

		for (int32 j = 0; j < WallSegments.Num(); ++j)
		{
			if (j == Index)
			{
				continue;
			}

			const FDungeonWallSegment& Other = WallSegments[j];
			if (!Other.WallActor.IsValid())
			{
				continue;
			}

			const bool bOtherHorizontal = IsHorizontal(Other);
			if (bThisHorizontal == bOtherHorizontal)
			{
				continue;
			}

			const FVector OtherLoc = Other.WallActor->GetActorLocation();

			if (bThisHorizontal)
			{
				const bool bXOverlap = OtherLoc.X >= ThisLoc.X - HalfLen && OtherLoc.X <= ThisLoc.X + HalfLen;
				const bool bYOverlap = ThisLoc.Y >= OtherLoc.Y - HalfLen && ThisLoc.Y <= OtherLoc.Y + HalfLen;

				if (bXOverlap && bYOverlap)
				{
					return true;
				}
			}
			else
			{
				const bool bXOverlap = ThisLoc.X >= OtherLoc.X - HalfLen && ThisLoc.X <= OtherLoc.X + HalfLen;
				const bool bYOverlap = OtherLoc.Y >= ThisLoc.Y - HalfLen && OtherLoc.Y <= ThisLoc.Y + HalfLen;

				if (bXOverlap && bYOverlap)
				{
					return true;
				}
			}
		}

		return false;
	};

	auto CollectAlongWallSegmentIndices = [&](int32 BaseIndex, TArray<int32>& OutIndices) -> bool
	{
		OutIndices.Reset();

		if (!WallSegments.IsValidIndex(BaseIndex))
		{
			return false;
		}

		const FDungeonWallSegment& BaseSeg = WallSegments[BaseIndex];
		if (!BaseSeg.WallActor.IsValid())
		{
			return false;
		}

		OutIndices.Add(BaseIndex);

		FIntPoint Tangent(0, 0);
		switch ((EWallDir)BaseSeg.Direction)
		{
		case EWallDir::Bottom:
		case EWallDir::Top:
			Tangent = FIntPoint(1, 0);
			break;

		case EWallDir::Left:
		case EWallDir::Right:
			Tangent = FIntPoint(0, 1);
			break;

		default:
			return false;
		}

		FIntPoint CurrentCell = BaseSeg.Cell;

		for (int32 s = 1; s < DoorSegments; ++s)
		{
			CurrentCell += Tangent;

			bool bFound = false;

			for (int32 i = 0; i < WallSegments.Num(); ++i)
			{
				if (i == BaseIndex)
				{
					continue;
				}

				const FDungeonWallSegment& Other = WallSegments[i];
				if (!Other.WallActor.IsValid())
				{
					continue;
				}

				if (Other.Cell == CurrentCell && Other.Direction == BaseSeg.Direction)
				{
					OutIndices.Add(i);
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				return false;
			}
		}

		for (int32 Index : OutIndices)
		{
			if (SegmentTouchesPerpendicular(Index))
			{
				return false;
			}
		}

		return true;
	};

	auto DestroySegmentAndOppositeRun = [&](int32 SegIndex)
	{
		TArray<int32> DoorRun;
		if (!CollectAlongWallSegmentIndices(SegIndex, DoorRun))
		{
			return;
		}

		for (int32 RunIndex : DoorRun)
		{
			if (!WallSegments.IsValidIndex(RunIndex))
			{
				continue;
			}

			FDungeonWallSegment& Seg = WallSegments[RunIndex];

			if (AWallTile* Actor = Seg.WallActor.Get())
			{
				Actor->Destroy();
			}
			Seg.WallActor = nullptr;

			const FIntPoint NeighborCell = GetNeighborCell(Seg);
			const uint8 OppositeDir = GetOppositeDirection(Seg.Direction);

			for (int32 j = 0; j < WallSegments.Num(); ++j)
			{
				if (j == RunIndex)
				{
					continue;
				}

				FDungeonWallSegment& Other = WallSegments[j];
				if (!Other.WallActor.IsValid())
				{
					continue;
				}

				if (Other.Cell == NeighborCell && Other.Direction == OppositeDir)
				{
					if (AWallTile* OtherActor = Other.WallActor.Get())
					{
						OtherActor->Destroy();
					}
					Other.WallActor = nullptr;
					break;
				}
			}
		}
	};

	auto SpawnDoorActorFromTransform = [&](const FTransform& T)
	{
		if (!WallTileClass)
		{
			return;
		}

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWallTile* DoorActor = World->SpawnActor<AWallTile>(
			WallTileClass,
			T.GetLocation(),
			T.GetRotation().Rotator(),
			Params);

		if (!DoorActor)
		{
			return;
		}

		GeneratedDoorActors.Add(DoorActor);

		DoorActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

		UStaticMeshComponent* DoorComp = DoorActor->GetItemMesh();
		if (!DoorComp)
		{
			DoorActor->Destroy();
			return;
		}

		DoorComp->SetMobility(EComponentMobility::Movable);

		FVector DoorScale = T.GetScale3D();
		DoorScale.X *= DoorSegments;
		DoorActor->SetActorScale3D(DoorScale);
	};

	auto ConfigurePortalForWall = [&](AWallTile* WallActor, const FDungeonWallSegment& Seg) -> bool
	{
		if (!WallActor)
		{
			return false;
		}

		USceneComponent* PortalFacingRoot = WallActor->GetPortalFacingRoot();
		USceneComponent* PortalSpawnPoint = WallActor->GetPortalSpawnPoint();

		if (!PortalFacingRoot || !PortalSpawnPoint)
		{
			UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: Chosen wall is missing portal helper components."));
			return false;
		}

		PortalFacingRoot->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		return true;
	};

	TArray<TArray<int32>> InteriorWallsPerRoom;
	InteriorWallsPerRoom.SetNum(RoomCount);

	TArray<int32> ExteriorWalls;

	for (int32 i = 0; i < WallSegments.Num(); ++i)
	{
		FDungeonWallSegment& Seg = WallSegments[i];
		if (!Seg.WallActor.IsValid())
		{
			continue;
		}

		int32* RoomA = CellToRoomIndex.Find(Seg.Cell);
		if (!RoomA)
		{
			continue;
		}

		const FIntPoint NeighborCell = GetNeighborCell(Seg);
		int32* RoomB = CellToRoomIndex.Find(NeighborCell);

		TArray<int32> DummyDoorSegs;
		if (!CollectAlongWallSegmentIndices(i, DummyDoorSegs))
		{
			continue;
		}

		if (RoomB && *RoomB != *RoomA)
		{
			InteriorWallsPerRoom[*RoomA].Add(i);
			InteriorWallsPerRoom[*RoomB].Add(i);
		}
		else if (!RoomB)
		{
			ExteriorWalls.Add(i);
		}
	}

	for (int32 RoomIndex = 0; RoomIndex < RoomCount; ++RoomIndex)
	{
		TArray<int32>& Candidates = InteriorWallsPerRoom[RoomIndex];
		if (Candidates.Num() == 0)
		{
			continue;
		}

		const int32 DoorsThisRoom = Rng.RandRange(DoorsPerRoomMin, DoorsPerRoomMax);
		const int32 NumDoorsForRoom = FMath::Min(DoorsThisRoom, Candidates.Num());

		for (int32 d = 0; d < NumDoorsForRoom && Candidates.Num() > 0; ++d)
		{
			const int32 PickIdx = Rng.RandRange(0, Candidates.Num() - 1);
			const int32 SegIndex = Candidates[PickIdx];
			Candidates.RemoveAtSwap(PickIdx);

			if (!WallSegments.IsValidIndex(SegIndex))
			{
				continue;
			}

			FDungeonWallSegment& Seg = WallSegments[SegIndex];
			if (!Seg.WallActor.IsValid())
			{
				continue;
			}

			const FTransform DoorTransform = Seg.WallActor->GetActorTransform();

			DestroySegmentAndOppositeRun(SegIndex);
			SpawnDoorActorFromTransform(DoorTransform);
		}
	}

	TArray<int32> ValidPortalCandidates;
	for (int32 SegIndex : ExteriorWalls)
	{
		if (!WallSegments.IsValidIndex(SegIndex))
		{
			continue;
		}

		const FDungeonWallSegment& Seg = WallSegments[SegIndex];
		if (!Seg.WallActor.IsValid())
		{
			continue;
		}

		ValidPortalCandidates.Add(SegIndex);
	}

	if (ValidPortalCandidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: No valid exterior wall candidates found for portal."));
		return;
	}

	const int32 PortalPickIdx = Rng.RandRange(0, ValidPortalCandidates.Num() - 1);
	const int32 ChosenSegIndex = ValidPortalCandidates[PortalPickIdx];

	if (!WallSegments.IsValidIndex(ChosenSegIndex))
	{
		return;
	}

	FDungeonWallSegment& ChosenSeg = WallSegments[ChosenSegIndex];
	if (!ChosenSeg.WallActor.IsValid())
	{
		return;
	}

	AWallTile* ChosenWallActor = ChosenSeg.WallActor.Get();
	if (!ChosenWallActor)
	{
		return;
	}

	if (!ConfigurePortalForWall(ChosenWallActor, ChosenSeg))
	{
		return;
	}

	USceneComponent* PortalSpawnPoint = ChosenWallActor->GetPortalSpawnPoint();
	if (!PortalSpawnPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: Chosen wall has no PortalSpawnPoint."));
		return;
	}

	GeneratedWallActors.Add(ChosenWallActor);
	AddExteriorDoorWorld(
		PortalSpawnPoint->GetComponentLocation(),
		PortalSpawnPoint->GetComponentRotation());

	// UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: Portal wall chosen: %s  SpawnLoc=%s  SpawnRot=%s"),
	// 	*ChosenWallActor->GetName(),
	// 	*PortalSpawnPoint->GetComponentLocation().ToString(),
	// 	*PortalSpawnPoint->GetComponentRotation().ToString());
}


// Called every frame
void ABSP_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABSP_FloorGenerator::BuildCeiling()
{
	if (!FloorTileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator - FloorTileClass is null"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const float BaseMeshSize = 100.f;
	const float CeilingZ = FloorZ + WallHeight;

	for (int32 RoomIndex = 0; RoomIndex < LeafRegions.Num(); ++RoomIndex)
	{
		const FBSPLeaf& Leaf = LeafRegions[RoomIndex];

		const int32 LeafW = Leaf.Width();
		const int32 LeafH = Leaf.Height();
		if (LeafW <= 0 || LeafH <= 0) continue;

		int32 PadLeft   = FMath::Clamp(RoomPaddingMin, 0, LeafW - 1);
		int32 PadRight  = FMath::Clamp(RoomPaddingMin, 0, LeafW - 1 - PadLeft);
		int32 PadBottom = FMath::Clamp(RoomPaddingMin, 0, LeafH - 1);
		int32 PadTop    = FMath::Clamp(RoomPaddingMin, 0, LeafH - 1 - PadBottom);

		const int32 RoomMinX = Leaf.Min.X + PadLeft;
		const int32 RoomMaxX = Leaf.Max.X - PadRight;
		const int32 RoomMinY = Leaf.Min.Y + PadBottom;
		const int32 RoomMaxY = Leaf.Max.Y - PadTop;

		const int32 RoomW = RoomMaxX - RoomMinX;
		const int32 RoomH = RoomMaxY - RoomMinY;
		if (RoomW <= 0 || RoomH <= 0) continue;

		const float RoomWorldWidth  = RoomW * TileSize;
		const float RoomWorldHeight = RoomH * TileSize;

		const FVector RoomCenter(
			(RoomMinX + RoomW * 0.5f) * TileSize,
			(RoomMinY + RoomH * 0.5f) * TileSize,
			CeilingZ);

		const FVector CeilingLocation = GetActorLocation() + RoomCenter;
		const FVector CeilingScale(RoomWorldWidth / BaseMeshSize, RoomWorldHeight / BaseMeshSize, 1.f);

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AFloorTile* CeilingActor = World->SpawnActor<AFloorTile>(FloorTileClass, CeilingLocation, FRotator(180.f, 0.f, 0.f), Params);
		if (!CeilingActor) continue;

		GeneratedCeilingActors.Add(CeilingActor);

		if (UStaticMeshComponent* MeshComp = CeilingActor->GetItemMesh())
		{
			MeshComp->SetMobility(EComponentMobility::Movable);
			CeilingActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			CeilingActor->SetActorScale3D(CeilingScale);
		}
		else
		{
			CeilingActor->Destroy();
		}
	}
}