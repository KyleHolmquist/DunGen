#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MYDialogueTypes.generated.h"

UENUM(BlueprintType)
enum class EDialogueOptionAction : uint8
{
	None,
	GoToNextNode,
	EndDialogue,
	SpawnNewDungeon,
	GiveQuest,
	TurnInQuest,

	TrainMaxHealth,
	TrainMaxStamina,
	TrainStaminaRegen,
	TrainRunSpeed,
	TrainAttackDamage,
	TrainDodgeSpeed,
	TrainDodgeCost,
	TrainBartering,

	ConfirmTrainingPurchase,
	DeclineTrainingPurchase,
	ReturnToTrainingMenu
};

USTRUCT(BlueprintType)
struct FDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OptionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDialogueOptionAction Action = EDialogueOptionAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 NextNodeIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDialogueOption> Options;
};

USTRUCT(BlueprintType)
struct FQuestAdjectiveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PlaceAdjective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString GreetingsWord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString MustWord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString TravelWord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CollectWord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AsManyWord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemAdjectiveA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemAdjectiveB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PossibleWord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FirstMeetingClause;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PredecessorWord;
};