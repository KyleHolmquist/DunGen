// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DunGenEnums.h"
#include "HitInterface.h"
#include "Prop.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API AProp : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProp();

    virtual void GetHit_Implementation(const FVector &ImpactPoint, AActor *Hitter) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	TArray<EDungeonTheme> DungeonThemes;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE TArray<EDungeonTheme> GetDungeonThemes() const { return DungeonThemes; }

};
