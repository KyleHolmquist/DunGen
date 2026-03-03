#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "DunGenEnums.h"
#include "DungeonManager.h"
#include "WallTile.h"
#include "FloorTile.h"
#include "FloorGeneratorBase.generated.h"

class AFloorTile;
class AWallTile;

USTRUCT(BlueprintType)
struct FExteriorDoor
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FRotator Rotation = FRotator::ZeroRotator;

    // Stored relative to the module actor
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector LocalLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FRotator LocalRotation = FRotator::ZeroRotator;
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
    UPROPERTY(VisibleAnywhere)
    FBox ModuleBoundsLocal;

    //Deterministic module bounds
    UFUNCTION(BlueprintCallable, Category="Dungeon")
    FBox GetModuleBoundsWorld(float ZMin, float ZMax) const;

    UPROPERTY()
    USceneComponent* Root = nullptr;

    // Adds a door using a world-space transform; stores it in local space.
    void AddExteriorDoorWorld(const FVector& WorldLocation, const FRotator& WorldRotation);

    // Returns doors in world space (computed from stored local data).
    TArray<FExteriorDoor> GetExteriorDoorsWorld() const;

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

    void SetMapSize(int SelectedMapWidth, int SelectedMapHeight);
    void SetFloorTile(TSubclassOf<AFloorTile> SelectedFloorTileClass);
    void SetWallTile(TSubclassOf<AWallTile> SelectedWallTileClass);
    void SetDoorTile(TSubclassOf<AWallTile> SelectedWallTileClass);

    void SetDungeonManager(ADungeonManager* OwningDungeonManager);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dungeon")
    bool bGenerateOnBeginPlay = false;

    UPROPERTY(BlueprintReadOnly)
    bool bHasFinishedGenerating = false;

    UFUNCTION(BlueprintCallable, Category=Dungeon)
    virtual bool IsEmpty(int32 X, int32 Y) const;
    

    void InitMapSize(int32 InWidth, int32 InHeight)
    {
        MapWidth = InWidth;
        MapHeight = InHeight;
    }

	FORCEINLINE TArray<EDungeonTheme> GetThemes() const { return DungeonThemes; }
    FORCEINLINE int32 GetMapWidth() const { return MapWidth; }
    FORCEINLINE int32 GetMapHeight() const { return MapHeight; }


protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floor")
    int32 MapWidth = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floor")
    int32 MapHeight = 40;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floor")
    float TileSize = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	TArray<EDungeonTheme> DungeonThemes;

	//Mesh for the floor
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	TSubclassOf<AFloorTile> FloorTileClass;

	//Mesh for the Walls
	UPROPERTY(EditAnywhere, Category = "Walls")
	TSubclassOf<AWallTile> WallTileClass;

    //Mesh for the Doors
	UPROPERTY(EditAnywhere, Category = "Walls")
	TSubclassOf<AWallTile> DoorTileClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	ADungeonManager* DungeonManager;

    UPROPERTY(VisibleAnywhere)
    TArray<FVector> GeneratedEmptyLocations;

    UPROPERTY(VisibleAnywhere)
    TArray<FVector> GeneratedDoorLocations;

public:
    FORCEINLINE const TArray<FVector>& GetGeneratedEmptyLocations() const { return GeneratedEmptyLocations; }
    FORCEINLINE const TArray<FVector>& GetGeneratedDoorLocations() const { return GeneratedDoorLocations; }
};