// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProceduralDungeonGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API AProceduralDungeonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AProceduralDungeonGameMode();

	UFUNCTION(BlueprintCallable)
	void SpawnPlayerAtTransform(const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable)
	void SpawnPlayerAtMainSpawn();

protected:
	
};
