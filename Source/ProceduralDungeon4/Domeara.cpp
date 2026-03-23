// Fill out your copyright notice in the Description page of Project Settings.


#include "Domeara.h"
#include "Components/SphereComponent.h"
#include "Airsto.h"
#include "DunGenHUD.h"
#include "DungeonManager.h"
#include "Kismet/GameplayStatics.h"
#include "MyDialogueTypes.h"
#include "Engine/DataTable.h"
#include "Portal.h"
#include "Components/WidgetComponent.h"
#include "InteractPromptWidget.h"
#include "DunGenOverlay.h"


ADomeara::ADomeara()
{
    InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractSphere->SetupAttachment((GetRootComponent()));

    InteractPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractPromptWidget"));
    InteractPromptWidget->SetupAttachment(RootComponent);
    InteractPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    InteractPromptWidget->SetDrawAtDesiredSize(true);
    InteractPromptWidget->SetVisibility(false);
    InteractPromptWidget->SetHiddenInGame(true);
}

void ADomeara::BeginPlay()
{
	Super::BeginPlay();

	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &ADomeara::OnSphereOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &ADomeara::OnSphereEndOverlap);

    DungeonManager = Cast<ADungeonManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonManager::StaticClass()));

    TArray<AActor*> PortalActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortal::StaticClass(), PortalActors);

    for (AActor* Actor : PortalActors)
    {
        APortal* Portal = Cast<APortal>(Actor);
        if (Portal && Portal->bIsHomePortal)
        {
            HomePortal = Portal;
            break;
        }
    }

    
	bFirstMeeting = true;
	bHasGivenQuest = false;
	bHasTradedTreasureForWisdom = false;
	
}

void ADomeara::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AAirsto* Airsto = Cast<AAirsto>(OtherActor);
    if (Airsto)
    {
        CurrentPlayer = Airsto;
        Airsto->SetDialogueTarget(this);
        //Airsto->ShowInteractPrompt(FText::FromString(TEXT("[F] Speak")));
    }

    if (Airsto && InteractPromptWidget)
    {
        InteractPromptWidget->SetVisibility(true);
        InteractPromptWidget->SetHiddenInGame(false);
    }

    if (UInteractPromptWidget* PromptWidget = Cast<UInteractPromptWidget>(InteractPromptWidget->GetUserWidgetObject()))
    {
        PromptWidget->SetPromptText(InteractionPrompt);
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
        //Airsto->HideInteractPrompt();
    }

    if (Airsto && InteractPromptWidget)
    {
        InteractPromptWidget->SetVisibility(false);
        InteractPromptWidget->SetHiddenInGame(true);
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
    FacePlayer();

    BuildDialogueLines();

    if (ActiveDialogueNodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::StartDialogue - No dialogue lines were built."));
        return;
    }

    bInDialogue = true;
    CurrentDialogueNodeIndex = 0;

    CurrentPlayer->EnterDialogueInputMode();

    ShowCurrentDialogueNode();

    if (GreetingsAnim)
    {
        GetMesh()->PlayAnimation(GreetingsAnim, false);
    }
    
}

void ADomeara::AdvanceDialogue()
{
    if (!bInDialogue) return;

    if (CurrentNodeHasOptions())
    {
        //Don't advance with interact when node requires a choice
        return;
    }

    ++CurrentDialogueNodeIndex;

    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        EndDialogue();
        return;
    }

    ShowCurrentDialogueNode();

    if (TalkingAnims.Num() > 0)
    {
        int Selection = FMath::RandRange(0, TalkingAnims.Num() - 1);
        GetMesh()->PlayAnimation(TalkingAnims[Selection], false);
    }
}

void ADomeara::SelectDialogueOption(int32 OptionIndex)
{
    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::SelectDialogueOption - Invalid current node index."));
        return;
    }

    const FDialogueNode& CurrentNode = ActiveDialogueNodes[CurrentDialogueNodeIndex];

    if (!CurrentNode.Options.IsValidIndex(OptionIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::SelectDialogueOption - Invalid option index: %d"), OptionIndex);
        return;
    }

    const FDialogueOption& SelectedOption = CurrentNode.Options[OptionIndex];
    HandleDialogueOption(SelectedOption);
}

