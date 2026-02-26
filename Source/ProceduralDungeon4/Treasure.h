// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Treasure.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API ATreasure : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(EditAnywhere, Category = "Treasure Properties")
	FString DisplayName;
	

private:

	UPROPERTY(EditAnywhere, Category = "Treasure Properties")
	int32 Gold;


public:
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE FString GetDisplayName() const { return DisplayName; }
	
};
