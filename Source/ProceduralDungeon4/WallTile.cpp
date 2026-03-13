// Fill out your copyright notice in the Description page of Project Settings.


#include "WallTile.h"
#include "Components/SceneComponent.h"

// Sets default values
AWallTile::AWallTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PortalSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PortalSpawnPoint"));
	PortalSpawnPoint->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AWallTile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWallTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

USceneComponent* AWallTile::GetPortalSpawnPoint() const
{
	return PortalSpawnPoint;
}


