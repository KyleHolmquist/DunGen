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

UCLASS()
class PROCEDURALDUNGEON4_API ADomeara : public ABaseCharacter, public IDialogueInterface
{
	GENERATED_BODY()

public:

	ADomeara();

	virtual void Speak();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* InteractSphere;

	UPROPERTY()
	AAirsto* CurrentPlayer;

	UPROPERTY(VisibleAnywhere, Category=Dialogue)
	bool bInDialogue = false;

	UPROPERTY(VisibleAnywhere, Category=Dialogue)
	int32 CurrentDialogueIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category=Dialogue)
	TArray<FText> ActiveDialogueLines;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartDialogue();
	void AdvanceDialogue();
	void EndDialogue();
	void BuildDialogueLines();
	void ShowCurrentDialogueLine();

	// ---- Quest Text ----
	UPROPERTY(EditDefaultsOnly, Category="Quest Text")
	UDataTable* QuestAdjectivesTable;

	UPROPERTY(VisibleAnywhere)
	ADungeonManager* DungeonManager;

	bool GetRandomAdjectiveValue(const UDataTable* Table, FString FQuestAdjectiveRow::* Field, FString& OutValue);
	FText GenerateFirstMeetingText(const UDataTable* Table, FString& PlayerName);
	FText GeneratePredecessorWisdomLine(const UDataTable* Table);
	FText GenerateGreetingsText(const UDataTable* AdjectiveTable, FString& PlayerName);
	FText GenerateQuestText(const UDataTable* AdjectiveTable, FString& PlayerName, FString& SelectedThemeName, FString& SelectedTreasureName);

	//Dialogue Bools
	bool bFirstMeeting = true;
	bool bHasGivenQuest = false;

	//Animations
	UPROPERTY(EditAnywhere, Category=Animation)
	UAnimationAsset* GreetingsAnim;
	
	UPROPERTY(EditAnywhere, Category=Animation)
	TArray<UAnimationAsset*> TalkingAnims;
	
};
