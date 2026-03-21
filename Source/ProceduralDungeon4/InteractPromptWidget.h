// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPromptWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API UInteractPromptWidget : public UUserWidget
{
	GENERATED_BODY()
public:

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PromptText;

private:

public:

	UFUNCTION(BlueprintCallable)
	void SetPromptText(const FText& NewText);

	
};
