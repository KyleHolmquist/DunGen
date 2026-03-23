// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenHUD.h"
#include "DunGenOverlay.h"
#include "DunGenDialogueOverlay.h"
#include "DunGenPauseMenu.h"
#include "DunGenControlsMenu.h"
#include "DunGenMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonManager.h"
#include "DunGenNameEntryMenu.h"


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
    
    ShowMainMenu();
    
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

void ADunGenHUD::StartGame()
{
    UE_LOG(LogTemp, Warning, TEXT("DunGen:: StartGame"));

    HideMainMenu();

    if (ADungeonManager* DungeonManager = Cast<ADungeonManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonManager::StaticClass())))
    {
        
        DungeonManager->BeginNameEntryFlow();
    }
}

void ADunGenHUD::ShowPauseMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("DunGen:: ShowPauseMenu"));
    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    if (!DunGenPauseMenu && DunGenPauseMenuClass)
    {
        DunGenPauseMenu = CreateWidget<UDunGenPauseMenu>(Controller, DunGenPauseMenuClass);
    }

    if (!DunGenPauseMenu) return;

    if (!DunGenPauseMenu->IsInViewport())
    {
        DunGenPauseMenu->AddToViewport();
    }

    UGameplayStatics::SetGamePaused(GetWorld(), true);

    Controller->bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(DunGenPauseMenu->TakeWidget());
    Controller->SetInputMode(InputMode);
}

void ADunGenHUD::HidePauseMenu()
{
    if (DunGenPauseMenu && DunGenPauseMenu->IsInViewport())
    {
        DunGenPauseMenu->RemoveFromParent();
    }

    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    UGameplayStatics::SetGamePaused(GetWorld(), false);

    Controller->bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    Controller->SetInputMode(InputMode);
}

void ADunGenHUD::TogglePauseMenu()
{
    if (DunGenMainMenu && DunGenMainMenu->IsInViewport())
    {
        return;
    }

    if (DunGenPauseMenu && DunGenPauseMenu->IsInViewport())
    {
        HidePauseMenu();
    }
    else
    {
        ShowPauseMenu();
    }
}

void ADunGenHUD::ShowControlsMenu(EControlsMenuSource Source)
{
    UE_LOG(LogTemp, Warning, TEXT("DunGen:: ShowControlsMenu"));

    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    ControlsMenuSource = Source;

    if (!DunGenControlsMenu && DunGenControlsMenuClass)
    {
        DunGenControlsMenu = CreateWidget<UDunGenControlsMenu>(Controller, DunGenControlsMenuClass);
    }

    if (!DunGenControlsMenu) return;

    //Hide whichever parent menu opened this
    if (Source == EControlsMenuSource::PauseMenu)
    {
        if (DunGenPauseMenu && DunGenPauseMenu->IsInViewport())
        {
            DunGenPauseMenu->RemoveFromParent();
        }

        UGameplayStatics::SetGamePaused(GetWorld(), true);
    }
    else if (Source == EControlsMenuSource::MainMenu)
    {
        if (DunGenMainMenu && DunGenMainMenu->IsInViewport())
        {
            DunGenMainMenu->RemoveFromParent();
        }

        UGameplayStatics::SetGamePaused(GetWorld(), false);
    }

    if (!DunGenControlsMenu->IsInViewport())
    {
        DunGenControlsMenu->AddToViewport();
    }

    Controller->bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(DunGenControlsMenu->TakeWidget());
    Controller->SetInputMode(InputMode);
}

void ADunGenHUD::ReturnFromControlsMenu()
{
    //Remove controls menu if it's on screen
    if (DunGenControlsMenu && DunGenControlsMenu->IsInViewport())
    {
        DunGenControlsMenu->RemoveFromParent();
    }

    switch (ControlsMenuSource)
    {
        case EControlsMenuSource::PauseMenu:
            ShowPauseMenu();
            break;

        case EControlsMenuSource::MainMenu:
            ShowMainMenu();
            break;

        default:
            break;
    }

    ControlsMenuSource = EControlsMenuSource::None;
}

void ADunGenHUD::ShowMainMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("ShowMainMenu triggered"));

    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    if (!DunGenMainMenu && DunGenMainMenuClass)
    {
        DunGenMainMenu = CreateWidget<UDunGenMainMenu>(Controller, DunGenMainMenuClass);
    }

    if (!DunGenMainMenu) return;

    if (!DunGenMainMenu->IsInViewport())
    {
        DunGenMainMenu->AddToViewport();
    }

    UGameplayStatics::SetGamePaused(GetWorld(), false);
    
    Controller->bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(DunGenMainMenu->TakeWidget());
    Controller->SetInputMode(InputMode);
}

void ADunGenHUD::HideMainMenu()
{
    if (DunGenMainMenu && DunGenMainMenu->IsInViewport())
    {
        DunGenMainMenu->RemoveFromParent();
    }

    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    Controller->bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    Controller->SetInputMode(InputMode);
}

void ADunGenHUD::GoToMainMenuFromPause()
{
    //Remove pause menu if it's on screen
    if (DunGenPauseMenu && DunGenPauseMenu->IsInViewport())
    {
        DunGenPauseMenu->RemoveFromParent();
    }

    //Ensure game is unpaused
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    //Show main menu
    ShowMainMenu();
}

void ADunGenHUD::ShowNameEntryMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("DunGen:: ShowNameEntryMenu"));

    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    if (!DunGenNameEntryMenu && DunGenNameEntryMenuClass)
    {
        DunGenNameEntryMenu = CreateWidget<UDunGenNameEntryMenu>(Controller, DunGenNameEntryMenuClass);
    }

    if (!DunGenNameEntryMenu) return;

    if (!DunGenNameEntryMenu->IsInViewport())
    {
        DunGenNameEntryMenu->AddToViewport();
    }

    Controller->bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(DunGenNameEntryMenu->TakeWidget());
    Controller->SetInputMode(InputMode);
}

void ADunGenHUD::HideNameEntryMenu()
{
    if (DunGenNameEntryMenu && DunGenNameEntryMenu->IsInViewport())
    {
        DunGenNameEntryMenu->RemoveFromParent();
    }

    APlayerController* Controller = GetOwningPlayerController();
    if (!Controller) return;

    Controller->bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    Controller->SetInputMode(InputMode);
}