void ADomeara::HandleDialogueOption(const FDialogueOption& SelectedOption)
{
    switch (SelectedOption.Action)
    {
        case EDialogueOptionAction::SpawnNewDungeon:
        {

            if (!DungeonManager)
            {
                UE_LOG(LogTemp, Warning, TEXT("Domeara::HandleDialogueOption - DungeonManager is null."));
                EndDialogue();
                return;
            }

            DungeonManager->SpawnNewDungeon();
            NewDungeonQuestInit();

            return;
        }

        case EDialogueOptionAction::EndDialogue:
        {
            EndDialogue();
            return;
        }

        case EDialogueOptionAction::GoToNextNode:
        {
            if (ActiveDialogueNodes.IsValidIndex(SelectedOption.NextNodeIndex))
            {
                CurrentDialogueNodeIndex = SelectedOption.NextNodeIndex;
                ShowCurrentDialogueNode();
            }
            else
            {
                EndDialogue();
            }
            return;
        }

        case EDialogueOptionAction::None:
        default:
        {
            if (ActiveDialogueNodes.IsValidIndex(SelectedOption.NextNodeIndex))
            {
                CurrentDialogueNodeIndex = SelectedOption.NextNodeIndex;
                ShowCurrentDialogueNode();
            }
            else
            {
                EndDialogue();
            }
            return;
        }
    }
}

void ADomeara::EndDialogue()
{
    bInDialogue = false;
    CurrentDialogueNodeIndex = INDEX_NONE;
    ActiveDialogueNodes.Empty();

    if (!CurrentPlayer) return;

    CurrentPlayer->ExitDialogueInputMode();
    CurrentPlayer->HideDialogueOptions();
    //CurrentPlayer->SetDialogueTarget(nullptr);

    APlayerController* PlayerController = Cast<APlayerController>(CurrentPlayer->GetController());
    if (!PlayerController) return;

    ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
    if (!DunGenHUD) return;

    DunGenHUD->HideDialogueOverlay();
}

