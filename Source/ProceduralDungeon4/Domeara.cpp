// Fill out your copyright notice in the Description page of Project Settings.


#include "Domeara.h"
#include "Components/SphereComponent.h"
#include "Airsto.h"
#include "DunGenHUD.h"
#include "DungeonManager.h"
#include "Kismet/GameplayStatics.h"
#include "MyDialogueTypes.h"
#include "Engine/DataTable.h"


ADomeara::ADomeara()
{
    InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractSphere->SetupAttachment((GetRootComponent()));
}

void ADomeara::BeginPlay()
{
	Super::BeginPlay();

	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &ADomeara::OnSphereOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &ADomeara::OnSphereEndOverlap);

    DungeonManager = Cast<ADungeonManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonManager::StaticClass()));
	
}

void ADomeara::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IDialogueInterface* DialogueInterface = Cast<IDialogueInterface>(OtherActor);
	if (DialogueInterface)
	{
		
	}

    AAirsto* Airsto = Cast<AAirsto>(OtherActor);
    if (Airsto)
    {
        CurrentPlayer = Airsto;
        Airsto->SetDialogueTarget(this);
        Airsto->ShowInteractButton();
    }
}

void ADomeara::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

    AAirsto* Airsto = Cast<AAirsto>(OtherActor);
    if (Airsto)
    {
        EndDialogue();
        CurrentPlayer = nullptr;
        Airsto->SetDialogueTarget(nullptr);
        Airsto->HideInteractButton();
    }

}

void ADomeara::Speak()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::Speak - CurrentPlayer is null."));
        return;
    }

    if (!bInDialogue)
    {
        StartDialogue();
    }
    else
    {
        AdvanceDialogue();
    }


}

void ADomeara::StartDialogue()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::StartDialogue - CurrentPlayer is null."));
        return;
    }

    BuildDialogueLines();

    if (ActiveDialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::StartDialogue - No dialogue lines were built."));
        return;
    }

    bInDialogue = true;
    CurrentDialogueIndex = 0;
    ShowCurrentDialogueLine();
}

void ADomeara::AdvanceDialogue()
{
    if (!bInDialogue) return;

    ++CurrentDialogueIndex;

    if (!ActiveDialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        EndDialogue();
        return;
    }

    ShowCurrentDialogueLine();
}

void ADomeara::EndDialogue()
{
    bInDialogue = false;
    CurrentDialogueIndex = INDEX_NONE;
    ActiveDialogueLines.Empty();

    if (!CurrentPlayer) return;

    APlayerController* PlayerController = Cast<APlayerController>(CurrentPlayer->GetController());
    if (!PlayerController) return;

    ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
    if (!DunGenHUD) return;

    DunGenHUD->HideDialogueOverlay();
}

void ADomeara::BuildDialogueLines()
{
    ActiveDialogueLines.Empty();

    const FString PlayerName = TEXT("Airsto");
    const FString SelectedThemeText = DungeonManager->GetSelectedThemeText();
    const FString SelectedTreasureText = DungeonManager->GetSelectedTreasureText();

    if (!DungeonManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::BuildDialogueLines - DungeonManager is null."));
        return;
    }

    if (!DungeonManager->HasPlayerEnteredPortal() && bHasGivenQuest)
    {
        ActiveDialogueLines.Add
        (
            FText::Format
            (
                FText::FromString("You must first travel through the portal to the {0} to retrieve the {1}."),
                FText::FromString(SelectedThemeText),
                FText::FromString(SelectedTreasureText)
            )
        ); 
        return;
    }

    const FText QuestLine = GenerateQuestText
    (
        QuestAdjectivesTable,
        PlayerName,
        SelectedThemeText,
        SelectedTreasureText
    );

    ActiveDialogueLines.Add(FText::FromString(TEXT("Greetings, Airsto.")));
    ActiveDialogueLines.Add(QuestLine);
    bHasGivenQuest = true;
}

void ADomeara::ShowCurrentDialogueLine()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::ShowCurrentDialogueLine - CurrentPlayer is null."));
        return;
    }

    if (!ActiveDialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::ShowCurrentDialogueLine - Invalid dialogue index: %d"), CurrentDialogueIndex);
        return;
    }

    CurrentPlayer->ShowDialogue
    (
        FText::FromString(TEXT("Domeara")),
        ActiveDialogueLines[CurrentDialogueIndex]
    );
}

bool ADomeara::GetRandomAdjectiveValue(const UDataTable* Table, FString FQuestAdjectiveRow::* Field, FString& OutValue)
{
	if (!Table) return false;

	static const FString Context(TEXT("QuestAdjectives"));
	TArray<FQuestAdjectiveRow*> Rows;
	Table->GetAllRows(Context, Rows);

	if (Rows.Num() == 0) return false;

	//Only collect non-empty values to account for varying column fills
	TArray<FString> ValidValues;

	for (FQuestAdjectiveRow* Row : Rows)
	{
		if (!Row) continue;

		const FString& Value = Row->*Field;
		if (!Value.IsEmpty())
		{
			ValidValues.Add(Value);
		}
	}

	if (ValidValues.Num() == 0) return false;

	const int32 Index = FMath::RandRange(0, ValidValues.Num() - 1);
	OutValue = ValidValues[Index];

	return true;
}

FText ADomeara::GenerateQuestText(const UDataTable* AdjectiveTable, const FString& PlayerName,const FString& SelectedThemeName, const FString& SelectedTreasureName)
{
	FString GreetingsWord, MustWord, TravelWord, PlaceAdj, CollectWord, AsManyWord, ItemAdjA, ItemAdjB, PossibleWord;

	//Pull from different random rows
	GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::GreetingsWord, GreetingsWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::MustWord, MustWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::TravelWord, TravelWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::PlaceAdjective, PlaceAdj);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::CollectWord, CollectWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::AsManyWord, AsManyWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::ItemAdjectiveA, ItemAdjA);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::ItemAdjectiveB, ItemAdjB);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::PossibleWord, PossibleWord);

	FString Template = 
		TEXT("{GreetingsWord}, {PlayerName}! {MustWord} {TravelWord} the {ThemeName} of {PlaceAdj} to {CollectWord} {AsManyWord} {ItemAdjA} {TreasureName}s of {ItemAdjB} {PossibleWord}.");

		Template = Template.Replace(TEXT("{GreetingsWord}"), *GreetingsWord);
		Template = Template.Replace(TEXT("{PlayerName}"), *PlayerName);
		Template = Template.Replace(TEXT("{MustWord}"), *MustWord);
		Template = Template.Replace(TEXT("{TravelWord}"), *TravelWord);
		Template = Template.Replace(TEXT("{ThemeName}"), *SelectedThemeName);
		Template = Template.Replace(TEXT("{PlaceAdj}"), *PlaceAdj);
		Template = Template.Replace(TEXT("{CollectWord}"), *CollectWord);
		Template = Template.Replace(TEXT("{AsManyWord}"), *AsManyWord);
		Template = Template.Replace(TEXT("{ItemAdjA}"), *ItemAdjA);
		Template = Template.Replace(TEXT("{TreasureName}"), *SelectedTreasureName);
		Template = Template.Replace(TEXT("{ItemAdjB}"), *ItemAdjB);
		Template = Template.Replace(TEXT("{PossibleWord}"), *PossibleWord);

		return FText::FromString(Template);
}