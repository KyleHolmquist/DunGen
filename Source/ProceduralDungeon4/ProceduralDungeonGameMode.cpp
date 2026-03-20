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

void AProceduralDungeonGameMode::SpawnPlayerAtMainSpawn()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerAtMainSpawn - No PlayerController found."));
        return;
    }

    AActor* MainSpawn = nullptr;
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);

    for (AActor* Actor : PlayerStarts)
    {
        if (Actor && Actor->ActorHasTag(FName("MainSpawn")))
        {
            MainSpawn = Actor;
            break;
        }
    }

    if (!MainSpawn && PlayerStarts.Num() > 0)
    {
        MainSpawn = PlayerStarts[0];
    }

    if (!MainSpawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerAtMainSpawn - No PlayerStart found."));
        return;
    }

    if (APawn* OldPawn = PC->GetPawn())
    {
        PC->UnPossess();
        OldPawn->Destroy();
    }

    RestartPlayerAtTransform(PC, MainSpawn->GetActorTransform());

    UE_LOG(LogTemp, Warning, TEXT("SpawnPlayerAtMainSpawn - Restarted player at main spawn."));
}