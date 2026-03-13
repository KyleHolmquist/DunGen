// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalManager.h"
#include "Portal.h"

// Sets default values
APortalManager::APortalManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APortalManager::BeginPlay()
{
	Super::BeginPlay();

	if (HomePortal && AwayPortal)
	{
		HomePortal->SetTeleportLocation(AwayPortal->GetTeleportPointLocation());
		AwayPortal->SetTeleportLocation(HomePortal->GetTeleportPointLocation());
	}
	
}

// Called every frame
void APortalManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

APortal* APortalManager::SpawnAwayPortal(FVector Location, FRotator Rotation)
{
	UWorld* World = GetWorld();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APortal* PortalActor = World->SpawnActor<APortal>(PortalClass, Location, Rotation, Params);

	if (!PortalActor) return nullptr;

	AwayPortal = PortalActor;

	if (HomePortal && AwayPortal)
	{
		HomePortal->SetConnectedPortal(AwayPortal);
		AwayPortal->SetConnectedPortal(HomePortal);

		AwayPortal->SetCanTeleport(false);
	}

	return PortalActor;
}

void APortalManager::DestroyAwayPortal()
{
	if (AwayPortal)
	{
		AwayPortal->Destroy();
		AwayPortal = nullptr;
	}
	
}

void APortalManager::SetHomePortalTeleportLocation()
{
	if (HomePortal && AwayPortal)
	{
		HomePortal->SetTeleportLocation(AwayPortal->GetTeleportPointLocation());
	}
}