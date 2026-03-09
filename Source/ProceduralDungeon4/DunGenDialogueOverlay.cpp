// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenDialogueOverlay.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UDunGenDialogueOverlay::SetName(FString DialogueName)
{
    if (NameText)
    {
        NameText->SetText(FText::FromString(DialogueName));
    }
}

void UDunGenDialogueOverlay::SetDialogueText(FString Dialogue)
{
    if (DialogueText)
    {
        DialogueText->SetText(FText::FromString(Dialogue));
    }
}