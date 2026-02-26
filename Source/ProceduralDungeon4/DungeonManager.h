// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DunGenEnums.h"
#include "DungeonManager.generated.h"

class AFloorTile;
class AWallTile;
class ATreasure;
class ABreakableActor;
class AEnemy;
class AProp;
class AFloorGeneratorBase;
struct FExteriorDoor;
class UDataTable;
struct FQuestAdjectiveRow;

UCLASS()
class PROCEDURALDUNGEON4_API ADungeonManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Which moduule types to spawn for test
	UPROPERTY(EditAnywhere, Category="Dungeon")
	TSubclassOf<AFloorGeneratorBase> FirstModuleClass;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	TSubclassOf<AFloorGeneratorBase> SecondModuleClass;

	//Floor Size
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 MapWidth = 30; 
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 MapHeight = 30;

	//Holmquist Size
	UPROPERTY(EditAnywhere, Category = "Dungeon")
	int32 HolmquistTiles = 80;

	//Desired doors
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 FirstModuleDoors = 2;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	int32 SecondModuleDoors = 2;

	//How far to separate modules so they don't overlap
	UPROPERTY(EditAnywhere, Category="Dungeon")
	float ModuleSeparation = 400.f;

	UPROPERTY(EditAnywhere, Category="Dungeon")
	float MinGapBetweenModules = 400.f;

	//Corridor meshes
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorFloorMesh;
	
	UPROPERTY(EditAnywhere, Category="Dungeon")
	UStaticMesh* CorridorWallMesh;

	// -- Corridors --
	bool AlignSecondModuleToFirst(AFloorGeneratorBase* ModuleA, AFloorGeneratorBase* ModuleB, FExteriorDoor& OutDoorA, FExteriorDoor& OutDoorB, int32& OutDoorBIndex);
	void BuildTiledCorridor(const FVector& FromWorld, const FVector& ToWorld, const FVector& GridOrigin, float TileSize, float Z);
	void BuildStraightCorridor(const FVector& FromWorld, const FVector& ToWorld, float TileSize, float Z);
	void BuildCorridor(const FExteriorDoor& Door, int32 NumTiles, float TileSize, float Z);
	bool BuildCorridorFromDoorOnBounds(AFloorGeneratorBase* Module, const FExteriorDoor& Door, int32 NumTiles, float TileSize, float Z, FVector& OutForward, FVector& OutLastTileWorld);

	//----Theme----

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	EDungeonTheme DungeonTheme;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<AFloorGeneratorBase> SelectedFloorGeneratorClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<AFloorTile> SelectedFloorTileClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<AWallTile> SelectedWallTileClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<ATreasure> SelectedTreasureClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<ABreakableActor> SelectedBreakableClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<AEnemy> SelectedEnemyClass;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TSubclassOf<AProp> SelectedPropClass;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<EDungeonTheme> DungeonThemes;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AFloorGeneratorBase>> FloorGenerators;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AFloorTile>> FloorTiles;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AWallTile>> WallTiles;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<ATreasure>> Treasures;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<ABreakableActor>> Breakables;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AEnemy>> Enemies;

	UPROPERTY(EditAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AProp>> Props;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AFloorGeneratorBase>> ThemedFloorGenerators;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AFloorTile>> ThemedFloorTiles;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AWallTile>> ThemedWallTiles;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<ATreasure>> ThemedTreasures;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<ABreakableActor>> ThemedBreakables;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AEnemy>> ThemedEnemies;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<TSubclassOf<AProp>> ThemedProps;

	bool bThemedFloorGeneratorsPopulated = false;
	bool bThemedFloorTilesPopulated = false;
	bool bThemedWallTilesPopulated = false;
	bool bThemedTreasuresPopulated = false;
	bool bThemedBreakablesPopulated = false;
	bool bThemedEnemiesPopulated = false;
	bool bThemedPropsPopulated = false;

	void SetDungeonTheme();
	void SetFloorGenerator();
	void SetFloorTile();
	void SetWallTile();
	void SetTreasure();
	void SetBreakable();
	void SetEnemy();
	void SetProp();

	// ---- End theme ----

	// ---- Dungeon Spawning ----

	int AccruedWisdom;
	int DungeonSize;
	int BreakableQuantityInLevel;
	int PropQuantityInLevel;
	int EnemyQuantityInLevel;
	int EnemyHealth;
	int EnemyWisdom;
	int TotalEmptySpaces;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	AFloorGeneratorBase* DungeonModule = nullptr;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<FIntPoint> EmptySpaces;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<FVector> EmptyLocations;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<AActor*> SpawnedActors;

	UPROPERTY(VisibleAnywhere, Category=Dungeon)
	TArray<AEnemy*> SpawnedEnemies;

	FTimerHandle PatrolDelayHandle;

	void InitializeDungeonLevelParams();
	void SetDungeonSize(int WisdomAmount);
	AFloorGeneratorBase* SpawnDungeonModule();
	void AddEmptySpacesToArray();
	void PopulateDungeon();
	void SpawnBreakables();
	void SpawnProps();
	void SpawnEnemies();
	void SetNumBreakablesInLevel();
	void SetNumPropsInLevel();
	void SetNumEnemiesInLevel();
	int SetEnemyHealth();
	int SetEnemyWisdom();
	void SetEnemyPatrolPoints(AEnemy* Enemy);
	void SetEnemyAttackTimer(AEnemy* Enemy);
	void StartPatrollingDelayed();

	// ---- Quest Text ----
	UPROPERTY(EditDefaultsOnly, Category="Quest Text")
	UDataTable* QuestAdjectivesTable;

	bool GetRandomAdjectiveValue(const UDataTable* Table, FString FQuestAdjectiveRow::* Field, FString& OutValue);
	FText GenerateQuestText(const UDataTable* AdjectiveTable, const FString& PlayerName,const FString& SelectedThemeName, const FString& SelectedTreasureName);

private:

	AFloorGeneratorBase* FirstModule = nullptr;
	AFloorGeneratorBase* SecondModule = nullptr;

	AFloorGeneratorBase* SpawnModule(TSubclassOf<AFloorGeneratorBase> ModuleClass, const FVector& Location, int32 DesiredDoors);
	

    UFUNCTION(BlueprintCallable, Category="Dungeon")
    AFloorGeneratorBase* SpawnConfiguredModule(
        TSubclassOf<AFloorGeneratorBase> ModuleClass,
        const FTransform& SpawnTransform,
        int32 InMapWidth,
        int32 InMapHeight,
        int32 InDoorCount
    );

	void ConnectModulesWithCorridor(AFloorGeneratorBase* A, AFloorGeneratorBase* B);
	void TryConnectModules();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	EDungeonTheme GetRandomDungeonTheme() const;
	TSubclassOf<AFloorGeneratorBase> GetRandomFloorGenerator();
	TSubclassOf<AFloorTile> GetRandomFloorTile();
	TSubclassOf<AWallTile> GetRandomWallTile();
	TSubclassOf<ATreasure> GetRandomTreasure();
	TSubclassOf<ABreakableActor> GetRandomBreakable();
	TSubclassOf<AEnemy> GetRandomEnemy();
	TSubclassOf<AProp> GetRandomProp();

	void AddToEmptyLocationsArray(FVector Location);


};
