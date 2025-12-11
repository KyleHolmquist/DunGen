#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorGeneratorBase.generated.h"

USTRUCT(BlueprintType)
struct FExteriorDoor
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FRotator Rotation = FRotator::ZeroRotator;
};

UCLASS(Abstract)
class PROCEDURALDUNGEON4_API AFloorGeneratorBase : public AActor
{
    GENERATED_BODY()

public:
    AFloorGeneratorBase();

    //How many exterior doors this module tries to carve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dungeon")
    int32 DesiredExteriorDoors = 2;

    //Filled by the generator when it creates exterior doors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Floor")
    TArray<FExteriorDoor> ExteriorDoors;

    //Called by the Dungeon Manager instead of BeginPlay
    UFUNCTION(BlueprintCallable, Category="Floor")
    virtual void GenerateFloor() PURE_VIRTUAL(AFloorGeneratorBase::GenerateFloor, );

    //Axis-aligned bounds in world space to keep spaces from overlapping
    UFUNCTION(BlueprintCallable, Category="Floor")
    virtual FBox GetFloorBounds() const PURE_VIRTUAL(AFloorGeneratorBase::GetFloorBounds, return FBox(EForceInit::ForceInitToZero););

    //Public getter function
    const TArray<FExteriorDoor>& GetExteriorDoors() const { return ExteriorDoors;}

    //Entry point the manager calls after spawn
    UFUNCTION(BlueprintCallable, Category ="Dungeon")
    virtual void GenerateModule();
};