// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API AHealthPickup : public AItem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Health Properties")
	int32 HealthAmount;

	double DesiredZ;

	UPROPERTY(EditAnywhere)
	float DriftRate = -15.f;
	

public:
	FORCEINLINE int32 GetHealthAmount() const { return HealthAmount; }
	FORCEINLINE void SetHealthAmount(int32 HealthQuantity) { HealthAmount = HealthQuantity; }
	
	
};
