// Fill out your copyright notice in the Description page of Project Settings.


#include "Saienicus.h"
#include "Components/SphereComponent.h"
#include "Airsto.h"
#include "DunGenHUD.h"
#include "Kismet/GameplayStatics.h"
#include "MyDialogueTypes.h"
#include "Engine/DataTable.h"
#include "Components/WidgetComponent.h"
#include "InteractPromptWidget.h"


ASaienicus::ASaienicus()
{
    InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractSphere->SetupAttachment((GetRootComponent()));

    InteractPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractPromptWidget"));
    InteractPromptWidget->SetupAttachment(RootComponent);
    InteractPromptWidget->SetWidgetSpace(EWidgetSpace::Screen); // usually easiest for readability
    InteractPromptWidget->SetDrawAtDesiredSize(true);
    InteractPromptWidget->SetVisibility(false);
    InteractPromptWidget->SetHiddenInGame(true);
}

void ASaienicus::BeginPlay()
{
	Super::BeginPlay();

	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &ASaienicus::OnSphereOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &ASaienicus::OnSphereEndOverlap);
	
}

void ASaienicus::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AAirsto* Airsto = Cast<AAirsto>(OtherActor);
    if (Airsto)
    {
        CurrentPlayer = Airsto;
        Airsto->SetDialogueTarget(this);
        //Airsto->ShowInteractPrompt(FText::FromString(TEXT("[E] Equip")));

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

void ASaienicus::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

void ASaienicus::Speak()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::Speak - CurrentPlayer is null."));
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

void ASaienicus::StartDialogue()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::StartDialogue - CurrentPlayer is null."));
        return;
    }

    BuildDialogueLines();

    if (ActiveDialogueNodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::StartDialogue - No dialogue lines were built."));
        return;
    }

    bInDialogue = true;
    CurrentDialogueNodeIndex = 0;

    CurrentPlayer->EnterDialogueInputMode();

    ShowCurrentDialogueNode();
    
}

void ASaienicus::AdvanceDialogue()
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
}

void ASaienicus::SelectDialogueOption(int32 OptionIndex)
{
    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::SelectDialogueOption - Invalid current node index."));
        return;
    }

    const FDialogueNode& CurrentNode = ActiveDialogueNodes[CurrentDialogueNodeIndex];

    if (!CurrentNode.Options.IsValidIndex(OptionIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::SelectDialogueOption - Invalid option index: %d"), OptionIndex);
        return;
    }

    const FDialogueOption& SelectedOption = CurrentNode.Options[OptionIndex];
    HandleDialogueOption(SelectedOption);
}

void ASaienicus::HandleDialogueOption(const FDialogueOption& SelectedOption)
{
    if (IsTrainingAction(SelectedOption.Action))
    {
        PendingTrainingAction = SelectedOption.Action;
        PendingTrainingCost = GetTrainingCost(PendingTrainingAction);

        ActiveDialogueNodes.Empty();
        BuildTrainingConfirmationNode();

        CurrentDialogueNodeIndex = 0;
        ShowCurrentDialogueNode();
        return;
    }

    switch (SelectedOption.Action)
    {
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
                UE_LOG(LogTemp, Warning,
                    TEXT("Saienicus::HandleDialogueOption - GoToNextNode had invalid NextNodeIndex: %d"),
                    SelectedOption.NextNodeIndex);

                EndDialogue();
            }

            return;
        }

        case EDialogueOptionAction::ConfirmTrainingPurchase:
        {
            if (TryPurchaseTraining())
            {
                ActiveDialogueNodes.Empty();
                BuildTrainingResultNode(FText::FromString(TEXT("Alright, your training is complete. Want to train in something else?")));
            }
            else
            {
                ActiveDialogueNodes.Empty();
                BuildTrainingResultNode(FText::FromString(TEXT("Sorry, kid but you don't have enough Wisdom for that training.")));
            }

            CurrentDialogueNodeIndex = 0;
            ShowCurrentDialogueNode();

            if (TalkingAnims.Num() > 0)
            {
                int Selection = FMath::RandRange(0, TalkingAnims.Num() - 1);
                GetMesh()->PlayAnimation(TalkingAnims[Selection], false);
            }

            return;
        }

        case EDialogueOptionAction::DeclineTrainingPurchase:
        case EDialogueOptionAction::ReturnToTrainingMenu:
        {
            ActiveDialogueNodes.Empty();
            BuildDialogueLines();

            CurrentDialogueNodeIndex = ActiveDialogueNodes.Num() - 1;
            ShowCurrentDialogueNode();
            return;
        }

        case EDialogueOptionAction::None:
        {
            if (ActiveDialogueNodes.IsValidIndex(SelectedOption.NextNodeIndex))
            {
                CurrentDialogueNodeIndex = SelectedOption.NextNodeIndex;
                ShowCurrentDialogueNode();
            }
            else
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("Saienicus::HandleDialogueOption - None action had invalid NextNodeIndex: %d"),
                    SelectedOption.NextNodeIndex);

                EndDialogue();
            }

            return;
        }

        case EDialogueOptionAction::SpawnNewDungeon:
        case EDialogueOptionAction::GiveQuest:
        case EDialogueOptionAction::TurnInQuest:
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Saienicus::HandleDialogueOption - Unsupported action for Saienicus: %d"),
                static_cast<uint8>(SelectedOption.Action));

            EndDialogue();
            return;
        }

        default:
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Saienicus::HandleDialogueOption - Unhandled action: %d"),
                static_cast<uint8>(SelectedOption.Action));

            EndDialogue();
            return;
        }
    }
}

