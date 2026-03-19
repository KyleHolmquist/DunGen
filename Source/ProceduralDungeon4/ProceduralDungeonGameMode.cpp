// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralDungeonGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AProceduralDungeonGameMode::AProceduralDungeonGameMode()
{
    
}

void AProceduralDungeonGameMode::SpawnPlayerAtTransform(const FTransform& SpawnTransform)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerAtTransform - No PlayerController found."));
        return;
    }

    if (APawn* OldPawn = PC->GetPawn())
    {
        PC->UnPossess();
        OldPawn->Destroy();
    }

    RestartPlayerAtTransform(PC, SpawnTransform);

    UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerAtTransform - Restarted player at transform."));
}