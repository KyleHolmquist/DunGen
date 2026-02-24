// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenHUD.h"
#include "DunGenOverlay.h"

void ADunGenHUD::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* Controller = World->GetFirstPlayerController();
        if (Controller && DunGenOverlayClass)
        {
            DunGenOverlay = CreateWidget<UDunGenOverlay>(Controller, DunGenOverlayClass);
            DunGenOverlay->AddToViewport();
        }
    }

    
}