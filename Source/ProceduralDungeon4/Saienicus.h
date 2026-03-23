// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "DialogueInterface.h"
#include "MyDialogueTypes.h"
#include "Saienicus.generated.h"

class AAirsto;
class USphereComponent;
class UDataTable;
class ADungeonManager;
struct FQuestAdjectiveRow;
class UAnimationAsset;
class UWidgetComponent;
class DungeonManager;

UCLASS()
class PROCEDURALDUNGEON4_API ASaienicus : public ABaseCharacter, public IDialogueInterface
{
	GENERATED_BODY()

public:

	ASaienicus();

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

	UPROPERTY()
	ADungeonManager* DungeonManager = nullptr;

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
	void BuildTrainingMenuNode();
	void BuildTrainingConfirmationNode();
	void BuildTrainingResultNode(const FText& ResultText);
	int32 GetTrainingCost(EDialogueOptionAction TrainingAction) const;
	bool IsTrainingAction(EDialogueOptionAction Action) const;
	bool TryPurchaseTraining();
	FText GetTrainingDisplayName(EDialogueOptionAction TrainingAction) const;

	void ShowCurrentDialogueNode();

	FString GetCurrentPlayerName() const;

	//Dialogue bools
	bool bFirstMeeting = true;

	EDialogueOptionAction PendingTrainingAction = EDialogueOptionAction::None;

	int32 PendingTrainingCost = 0;

	//Animations
	UPROPERTY(EditAnywhere, Category=Animation)
	UAnimationAsset* GreetingsAnim;
	UPROPERTY(EditAnywhere, Category=Animation)
	UAnimationAsset* TrainingMenuAnim;
	
	UPROPERTY(EditAnywhere, Category=Animation)
	TArray<UAnimationAsset*> TalkingAnims;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction)
	TObjectPtr<class UWidgetComponent> InteractPromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
	FText InteractionPrompt = FText::FromString(TEXT("[F] Speak"));

	void FacePlayer();
	
};
