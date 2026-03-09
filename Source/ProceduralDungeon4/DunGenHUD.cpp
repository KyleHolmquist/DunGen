// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenHUD.h"
#include "DunGenOverlay.h"
#include "DunGenDialogueOverlay.h"

void ADunGenHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* Controller = GetOwningPlayerController();
    if (Controller && DunGenOverlayClass)
    {
        DunGenOverlay = CreateWidget<UDunGenOverlay>(Controller, DunGenOverlayClass);
        if (DunGenOverlay)
        {
            DunGenOverlay->AddToViewport();
        }
    }

    //Create the dialogue overlay
    if (Controller && DunGenDialogueOverlayClass)
    {
        DunGenDialogueOverlay = CreateWidget<UDunGenDialogueOverlay>(Controller, DunGenDialogueOverlayClass);
    }

    // UWorld* World = GetWorld();
    // if (World)
    // {
    //     APlayerController* Controller = World->GetFirstPlayerController();
    //     if (Controller && DunGenOverlayClass)
    //     {
    //         DunGenOverlay = CreateWidget<UDunGenOverlay>(Controller, DunGenOverlayClass);
    //         DunGenOverlay->AddToViewport();
    //     }
    // }
    
}

void ADunGenHUD::ShowDialogueOverlay()
{
    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    if (!DunGenDialogueOverlay && DunGenDialogueOverlayClass)
    {
        DunGenDialogueOverlay = CreateWidget<UDunGenDialogueOverlay>(Controller, DunGenDialogueOverlayClass);
    }

    if (DunGenDialogueOverlay && !DunGenDialogueOverlay->IsInViewport())
    {
        DunGenDialogueOverlay->AddToViewport();
    }
}

void ADunGenHUD::HideDialogueOverlay()
{
    if (DunGenDialogueOverlay && DunGenDialogueOverlay->IsInViewport())
    {
        DunGenDialogueOverlay->RemoveFromParent();
    }
}