#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MYDialogueTypes.generated.h"

USTRUCT(BlueprintType)
struct FQuestAdjectiveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemAdjective;

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