void ADomeara::BuildDialogueLines()
{
    if (!DungeonManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::BuildDialogueLines - DungeonManager is null."));
        return;
    }

    ActiveDialogueNodes.Empty();

    FString PlayerName = GetCurrentPlayerName();
    FString SelectedThemeText = DungeonManager->GetSelectedThemeText();
    FString SelectedTreasureText = DungeonManager->GetSelectedTreasureText();

    if (bFirstMeeting)
    {
        FText FirstMeetingLine = GenerateFirstMeetingText
        (
            QuestAdjectivesTable,
            PlayerName
        );

        FText PredecessorWisdomLine = GeneratePredecessorWisdomLine
        (
            QuestAdjectivesTable
        );

        FDialogueNode FirstMeetingNode;
        FirstMeetingNode.Line = FirstMeetingLine;
        ActiveDialogueNodes.Add(FirstMeetingNode);

        FDialogueNode PredecessorWisdomNode;
        PredecessorWisdomNode.Line = PredecessorWisdomLine;
        ActiveDialogueNodes.Add(PredecessorWisdomNode);

        bFirstMeeting = false;
    }
    else if (!DungeonManager->HasPlayerEnteredPortal() && !bHasGivenQuest)
    {
        FText GreetingsLine = GenerateGreetingsText
        (
            QuestAdjectivesTable,
            PlayerName
        );

        FDialogueNode GreetingsNode;
        GreetingsNode.Line = GreetingsLine;
        ActiveDialogueNodes.Add(GreetingsNode);
    }

    if (!DungeonManager->HasPlayerEnteredPortal() && bHasGivenQuest)
    {
        FDialogueNode MustTravelNode;
        MustTravelNode.Line = FText::Format
        (
            FText::FromString("You must travel through the portal to the {0} to retrieve the {1}."),
            FText::FromString(SelectedThemeText),
            FText::FromString(SelectedTreasureText)
        );
        ActiveDialogueNodes.Add(MustTravelNode);
        return;
    }

    if (DungeonManager->HasPlayerEnteredPortal())
    {
        FDialogueNode GladYoureAliveNode;
        FString Line =
            TEXT("I'm glad to see you've returned in one piece, {PlayerName}.");
        Line = Line.Replace(TEXT("{PlayerName}"), *PlayerName);
        GladYoureAliveNode.Line = FText::FromString(Line);
        ActiveDialogueNodes.Add(GladYoureAliveNode);

        if (!bHasTradedTreasureForWisdom)
        {
            FDialogueNode ItsGivingWisdomNode;
            int TreasureAmount = CurrentPlayer->GetTreasureAmount();
            int WisdomAmount = TreasureAmount * CurrentPlayer->GetWisdomMultiplier();
            ItsGivingWisdomNode.Line = FText::Format
            (
                FText::FromString("Here's {0} Wisdom for the {1} {2}s you brought back from the {3}."),
                FText::AsNumber(WisdomAmount),
                FText::AsNumber(TreasureAmount),
                FText::FromString(SelectedTreasureText),
                FText::FromString(SelectedThemeText)
            );

            ActiveDialogueNodes.Add(ItsGivingWisdomNode);
            DungeonManager->AddToAccruedWisdom(WisdomAmount);
            DungeonManager->AddToBankedWisdom(WisdomAmount);

            if (DungeonManager)
            {
                DungeonManager->AddToBankedWisdom(WisdomAmount);
                DungeonManager->AddToAccruedWisdom(WisdomAmount);
            }

            if (CurrentPlayer)
            {
                if (ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD()))
                {
                    if (UDunGenOverlay* Overlay = DunGenHUD->GetDunGenOverlay())
                    {
                        Overlay->SetWisdom(DungeonManager ? DungeonManager->GetBankedWisdom() : 0);
                    }
                }

                CurrentPlayer->SetTreasureAmount(0);
            }

            bHasTradedTreasureForWisdom = true;
        }

        FDialogueNode ContinueNode;
        ContinueNode.Line = FText::FromString("Would you like me to open another portal?");
        ContinueNode.Options =
        {
            FDialogueOption{FText::FromString("Yes"), EDialogueOptionAction::SpawnNewDungeon, INDEX_NONE},
            FDialogueOption{FText::FromString("No"), EDialogueOptionAction::EndDialogue, INDEX_NONE}
        };
        ActiveDialogueNodes.Add(ContinueNode);

        return;
    }

    FText QuestLine = GenerateQuestText
    (
        QuestAdjectivesTable,
        PlayerName,
        SelectedThemeText,
        SelectedTreasureText
    );

    FDialogueNode QuestNode;
    QuestNode.Line = QuestLine;
    ActiveDialogueNodes.Add(QuestNode);

    FDialogueNode OpenPortalNode;
    OpenPortalNode.Line = FText::FromString("I've opened a portal that will send you there.");
    ActiveDialogueNodes.Add(OpenPortalNode);

    if (HomePortal)
    {
        HomePortal->SetPortalActive(true);
    }

    bHasGivenQuest = true;
}

void ADomeara::ShowCurrentDialogueNode()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::ShowCurrentDialogueNode - CurrentPlayer is null"));
        return;
    }

    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Domeara::ShowCurrentDialogueNode - Invalid dialogue node index: %d"), CurrentDialogueNodeIndex);
        return;
    }

    const FDialogueNode& CurrentNode = ActiveDialogueNodes[CurrentDialogueNodeIndex];

    CurrentPlayer->ShowDialogue
    (
        FText::FromString(TEXT("Domeara")),
        CurrentNode.Line
    );

    if (CurrentNode.Options.Num() > 0)
    {
        CurrentPlayer->ShowDialogueOptions(CurrentNode.Options);
    }
    else
    {
        CurrentPlayer->HideDialogueOptions();
    }
}

bool ADomeara::CurrentNodeHasOptions() const
{
    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        return false;
    }

    return ActiveDialogueNodes[CurrentDialogueNodeIndex].Options.Num() > 0;
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

FString ADomeara::GetCurrentPlayerName() const
{
    if (CurrentPlayer && CurrentPlayer->HasValidPlayerName())
    {
        return CurrentPlayer->GetPlayerName();
    }

    return TEXT("Airsto");
}

FText ADomeara::GenerateFirstMeetingText(const UDataTable* AdjectiveTable, const FString& PlayerName)
{
    FString FirstMeetingClause;

    //Pull from FirstMeeting Row
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::FirstMeetingClause, FirstMeetingClause);

    FString Template = 
        TEXT("{FirstMeetingClause}, {PlayerName}.");

        Template = Template.Replace(TEXT("{FirstMeetingClause}"), *FirstMeetingClause);
        Template = Template.Replace(TEXT("{PlayerName}"), *PlayerName);

        return FText::FromString(Template);

}

