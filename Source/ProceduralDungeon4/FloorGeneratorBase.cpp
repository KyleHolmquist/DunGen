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
        W.Location = ModuleXf.TransformPosition(Door.Location); // now used as "world location"
        W.Rotation = ModuleXf.TransformRotation(Door.Rotation.Quaternion()).Rotator(); // now "world rotation"
        Out.Add(W);
    }
    return Out;
}