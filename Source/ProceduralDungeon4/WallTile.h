// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "WallTile.generated.h"

class USceneComponent;

UCLASS()
class PROCEDURALDUNGEON4_API AWallTile : public ATile
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWallTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Portal)
	USceneComponent* PortalFacingRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Portal)
	USceneComponent* PortalSpawnPoint;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	USceneComponent* GetPortalFacingRoot() const { return PortalFacingRoot; }
	USceneComponent* GetPortalSpawnPoint() const { return PortalSpawnPoint;}

};