void ASaienicus::EndDialogue()
{
    bInDialogue = false;
    CurrentDialogueNodeIndex = INDEX_NONE;
    ActiveDialogueNodes.Empty();
    PendingTrainingAction = EDialogueOptionAction::None;
    PendingTrainingCost = 0;

    if (!CurrentPlayer) return;

    CurrentPlayer->ExitDialogueInputMode();
    CurrentPlayer->HideDialogueOptions();
    //CurrentPlayer->SetDialogueTarget(nullptr);

    APlayerController* PlayerController = Cast<APlayerController>(CurrentPlayer->GetController());
    if (!PlayerController) return;

    ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
    if (!DunGenHUD) return;

    DunGenHUD->HideDialogueOverlay();

    if (GreetingsAnim)
    {
        GetMesh()->PlayAnimation(GreetingsAnim, false);
    }
}

void ASaienicus::BuildDialogueLines()
{

    ActiveDialogueNodes.Empty();

    FString PlayerName = GetCurrentPlayerName();

    if (!bFirstMeeting)
    {
        FDialogueNode GreetingsNode;
        GreetingsNode.Line = FText::FromString(
            FString::Printf(
                TEXT("Hail, %s."),
                *PlayerName
            )
        );
        ActiveDialogueNodes.Add(GreetingsNode);

    }

    if (bFirstMeeting)
    {
		FDialogueNode GreetingNode;
		GreetingNode.Line = FText::FromString(
			FString::Printf(
				TEXT("Ah, %s. I see you've finally sought me out."),
				*PlayerName
			)
		);
		ActiveDialogueNodes.Add(GreetingNode);

        if (GreetingsAnim)
        {
            GetMesh()->PlayAnimation(GreetingsAnim, false);
        }

		bFirstMeeting = false;
    }

	FDialogueNode OfferTrainingNode;
	OfferTrainingNode.Line = FText::FromString(
		TEXT("Care to trade any of the Wisdom you've gained in exchange for my training?")
	);

	const int32 TrainingMenuNodeIndex = ActiveDialogueNodes.Num() + 1;

	OfferTrainingNode.Options.Add({
		FText::FromString(TEXT("Yes.")),
		EDialogueOptionAction::GoToNextNode,
		TrainingMenuNodeIndex
	});

	OfferTrainingNode.Options.Add({
		FText::FromString(TEXT("Not right now.")),
		EDialogueOptionAction::EndDialogue,
		INDEX_NONE
	});

	ActiveDialogueNodes.Add(OfferTrainingNode);

	BuildTrainingMenuNode();
}

void ASaienicus::BuildTrainingMenuNode()
{
    if (TrainingMenuAnim)
    {
        GetMesh()->PlayAnimation(TrainingMenuAnim, false);
    }

	FDialogueNode TrainingNode;
	TrainingNode.Line = FText::FromString(
		TEXT("Choose wisely.")
	);

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Maximum Health")),
		EDialogueOptionAction::TrainMaxHealth,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Maximum Stamina")),
		EDialogueOptionAction::TrainMaxStamina,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Stamina Regeneration")),
		EDialogueOptionAction::TrainStaminaRegen,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Run Speed")),
		EDialogueOptionAction::TrainRunSpeed,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Attack Damage")),
		EDialogueOptionAction::TrainAttackDamage,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Dodge Speed")),
		EDialogueOptionAction::TrainDodgeSpeed,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Dodge Cost")),
		EDialogueOptionAction::TrainDodgeCost,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Train Bartering")),
		EDialogueOptionAction::TrainBartering,
		INDEX_NONE
	});

	TrainingNode.Options.Add({
		FText::FromString(TEXT("Nevermind.")),
		EDialogueOptionAction::EndDialogue,
		INDEX_NONE
	});

	ActiveDialogueNodes.Add(TrainingNode);
}

