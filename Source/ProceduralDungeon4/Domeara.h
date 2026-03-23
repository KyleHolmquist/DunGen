// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "DialogueInterface.h"
#include "Domeara.generated.h"

class AAirsto;
class USphereComponent;
class UDataTable;
class ADungeonManager;
struct FQuestAdjectiveRow;
class UAnimationAsset;
struct FDialogueNode;
struct FDialogueOption;
class APortal;
class UWidgetComponent;

UCLASS()
class PROCEDURALDUNGEON4_API ADomeara : public ABaseCharacter, public IDialogueInterface
{
	GENERATED_BODY()

public:

	ADomeara();

	virtual void Speak();

	virtual void SelectDialogueOption(int32 OptionIndex) override;
	void HandleDialogueOption(const FDialogueOption& SelectedOption);
	bool CurrentNodeHasOptions() const;

    UFUNCTION(BlueprintCallable, Category = Dialogue)
    void ResetFirstMeetingState();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* InteractSphere;

	UPROPERTY()
	AAirsto* CurrentPlayer;

	UPROPERTY(VisibleAnywhere, Category=Dialogue)
	bool bInDialogue = false;

	UPROPERTY(VisibleAnywhere, Category=Dialogue)
	TArray<FDialogueNode> ActiveDialogueNodes;

	UPROPERTY(VisibleAnywhere, Category=Dialogue)
	int32 CurrentDialogueNodeIndex = INDEX_NONE;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartDialogue();
	void AdvanceDialogue();
	void EndDialogue();
	void BuildDialogueLines();

	void ShowCurrentDialogueNode();

	void NewDungeonQuestInit();

	// ---- Quest Text ----
	UPROPERTY(EditDefaultsOnly, Category="Quest Text")
	UDataTable* QuestAdjectivesTable;

	UPROPERTY(VisibleAnywhere)
	ADungeonManager* DungeonManager;

	bool GetRandomAdjectiveValue(const UDataTable* Table, FString FQuestAdjectiveRow::* Field, FString& OutValue);
	FString GetCurrentPlayerName() const;
	FText GenerateFirstMeetingText(const UDataTable* Table, const FString& PlayerName);
	FText GeneratePredecessorWisdomLine(const UDataTable* Table);
	FText GenerateGreetingsText(const UDataTable* AdjectiveTable, const FString& PlayerName);
	FText GenerateQuestText(const UDataTable* AdjectiveTable, const FString& PlayerName, const FString& SelectedThemeName, const 	FString& SelectedTreasureName);

	//Dialogue Bools
	
	bool bFirstMeeting = true;
	bool bHasGivenQuest = false;
	bool bHasTradedTreasureForWisdom = false;

	//Animations
	UPROPERTY(EditAnywhere, Category=Animation)
	UAnimationAsset* GreetingsAnim;
	
	UPROPERTY(EditAnywhere, Category=Animation)
	TArray<UAnimationAsset*> TalkingAnims;

	UPROPERTY()
	APortal* HomePortal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction)
	TObjectPtr<class UWidgetComponent> InteractPromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
	FText InteractionPrompt = FText::FromString(TEXT("[F] Speak"));

	void FacePlayer();
	
};
