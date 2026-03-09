// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalManager.generated.h"

class APortal;

UCLASS()
class PROCEDURALDUNGEON4_API APortalManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = Portal)
	TSubclassOf<APortal> PortalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	APortal* HomePortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	APortal* AwayPortal;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	APortal* SpawnAwayPortal(FVector Location, FRotator Rotation);

	UFUNCTION()
	void DestroyAwayPortal();

};