void ASaienicus::ShowCurrentDialogueNode()
{
    if (!CurrentPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::ShowCurrentDialogueNode - CurrentPlayer is null"));
        return;
    }

    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Saienicus::ShowCurrentDialogueNode - Invalid dialogue node index: %d"), CurrentDialogueNodeIndex);
        return;
    }

    const FDialogueNode& CurrentNode = ActiveDialogueNodes[CurrentDialogueNodeIndex];

    CurrentPlayer->ShowDialogue
    (
        FText::FromString(TEXT("Saienicus")),
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

bool ASaienicus::CurrentNodeHasOptions() const
{
    if (!ActiveDialogueNodes.IsValidIndex(CurrentDialogueNodeIndex))
    {
        return false;
    }

    return ActiveDialogueNodes[CurrentDialogueNodeIndex].Options.Num() > 0;
}

FString ASaienicus::GetCurrentPlayerName() const
{
    
    return TEXT("Airsto");
}

bool ASaienicus::IsTrainingAction(EDialogueOptionAction Action) const
{
	switch (Action)
	{
		case EDialogueOptionAction::TrainMaxHealth:
		case EDialogueOptionAction::TrainMaxStamina:
		case EDialogueOptionAction::TrainStaminaRegen:
		case EDialogueOptionAction::TrainRunSpeed:
		case EDialogueOptionAction::TrainAttackDamage:
		case EDialogueOptionAction::TrainDodgeSpeed:
		case EDialogueOptionAction::TrainDodgeCost:
		case EDialogueOptionAction::TrainBartering:
			return true;

		default:
			return false;
	}
}

int32 ASaienicus::GetTrainingCost(EDialogueOptionAction TrainingAction) const
{
	switch (TrainingAction)
	{
		case EDialogueOptionAction::TrainMaxHealth:      return 10;
		case EDialogueOptionAction::TrainMaxStamina:     return 10;
		case EDialogueOptionAction::TrainStaminaRegen:   return 10;
		case EDialogueOptionAction::TrainRunSpeed:       return 10;
		case EDialogueOptionAction::TrainAttackDamage:   return 10;
		case EDialogueOptionAction::TrainDodgeSpeed:     return 10;
		case EDialogueOptionAction::TrainDodgeCost:      return 10;
		case EDialogueOptionAction::TrainBartering:      return 10;
		default:                                         return 0;
	}
}

FText ASaienicus::GetTrainingDisplayName(EDialogueOptionAction TrainingAction) const
{
	switch (TrainingAction)
	{
		case EDialogueOptionAction::TrainMaxHealth:
			return FText::FromString(TEXT("Maximum Health"));

		case EDialogueOptionAction::TrainMaxStamina:
			return FText::FromString(TEXT("Maximum Stamina"));

		case EDialogueOptionAction::TrainStaminaRegen:
			return FText::FromString(TEXT("Stamina Regeneration"));

		case EDialogueOptionAction::TrainRunSpeed:
			return FText::FromString(TEXT("Run Speed"));

		case EDialogueOptionAction::TrainAttackDamage:
			return FText::FromString(TEXT("Attack Damage"));

		case EDialogueOptionAction::TrainDodgeSpeed:
			return FText::FromString(TEXT("Dodge Speed"));

		case EDialogueOptionAction::TrainDodgeCost:
			return FText::FromString(TEXT("Dodge Cost"));

		case EDialogueOptionAction::TrainBartering:
			return FText::FromString(TEXT("Bartering"));

		default:
			return FText::FromString(TEXT("Training"));
	}
}

void ASaienicus::BuildTrainingConfirmationNode()
{
	FDialogueNode ConfirmNode;

	const FString TrainingName = GetTrainingDisplayName(PendingTrainingAction).ToString();

	ConfirmNode.Line = FText::FromString(
		FString::Printf(
			TEXT("Training to improve your %s will cost %d Wisdom. Sound good?"),
			*TrainingName,
			PendingTrainingCost
		)
	);

	ConfirmNode.Options.Add({
		FText::FromString(TEXT("Let's do it.")),
		EDialogueOptionAction::ConfirmTrainingPurchase,
		INDEX_NONE
	});

	ConfirmNode.Options.Add({
		FText::FromString(TEXT("No thanks.")),
		EDialogueOptionAction::DeclineTrainingPurchase,
		INDEX_NONE
	});

	ActiveDialogueNodes.Add(ConfirmNode);

    if (TalkingAnims.Num() > 0)
    {
        int Selection = FMath::RandRange(0, TalkingAnims.Num() - 1);
        GetMesh()->PlayAnimation(TalkingAnims[Selection], false);
    }
}

void ASaienicus::BuildTrainingResultNode(const FText& ResultText)
{
	FDialogueNode ResultNode;
	ResultNode.Line = ResultText;

	ResultNode.Options.Add({
		FText::FromString(TEXT("Train something else?")),
		EDialogueOptionAction::ReturnToTrainingMenu,
		INDEX_NONE
	});

	ResultNode.Options.Add({
		FText::FromString(TEXT("I'm good for now.")),
		EDialogueOptionAction::EndDialogue,
		INDEX_NONE
	});

	ActiveDialogueNodes.Add(ResultNode);

    if (TalkingAnims.Num() > 0)
    {
        int Selection = FMath::RandRange(0, TalkingAnims.Num() - 1);
        GetMesh()->PlayAnimation(TalkingAnims[Selection], false);
    }
}

bool ASaienicus::TryPurchaseTraining()
{
    if (!CurrentPlayer) return false;

    const int32 CurrentWisdom = CurrentPlayer->GetWisdomAmount();
    if (CurrentWisdom < PendingTrainingCost)
    {
        return false;
    }

    switch (PendingTrainingAction)
    {
        case EDialogueOptionAction::TrainMaxHealth:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            int32 CurrentMax = CurrentPlayer->GetMaxHealth();
            int32 NewMax = CurrentMax + FMath::RoundToInt(CurrentMax * 0.25f);
            CurrentPlayer->SetMaxHealth(NewMax);
            return true;
        }

        case EDialogueOptionAction::TrainMaxStamina:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            int32 CurrentMax = CurrentPlayer->GetMaxStamina();
            int32 NewMax = CurrentMax + FMath::RoundToInt(CurrentMax * 0.25f);
            CurrentPlayer->SetMaxStamina(NewMax);
            return true;
        }

        case EDialogueOptionAction::TrainStaminaRegen:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            float CurrentRate = CurrentPlayer->GetStaminaRegenRate();
            float NewRate = CurrentRate + CurrentRate * 0.25f;
            CurrentPlayer->SetStaminaRegenRate(NewRate);
            return true;
        }

        case EDialogueOptionAction::TrainRunSpeed:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            float CurrentSpeed = CurrentPlayer->GetBaseWalkSpeed();
            float NewSpeed = CurrentSpeed + CurrentSpeed * 0.25f;
            CurrentPlayer->SetBaseWalkSpeed(NewSpeed);
            return true;
        }

        case EDialogueOptionAction::TrainAttackDamage:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            float CurrentDamage = CurrentPlayer->GetWeaponDamage();
            float NewDamage = CurrentDamage + CurrentDamage * 0.25f;
            CurrentPlayer->SetWeaponDamage(NewDamage);
            return true;
        }

        case EDialogueOptionAction::TrainDodgeSpeed:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            float CurrentSpeed = CurrentPlayer->GetBaseDodgeSpeed();
            float NewSpeed = CurrentSpeed + CurrentSpeed * 0.25f;
            CurrentPlayer->SetBaseDodgeSpeed(NewSpeed);
            return true;
        }

        case EDialogueOptionAction::TrainDodgeCost:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            float CurrentCost = CurrentPlayer->GetDodgeCost();
            float NewCost = CurrentCost - CurrentCost * 0.25f;
            CurrentPlayer->SetDodgeCost(NewCost);
            return true;
        }

        case EDialogueOptionAction::TrainBartering:
        {
            CurrentPlayer->AddToWisdomAmount(-PendingTrainingCost);

            float CurrentRate = CurrentPlayer->GetWisdomMultiplier();
            float NewRate = CurrentRate + CurrentRate * 0.25f;
            CurrentPlayer->SetWisdomMultiplier(NewRate);
            return true;
        }

        default:
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Saienicus::TryPurchaseTraining - Invalid pending training action: %d"),
                static_cast<uint8>(PendingTrainingAction));
            return false;
        }
    }
}