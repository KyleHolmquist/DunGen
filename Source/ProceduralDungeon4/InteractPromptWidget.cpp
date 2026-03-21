// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractPromptWidget.h"
#include "Components/TextBlock.h"

void UInteractPromptWidget::SetPromptText(const FText& NewText)
{
    if (PromptText)
    {
        PromptText->SetText(NewText);
    }
}