// Fill out your copyright notice in the Description page of Project Settings.


#include "Airsto.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AttributeComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "Animation/AnimMontage.h"
#include "DunGenHUD.h"
#include "DunGenOverlay.h"
#include "Wisdom.h"
#include "Treasure.h"
#include "DialogueInterface.h"
#include "DunGenDialogueOverlay.h"
#include "MyDialogueTypes.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ProceduralDungeonGameMode.h"

AAirsto::AAirsto()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

}

void AAirsto::BeginPlay()
{
	Super::BeginPlay();

	CurrentDodgeSpeed = BaseDodgeSpeed;

    //InitializeEnhancedInput();
    Tags.Add(FName("EngageableTarget"));
    InitializeDunGenOverlay();
	
}

void AAirsto::PawnClientRestart()
{
    Super::PawnClientRestart();

    InitializeEnhancedInput();
    InitializeDunGenOverlay();
}

void AAirsto::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attributes && DunGenOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		DunGenOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

	if (ActionState == EActionState::EAS_Dodging)
    {
        //Move the player for the dodge
        FVector Vel = GetCharacterMovement()->Velocity;
        Vel.X = DodgeDirection.X * CurrentDodgeSpeed;
        Vel.Y = DodgeDirection.Y * CurrentDodgeSpeed;
        GetCharacterMovement()->Velocity = Vel;

        //Rotate toward dodge direction
        const FRotator TargetRot(0.f, DodgeDirection.Rotation().Yaw, 0.f);
        const FRotator NewRot =
            FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 20.f);

        SetActorRotation(NewRot);

		CreateFields(GetActorLocation());
    	HandleDodgeImpact();
    }

	if (bApplyingAttackFields)
	{
		if (EquippedWeapon)
		{
			CreateFields(EquippedWeapon->GetActorLocation());
		}
	}
}

void AAirsto::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}


void AAirsto::AddWisdom(AWisdom* Wisdom)
{
	if (Attributes && DunGenOverlay)
	{
		Attributes->AddWisdom(Wisdom->GetWisdom());
		DunGenOverlay->SetWisdom(Attributes->GetWisdom());
	}
}

void AAirsto::AddGold(ATreasure* Treasure)
{
	if (Attributes && DunGenOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		DunGenOverlay->SetGold(Attributes->GetTreasure());
	}
}

void AAirsto::AddToGoldAmount(int Amount)
{
	if (Attributes && DunGenOverlay)
	{
		Attributes->AddGold(Amount);
		DunGenOverlay->SetGold(Attributes->GetTreasure());
	}
}

void AAirsto::AddToWisdomAmount(int Amount)
{
	if (Attributes && DunGenOverlay)
	{
		Attributes->AddWisdom(Amount);
		DunGenOverlay->SetWisdom(Attributes->GetWisdom());
	}
}

void AAirsto::InitializeEnhancedInput()
{
    if (APlayerController *PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
			Subsystem->ClearAllMappings();
            Subsystem->AddMappingContext(AirstoMappingContext, 0);

        }
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AAirsto::InitializeEnhancedInput - Input subsystem is null."));
		}

        Tags.Add(FName("Player"));
    }
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::InitializeEnhancedInput - Controller is null."));

	}
}
void AAirsto::InitializeDunGenOverlay()
{
    APlayerController *PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        ADunGenHUD *DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
        if (DunGenHUD)
        {

            DunGenOverlay = DunGenHUD->GetDunGenOverlay();
            if (DunGenOverlay && Attributes)
            {
				DunGenOverlay->HideInteractButton();
                DunGenOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
                DunGenOverlay->SetStaminaBarPercent(1.f);
                DunGenOverlay->SetGold(0);
                DunGenOverlay->SetWisdom(0);
            }
        }
    }
}

void AAirsto::Move(const FInputActionValue& Value)
{

	if (ActionState != EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	// const FVector Forward = GetActorForwardVector();
	// AddMovementInput(Forward, MovementVector.Y);

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);

	// const FVector Right = GetActorRightVector();
	// AddMovementInput(Right, MovementVector.X);

}

