// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DunGenEnums.h"
#include "Tile.generated.h"

class UStaticMeshComponent;

UCLASS()
class PROCEDURALDUNGEON4_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Dungeon")
	TArray<EDungeonTheme> DungeonThemes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Dungeon")
	ETileType TileType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Dungeon")
	UStaticMeshComponent* ItemMesh;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE TArray<EDungeonTheme> GetThemes() const { return DungeonThemes; }
	FORCEINLINE ETileType GetTileType() const { return TileType; }
	FORCEINLINE UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }

};
