// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomGenerator.h"
#include "GridSpace.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProceduralDungeonGameMode.h"
#include "DungeonRoom.h"

// Sets default values
ARoomGenerator::ARoomGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARoomGenerator::BeginPlay()
{
	Super::BeginPlay();

	
		
}

// Called every frame
void ARoomGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