void AAirsto::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisVector.Y);
		AddControllerPitchInput(LookAxisVector.X);
	}
}

void AAirsto::LookX(const FInputActionValue& Value)
{
	const float LookX = Value.Get<float>();
	if (GetController())
	{
		AddControllerYawInput(LookX);
	}
}

void AAirsto::LookY(const FInputActionValue& Value)
{
	const float LookY = Value.Get<float>();
	if (GetController())
	{
		AddControllerPitchInput(LookY);
	}
}

void AAirsto::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
	
}

bool AAirsto::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

bool AAirsto::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void AAirsto::Equip(const FInputActionValue& Value)
{

	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		const bool bEquip = Value.Get<bool>();
		if (bEquip)
		{
			if (EquippedWeapon)
			{
				bHasWeapon = true;
				return;
				// EquippedWeapon->Destroy();
			}
            GetWeaponType(OverlappingWeapon);
			bHasWeapon = true;
		}
    }
	else 
	{
		if (CanDisarm())
		{
			const bool bEquip = Value.Get<bool>();
			if (bEquip)
			{
				Disarm();
			}
		}
		else if (CanArm())
		{
			const bool bEquip = Value.Get<bool>();
			if (bEquip)
			{
                Arm();
            }
		}
	}
}

void AAirsto::Unequip(const FInputActionValue& Value)
{

    if (!EquippedWeapon) return;

    //Drop direction = forward + slight upward
    FVector Forward = GetActorForwardVector();
    FVector Impulse = Forward * 300.f + FVector(0.f, 0.f, 200.f);

    EquippedWeapon->Drop(Impulse);

    EquippedWeapon = nullptr;
	bHasWeapon = false;

    CharacterState = ECharacterState::ECS_Unequipped;
    ActionState = EActionState::EAS_Unoccupied;
}

void AAirsto::GetWeaponType(AWeapon * OverlappingWeapon)
{

	if (OverlappingWeapon)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedWeapon;
	}

    OverlappingWeapon->SetOwner(this);
    OverlappingWeapon->SetInstigator(this);
    WeaponType = OverlappingWeapon->GetWeaponType();
    OverlappingItem = nullptr;
    EquippedWeapon = OverlappingWeapon;
	FinishEquipping();
}

void AAirsto::Arm()
{
    PlayEquipMontage(FName("Equip"));
    ActionState = EActionState::EAS_EquippingWeapon;
    WeaponType = EquippedWeapon->GetWeaponType();
	CharacterState = ECharacterState::ECS_EquippedWeapon;
}
 
void AAirsto::Attack(const FInputActionValue& Value)
{	
	Super::Attack(Value);
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;

		if (Attributes && DunGenOverlay)
		{
			Attributes->UseStamina(Attributes->GetAttackCost());
			DunGenOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
	}
}

void AAirsto::Dodge(const FInputActionValue& Value)
{
	if (IsOccupied() || !HasDodgeStamina()) return;

    DodgeDirection = GetLastMovementInputVector();
    DodgeDirection.Z = 0.f;

    if (DodgeDirection.IsNearlyZero())
    {
        DodgeDirection = GetActorForwardVector();
        DodgeDirection.Z = 0.f;
    }

    DodgeDirection.Normalize();

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

    PlayDodgeMontage();
    ActionState = EActionState::EAS_Dodging;

    if (Attributes && DunGenOverlay)
    {
        Attributes->UseStamina(Attributes->GetDodgeCost());
        DunGenOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
    }

}

void AAirsto::Interact(const FInputActionValue& Value)
{
	if (!DialogueTarget) return;

	IDialogueInterface* Dialogue = Cast<IDialogueInterface>(DialogueTarget);
	if (Dialogue)
	{
		Dialogue->Speak();
	}
}

