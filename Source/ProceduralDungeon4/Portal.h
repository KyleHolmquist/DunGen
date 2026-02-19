// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UParticleSystemComponent;
class USoundBase;
class UBoxComponent;

UCLASS()
class PROCEDURALDUNGEON4_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void SpawnEntrySound();

	void TeleportPlayer();

	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* PortalEffect;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* EntryBox;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ExitBox;
	
	UPROPERTY(VisibleAnywhere)
	bool bCanTeleport = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TeleportLocation;

private:

	UPROPERTY(EditAnywhere)
	USoundBase* TeleportSound;

	UPROPERTY(EditAnywhere)
	USoundBase* PortalHumSound;

public:	

	FORCEINLINE FVector GetTeleportLocation() const { return TeleportLocation; }
	void SetTeleportLocation(FVector SelectedTeleportLocation);
	

};
