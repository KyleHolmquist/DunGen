// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HitInterface.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "DunGenEnums.h"
#include "BreakableActor.generated.h"

class UGeometryCollectionComponent;
class ATreasure;

UCLASS()
class PROCEDURALDUNGEON4_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:
	ABreakableActor();
	virtual void Tick(float DeltaTime) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	void HandleHit();
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Dungeon")
	TArray<EDungeonTheme> DungeonThemes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UGeometryCollectionComponent* GeometryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class TSubclassOf<ATreasure> TreasureClass;

	UFUNCTION()
	void OnChaosBreakEvent(const FChaosBreakEvent& BreakEvent);

private:

	UPROPERTY(EditAnywhere, Category = "Breakable Properties")
	TArray<TSubclassOf<class ATreasure>> TreasureClasses;

	bool bBroken = false;

public:

	UFUNCTION()
	void SetTreasureClass(TSubclassOf<ATreasure> SelectedTreasureClass );

	FORCEINLINE TArray<EDungeonTheme> GetThemes() const { return DungeonThemes; }


};