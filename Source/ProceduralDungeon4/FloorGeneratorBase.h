#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorGeneratorBase.generated.h"

USTRUCT(BlueprintType)
struct FExternalDoorInfo
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
    
    UPROPERTY(EditAnywhere, Category="Floor")
    int32 GridWidth = 40;

    UPROPERTY(EditAnywhere, Category="Floor")
    int32 GridHeight = 30;

    UPROPERTY(EditAnywhere, Category="Floor")
    float TileSize = 100.f;

    //Filled by the generator when it creates exterior doors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Floor")
    TArray<FExternalDoorInfo> ExteriorDoors;

    //Called by the Dungeon Manager instead of BeginPlay
    UFUNCTION(BlueprintCallable, Category="Floor")
    virtual void GenerateFloor() PURE_VIRTUAL(AFloorGeneratorBase::GenerateFloor, );

    //Axis-aligned bounds in world space to keep spaces from overlapping
    UFUNCTION(BlueprintCallable, Category="Floor")
    virtual FBox GetFloorBounds() const PURE_VIRTUAL(AFloorGeneratorBase::GetFloorBounds, return FBox(EForceInit::ForceInitToZero););

    void GetExteriorDoors(TArray<FExternalDoorInfo>& OutDoors) const
    {
        OutDoors = ExteriorDoors;
    }
};