void AAirsto::Pause(const FInputActionValue& Value)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController) return;

    ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
    if (!DunGenHUD) return;

    DunGenHUD->TogglePauseMenu();
}

bool AAirsto::HasDodgeStamina()
{
    return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}
bool AAirsto::HasAttackStamina()
{
    return Attributes && Attributes->GetStamina() > Attributes->GetAttackCost();
}
bool AAirsto::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped &&
		HasAttackStamina();
}

bool AAirsto::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AAirsto::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void AAirsto::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void AAirsto::AttachWeaponToSpine()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AAirsto::AttachWeaponToHand()
{

	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("WeaponSocket"));
	}
}

void AAirsto::FinishEquipping()
{
	if (ActionState != EActionState::EAS_Unoccupied)
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

void AAirsto::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AAirsto::ShowInteractButton()
{
	if (DunGenOverlay)
	{
		DunGenOverlay->ShowInteractButton();
	}
}
void AAirsto::HideInteractButton()
{
	if (DunGenOverlay)
	{
		DunGenOverlay->HideInteractButton();
	}
}

void AAirsto::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
	FinishEquipping();
}

void AAirsto::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();

	AProceduralDungeonGameMode* GameMode = 
		Cast<AProceduralDungeonGameMode>(GetWorld()->GetAuthGameMode());

	GetWorldTimerManager().SetTimer
	(
		RespawnTimerHandle,
		this,
		&AAirsto::HandleRespawn,
		RespawnDelay,
		false
	);

	if (EquippedWeapon)
	{
		SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

		//Drop direction = forward + slight upward
		FVector Forward = GetActorForwardVector();
		FVector Impulse = Forward * 300.f + FVector(0.f, 0.f, 200.f);
		EquippedWeapon->Drop(Impulse);
		EquippedWeapon = nullptr;
		CharacterState = ECharacterState::ECS_Unequipped;
	}

	DisableInput(nullptr);

	GetCharacterMovement()->DisableMovement();
	
}

void AAirsto::AttackBegin()
{
	bApplyingAttackFields = true;
}

void AAirsto::AttackEnd()
{
	bApplyingAttackFields = false;
	ActionState = EActionState::EAS_Unoccupied;
}

void AAirsto::DodgeEnd()
{
	Super::DodgeEnd();
	
	//Stop dodge motion and just keep Z
    FVector V = GetCharacterMovement()->Velocity;
    V.X = 0.f;
    V.Y = 0.f;
    GetCharacterMovement()->Velocity = V;

    //Restore friction and braking
    if (bSavedDodgeFriction)
    {
        GetCharacterMovement()->BrakingFrictionFactor = SavedBrakingFrictionFactor;
        GetCharacterMovement()->GroundFriction = SavedGroundFriction;
    }

	//Restore the rotation settings
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

    ActionState = EActionState::EAS_Unoccupied;
}

void AAirsto::DisableMeshCollision()
{
	Super::DisableMeshCollision();

}

void AAirsto::EnableMeshCollision()
{
	Super::EnableMeshCollision();

}

void AAirsto::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AAirsto::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAirsto::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AAirsto::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AAirsto::Equip);
		EnhancedInputComponent->BindAction(UnequipAction, ETriggerEvent::Triggered, this, &AAirsto::Unequip);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AAirsto::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AAirsto::Dodge);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AAirsto::Interact);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AAirsto::Pause);

		
		EnhancedInputComponent->BindAction(LookXAction, ETriggerEvent::Triggered, this, &AAirsto::LookX);
		EnhancedInputComponent->BindAction(LookYAction, ETriggerEvent::Triggered, this, &AAirsto::LookY);
	}

}

void AAirsto::GetHit_Implementation(const FVector &ImpactPoint, AActor* Hitter)
{
    Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes ->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

float AAirsto::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
    SetHUDHealth();
    return DamageAmount;
}

void AAirsto::SetHUDHealth()
{
    if (DunGenOverlay && Attributes)
    {
        DunGenOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
    }
}

