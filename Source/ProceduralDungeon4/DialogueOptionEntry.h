// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueOptionEntry.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueOptionClicked, int32, ClickedOptionIndex);

UCLASS()
class PROCEDURALDUNGEON4_API UDialogueOptionEntry : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual bool Initialize() override;

    UFUNCTION(BlueprintCallable)
    void SetupOption(const FText& InOptionText, int32 InOptionIndex);

    UPROPERTY(BlueprintAssignable, Category =Dialogue)
    FOnDialogueOptionClicked OnDialogueOptionClicked;

protected:
    UFUNCTION()
    void HandleButtonClicked();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> OptionButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> OptionText;

    UPROPERTY(BlueprintReadOnly, Category =Dialogue)
    int32 OptionIndex = INDEX_NONE;


	
};
