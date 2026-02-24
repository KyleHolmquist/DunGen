// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DunGenOverlay.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALDUNGEON4_API UDunGenOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthBarPercent(float Percent);
	void SetStaminaBarPercent(float Percent);
	void SetGold(int32 Gold);
	void SetWisdom(int32 Wisdom);
	void ShowInteractButton();
	void HideInteractButton();

private:

	UPROPERTY(meta= (BindWidget))
	class UProgressBar* HealthProgressBar;
	
	UPROPERTY(meta= (BindWidget))
	UProgressBar* StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WisdomText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractButtonText;

	UPROPERTY(meta = (BindWidget))
	class UBorder* InteractBorder;
	
};
