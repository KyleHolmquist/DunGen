#include "FloorGeneratorBase.h"

AFloorGeneratorBase::AFloorGeneratorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	RootComponent->SetMobility(EComponentMobility::Movable);
}

void AFloorGeneratorBase::GenerateModule()
{
	//Base version does nothing. Derived classes override.
}

void AFloorGeneratorBase::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		GenerateModule();
	}
}

void AFloorGeneratorBase::AddExteriorDoorWorld(const FVector& WorldLocation, const FRotator& WorldRotation)
{
    FExteriorDoor Door;
    const FTransform ModuleXf = GetActorTransform();
    Door.LocalLocation = ModuleXf.InverseTransformPosition(WorldLocation);
    Door.LocalRotation = ModuleXf.InverseTransformRotation(WorldRotation.Quaternion()).Rotator();
    ExteriorDoors.Add(Door);
}

TArray<FExteriorDoor> AFloorGeneratorBase::GetExteriorDoorsWorld() const
{
    TArray<FExteriorDoor> Out;
    Out.Reserve(ExteriorDoors.Num());

    const FTransform ModuleXf = GetActorTransform();

    for (const FExteriorDoor& Door : ExteriorDoors)
    {
        FExteriorDoor W = Door;

        W.Location = ModuleXf.TransformPosition(Door.LocalLocation);
        W.Rotation = ModuleXf.TransformRotation(Door.LocalRotation.Quaternion()).Rotator();
		
        Out.Add(W);
    }
    return Out;
}

FBox AFloorGeneratorBase::GetModuleBoundsWorld(float ZMin, float ZMax) const
{
	const float Width = MapWidth * TileSize;
	const float Height = MapHeight * TileSize;

	const FTransform Transform = GetActorTransform();

	//Build an AABB in world space by transforming the four footprint corners
	const FVector Local00(0.f, 0.f, ZMin);
	const FVector Local10(Width, 0.f, ZMin);
	const FVector Local01(0.f, Height, ZMin);
	const FVector Local11(Width, Height, ZMin);

	const FVector Local00_Top(0.f, 0.f, ZMax);
	const FVector Local10_Top(Width, 0.f, ZMax);
	const FVector Local01_Top(0.f, Height, ZMax);
	const FVector Local11_Top(Width, Height, ZMax);

	FBox Box (ForceInit);

	Box += Transform.TransformPosition(Local00);
	Box += Transform.TransformPosition(Local10);
	Box += Transform.TransformPosition(Local01);
	Box += Transform.TransformPosition(Local11);

	Box += Transform.TransformPosition(Local00_Top);
	Box += Transform.TransformPosition(Local10_Top);
	Box += Transform.TransformPosition(Local01_Top);
	Box += Transform.TransformPosition(Local11_Top);

	return Box;

}

bool AFloorGeneratorBase::IsEmpty(int32 X, int32 Y) const
{
	return false;
}

void AFloorGeneratorBase::SetMapSize(int SelectedMapWidth, int SelectedMapHeight)
{
	MapWidth = SelectedMapWidth;
	MapHeight = SelectedMapHeight;
}

void AFloorGeneratorBase::SetFloorTile(TSubclassOf<AFloorTile> SelectedFloorTileClass)
{
	FloorTileClass = SelectedFloorTileClass;
}
void AFloorGeneratorBase::SetWallTile(TSubclassOf<AWallTile> SelectedWallTileClass)
{
	WallTileClass = SelectedWallTileClass;
}
void AFloorGeneratorBase::SetDoorTile(TSubclassOf<AWallTile> SelectedWallTileClass)
{
	DoorTileClass = SelectedWallTileClass;
}

void AFloorGeneratorBase::SetDungeonManager(ADungeonManager* OwningDungeonManager)
{
	DungeonManager = OwningDungeonManager;
}