void AAirsto::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
}

void AAirsto::InitializeDunGenDialogueOverlay()
{
    APlayerController *PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        ADunGenHUD *DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
        if (DunGenHUD)
        {
            DunGenDialogueOverlay = DunGenHUD->GetDunGenDialogueOverlay();
        }
    }
}

void AAirsto::SetSpringArmLength(float Length)
{
	if (SpringArm)
	{
		SpringArm->TargetArmLength = Length;
	}
}


float AAirsto::GetMaxHealth() const
{
	if (Attributes)
	{
		return Attributes->GetMaxHealth();
	}

	return 0;
}

void AAirsto::SetMaxHealth(float Amount)
{
	if (Attributes)
	{
		Attributes->SetMaxHealth(Amount);
	}
}

float AAirsto::GetMaxStamina() const
{
	if (Attributes)
	{
		return Attributes->GetMaxStamina();
	}

	return 0;
}

void AAirsto::SetMaxStamina(float Amount)
{
	if (Attributes)
	{
		Attributes->SetMaxStamina(Amount);
	}
}

float AAirsto::GetStaminaRegenRate() const
{
	if (Attributes)
	{
		return Attributes->GetStaminaRegenRate();
	}

	return 0;
}

void AAirsto::SetStaminaRegenRate(float Amount)
{
	if (Attributes)
	{
		Attributes->SetStaminaRegenRate(Amount);
	}
}

void AAirsto::SetBaseWalkSpeed(float Speed)
{
	BaseWalkSpeed = Speed;
}

void AAirsto::SetCurrentWalkSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NewSpeed;
	}
}

void AAirsto::SetBaseDodgeSpeed(float Speed)
{
	BaseDodgeSpeed = Speed;
}

void AAirsto::SetCurrentDodgeSpeed(float Speed)
{
	CurrentDodgeSpeed = Speed;
}

float AAirsto::GetDodgeCost() const
{
	if (Attributes)
	{
		return Attributes->GetDodgeCost();
	}

	return 0;
}

void AAirsto::SetDodgeCost(float DodgeCost)
{
	if (Attributes)
	{
		Attributes->SetDodgeCost(DodgeCost);
	}
}

float AAirsto::GetWeaponDamage() const
{
	if (EquippedWeapon)
	{
		return EquippedWeapon->GetWeaponDamage();
	}

	return 0;
}

void AAirsto::SetWeaponDamage(float WeaponDamage)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponDamage(WeaponDamage);
	}
}

int32 AAirsto::GetTreasureAmount() const
{
	if (Attributes)
	{
		return Attributes->GetTreasure();
	}

	return 0;
}

int32 AAirsto::GetWisdomAmount() const
{
	if (Attributes)
	{
		return Attributes->GetWisdom();
	}

	return 0;
}

void AAirsto::SetWisdomMultiplier(float Multiplier)
{
	WisdomMultiplier = Multiplier;
}

void AAirsto::SelectDialogueOption(int32 OptionIndex)
{

	if (!DialogueTarget)
	{
        UE_LOG(LogTemp, Warning, TEXT("Airsto::SelectDialogueOption - DialogueTarget is null."));
        return;
	}

	IDialogueInterface* DialogueInterface = Cast<IDialogueInterface>(DialogueTarget);
	if (!DialogueInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("Airsto::SelectDialogueOption - DialogueTarget does not implement DialogueInterface."));
		return;
	}

	DialogueInterface->SelectDialogueOption(OptionIndex);

}

void AAirsto::ShowDialogueOptions(const TArray<FDialogueOption>& Options)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController) return;

    ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
    if (!DunGenHUD) return;

    UDunGenDialogueOverlay* DialogueOverlay = DunGenHUD->GetDunGenDialogueOverlay();
    if (!DialogueOverlay) return;

    DialogueOverlay->ShowDialogueOptions(Options);
}