FText ADomeara::GeneratePredecessorWisdomLine(const UDataTable* AdjectiveTable)
{
    FString PredecessorWord;

    //Pull from Predecessor Row
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::PredecessorWord, PredecessorWord);

    int32 CurrentBankedWisdom = DungeonManager->GetBankedWisdom();

    FString Template = 
        TEXT("Your {PredecessorWord} left you {CurrentBankedWisdom} Wisdom.");

        Template = Template.Replace(TEXT("{PredecessorWord}"), *PredecessorWord);
        Template = Template.Replace(TEXT("{CurrentBankedWisdom}"), *FString::FromInt(CurrentBankedWisdom));

        return FText::FromString(Template);
}


FText ADomeara::GenerateGreetingsText(const UDataTable* AdjectiveTable, const FString& PlayerName)
{
    FString GreetingsWord;

    //Pull from Greetings Row
	GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::GreetingsWord, GreetingsWord);

    FString Template =
        TEXT("{GreetingsWord}, {PlayerName}.");

        Template = Template.Replace(TEXT("{GreetingsWord}"), *GreetingsWord);
        Template = Template.Replace(TEXT("{PlayerName}"), *PlayerName);

        return FText::FromString(Template);
}

FText ADomeara::GenerateQuestText(const UDataTable* AdjectiveTable, const FString& PlayerName, const FString& SelectedThemeName, const FString& SelectedTreasureName)
{
	FString MustWord, TravelWord, PlaceAdj, CollectWord, AsManyWord, ItemAdjA, ItemAdjB, PossibleWord;

	//Pull from different random rows
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::MustWord, MustWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::TravelWord, TravelWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::PlaceAdjective, PlaceAdj);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::CollectWord, CollectWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::AsManyWord, AsManyWord);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::ItemAdjectiveA, ItemAdjA);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::ItemAdjectiveB, ItemAdjB);
    GetRandomAdjectiveValue(AdjectiveTable, &FQuestAdjectiveRow::PossibleWord, PossibleWord);

	FString Template = 
		TEXT("{MustWord} {TravelWord} the {ThemeName} of {PlaceAdj} to {CollectWord} {AsManyWord} {ItemAdjA} {TreasureName}s of {ItemAdjB} {PossibleWord}.");


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

void ADomeara::NewDungeonQuestInit()
{
    //Reset quest state flags for the new assignment
    bHasGivenQuest = false;
    bHasTradedTreasureForWisdom = false;
    DungeonManager->SetPlayerEnteredPortal(false);

    //Rebuild this conversation into the new quest assignment lines
    ActiveDialogueNodes.Empty();

    FString PlayerName = GetCurrentPlayerName();
    FString SelectedThemeText = DungeonManager->GetSelectedThemeText();
    FString SelectedTreasureText = DungeonManager->GetSelectedTreasureText();

    FText QuestLine = GenerateQuestText
    (
        QuestAdjectivesTable,
        PlayerName,
        SelectedThemeText,
        SelectedTreasureText
    );

    FDialogueNode QuestNode;
    QuestNode.Line = QuestLine;
    ActiveDialogueNodes.Add(QuestNode);

    FDialogueNode OpenPortalNode;
    OpenPortalNode.Line = FText::FromString("I've opened a portal that will send you there.");
    ActiveDialogueNodes.Add(OpenPortalNode);

    bHasGivenQuest = true;
    CurrentDialogueNodeIndex = 0;
    bInDialogue = true;

    ShowCurrentDialogueNode();
}

void ADomeara::ResetFirstMeetingState()
{
    bFirstMeeting = true;
}

void ADomeara::FacePlayer()
{
    if (!CurrentPlayer) return;

    FVector NPC_Location = GetActorLocation();
    FVector Player_Location = CurrentPlayer->GetActorLocation();

    FVector Direction = Player_Location - NPC_Location;
    Direction.Z = 0.f;

    if (Direction.IsNearlyZero()) return;

    FRotator TargetRotation = Direction.Rotation();

    SetActorRotation(FRotator(0.f, TargetRotation.Yaw, 0.f));
}