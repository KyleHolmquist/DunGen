// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenDialogueOverlay.h"
#include "Components/Border.h"
#include "MyDialogueTypes.h"
#include "DunGenDialogueOverlay.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "DialogueOptionEntry.h"
#include "Blueprint/WidgetTree.h"
#include "Airsto.h"

void UDunGenDialogueOverlay::SetName(FString DialogueName)
{
    if (NameText)
    {
        NameText->SetText(FText::FromString(DialogueName));
    }
}

void UDunGenDialogueOverlay::SetDialogueText(const FText& InText)
{
    if (DialogueText)
    {
        DialogueText->SetText(InText);
    }
}

void UDunGenDialogueOverlay::ShowDialogueOptions(const TArray<FDialogueOption>& Options)
{
    if (!DialogueOptionsBox)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueOptionsBox is null in ShowDialogueOptions"));
        return;
    }

    if (!DialogueOptionButtonClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueOptionButtonClass is null in ShowDialogueOptions"));
        return;
    }

    DialogueOptionsBox->ClearChildren();
    DialogueOptionsBox->SetVisibility(ESlateVisibility::Visible);

    UE_LOG(LogTemp, Warning, TEXT("ShowDialogueOptions called with %d options"), Options.Num());

    for (int32 i = 0; i < Options.Num(); ++i)
    {
        UDialogueOptionEntry* OptionEntry = CreateWidget<UDialogueOptionEntry>(GetWorld(), DialogueOptionButtonClass);

        if (!OptionEntry)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create DialogueOptionEntry at index %d"), i);
            continue;
        }

        OptionEntry->SetupOption(Options[i].OptionText, i);
        OptionEntry->OnDialogueOptionClicked.AddDynamic(this, &UDunGenDialogueOverlay::HandleDialogueOptionClicked);

        DialogueOptionsBox->AddChild(OptionEntry);

        UE_LOG(LogTemp, Warning, TEXT("Added dialogue option widget at index %d"), i);
    }
}

void UDunGenDialogueOverlay::HideDialogueOptions()
{
    if (!DialogueOptionsBox)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueOptionsBox is null in HideDialogueOptions"));
        return;
    }

    DialogueOptionsBox->ClearChildren();
    DialogueOptionsBox->SetVisibility(ESlateVisibility::Collapsed);
}

void UDunGenDialogueOverlay::HandleDialogueOptionClicked(int32 OptionIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("Dialogue option clicked in overlay: %d"), OptionIndex);

    APawn* OwningPawn = GetOwningPlayerPawn();
    if (!OwningPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetOwningPlayerPawn returned null in HandleDialogueOptionClicked"));
        return;
    }

    AAirsto* Airsto = Cast<AAirsto>(OwningPawn);
    if (!Airsto)
    {
        UE_LOG(LogTemp, Warning, TEXT("Owning pawn is not AAirsto in HandleDialogueOptionClicked"));
        return;
    }

    Airsto->SelectDialogueOption(OptionIndex);
}