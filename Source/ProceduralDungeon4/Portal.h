// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UParticleSystemComponent;
class USoundBase;
class UBoxComponent;
class AAirsto;
class ADungeonManager;

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
	UFUNCTION()
	void TeleportPlayer(AAirsto* Airsto, FVector Location);
	
	void SpawnEntrySound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	UStaticMeshComponent* PortalMesh;

	UPROPERTY(EditAnywhere, Category = Portal)
	UParticleSystemComponent* PortalEffect;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* EntryBox;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ExitBox;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TeleportBox;

	UPROPERTY(EditAnywhere, Category = Portal)
	APortal* ConnectedPortal;
	
	UPROPERTY(VisibleAnywhere, Category = Portal)
	bool bCanTeleport = true;

	UPROPERTY()
	bool bTeleportLocationSet = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	FVector TeleportLocation;

	UPROPERTY()
	ADungeonManager* DungeonManager;

private:

	UPROPERTY(EditAnywhere, Category = Portal)
	USoundBase* TeleportSound;

	UPROPERTY(EditAnywhere, Category = Portal)
	USoundBase* PortalHumSound;

public:	

	FORCEINLINE FVector GetPortalLocation() { return GetActorLocation(); }
	FORCEINLINE FVector GetTeleportLocation() { return TeleportLocation; }
	FVector GetTeleportPointLocation() const;
	void SetTeleportLocation(FVector SelectedTeleportLocation);
	void SetConnectedPortal(APortal* InPortal) { ConnectedPortal = InPortal; }
	

};
