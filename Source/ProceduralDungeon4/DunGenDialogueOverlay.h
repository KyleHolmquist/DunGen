// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DunGenDialogueOverlay.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API UDunGenDialogueOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetName(FString DialogueName);
	void SetDialogueText(FString Dialogue);

private:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueText;
	
};
