// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DunGenDialogueOverlay.generated.h"

struct FDialogueOption;
class UTextBlock;
class UVerticalBox;
class UUserWidget;
class UDialogueOptionEntry;

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API UDunGenDialogueOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetName(FString DialogueName);
	void SetDialogueText(const FText& InText);
	void ShowDialogueOptions(const TArray<FDialogueOption>& Options);
	void SelectDialogueOption(int32 OptionIndex);
	void HideDialogueOptions();

protected:

    UFUNCTION()
    void HandleDialogueOptionClicked(int32 OptionIndex);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> DialogueOptionsBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Dialogue)
	TSubclassOf<UDialogueOptionEntry> DialogueOptionButtonClass;

private:
	
};