void AAirsto::HideDialogueOptions()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController) return;

    ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
    if (!DunGenHUD) return;

    UDunGenDialogueOverlay* DialogueOverlay = DunGenHUD->GetDunGenDialogueOverlay();
    if (!DialogueOverlay) return;

    DialogueOverlay->HideDialogueOptions();
}

void AAirsto::ShowDialogue(const FText& SpeakerName, const FText& Text)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::ShowDialogue - PlayerController is null."));
		return;
	}

	ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
	if (!DunGenHUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::ShowDialogue - DunGenHUD is null."));
		return;
	}

	DunGenHUD->ShowDialogueOverlay();

	UDunGenDialogueOverlay* DialogueOverlay = DunGenHUD->GetDunGenDialogueOverlay();
	if (!DialogueOverlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::ShowDialogue - DialogueOverlay is null."));
		return;
	}

	DialogueOverlay->SetName(SpeakerName.ToString());
	DialogueOverlay->SetDialogueText(Text);

	//EnterDialogueInputMode();
}

void AAirsto::EnterDialogueInputMode()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::EnterDialogueInputMode - PlayerController is null."));
		return;
	}

	ADunGenHUD* DunGenHUD = Cast<ADunGenHUD>(PlayerController->GetHUD());
	if (!DunGenHUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::EnterDialogueInputMode - DunGenHUD is null."));
		return;
	}

	UDunGenDialogueOverlay* DialogueOverlay = DunGenHUD->GetDunGenDialogueOverlay();
	if (!DialogueOverlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::EnterDialogueInputMode - DialogueOverlay is null."));
		return;
	}

	//Unlock and Unhide the Mouse
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(DialogueOverlay->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	//Enter input mode
	PlayerController->SetInputMode(InputMode);
	//Show the mouse cursor
	PlayerController->bShowMouseCursor = true;
	//Stop moving the camera with the mouse
	PlayerController->SetIgnoreLookInput(true);
}

void AAirsto::ExitDialogueInputMode()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAirsto::ExitDialogueInputMode - PlayerController is null."));
		return;
	}

	//Restore the input mode
	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	//Hide the mouse cursor
	PlayerController->bShowMouseCursor = false;
	//Allow mouse to control camera
	PlayerController->SetIgnoreLookInput(false);
}

void AAirsto::HandleDodgeImpact()
{
    FVector Start = GetActorLocation();
    float Radius = 150.f;

    TArray<FHitResult> Hits;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        Start,
        Start,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        Params
    );

    if (!bHit) return;

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor) continue;

        //Only hit breakables
        if (HitActor->ActorHasTag(TEXT("Breakable")))
        {
            UE_LOG(LogTemp, Warning, TEXT("Dodge hit: %s"), *HitActor->GetName());

            IHitInterface* HitInterface = Cast<IHitInterface>(HitActor);
            if (HitInterface)
            {
                HitInterface->Execute_GetHit(HitActor, Hit.ImpactPoint, this);
            }
        }
    }
}

void AAirsto::HandleRespawn()
{
	AProceduralDungeonGameMode* GameMode = Cast<AProceduralDungeonGameMode>(GetWorld()->GetAuthGameMode());

	if (GameMode)
	{
		GameMode->SpawnPlayerAtMainSpawn();
	}

	Destroy();
}

void AAirsto::ShowInteractPrompt(const FText& PromptText)
{
    CurrentInteractPrompt = PromptText;
    bInteractPromptVisible = true;

    UE_LOG(LogTemp, Warning, TEXT("Prompt Shown: %s"), *PromptText.ToString());
}

void AAirsto::HideInteractPrompt()
{
    CurrentInteractPrompt = FText::GetEmpty();
    bInteractPromptVisible = false;

    UE_LOG(LogTemp, Warning, TEXT("Prompt Hidden"));
}

void AAirsto::SetHasWeapon(bool bPlayerHasWeapon)
{
	bHasWeapon = bPlayerHasWeapon;
}