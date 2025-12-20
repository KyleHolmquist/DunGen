// Fill out your copyright notice in the Description page of Project Settings.


#include "Airsto.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAirsto::AAirsto()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	SpringArm->bUsePawnControlRotation = true;
	ViewCamera->bUsePawnControlRotation = false;

	// Typical 3rd-person character settings:
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

}

// Called when the game starts or when spawned
void AAirsto::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(AirstoContext, 0);
		}
		
	}

	InitializeEnhancedInput();
	
}

void AAirsto::InitializeEnhancedInput()
{
	APlayerController *PlayerController = Cast<APlayerController>(Controller);

    if (!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	
	if (Subsystem && AirstoContext)
	{
		Subsystem->AddMappingContext(AirstoContext, 1);
	}

	Tags.Add(FName("Player"));
}

// Called every frame
void AAirsto::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAirsto::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
    	UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent OK"));
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAirsto::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAirsto::Look);
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AAirsto::Roll);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AAirsto::Attack);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerInputComponent is NOT an EnhancedInputComponent"));

	}

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

void AAirsto::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}
 
void AAirsto::Roll(const FInputActionValue& Value)
{
	PlayRollMontage();
}

void AAirsto::Attack(const FInputActionValue& Value)
{
	bool AttackBool = Value.Get<bool>();
	if (AttackBool) UE_LOG(LogTemp, Warning, TEXT("Attack triggered value"));
	PlayAttackMontage();
}

void AAirsto::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage);
        AnimInstance->Montage_JumpToSection(SectionName, Montage);
    }
}

void AAirsto::PlayRollMontage()
{
	PlayMontageSection(RollMontage, FName("Default"));
}

void AAirsto::PlayAttackMontage()
{
	PlayMontageSection(AttackMontage, FName("Attack_Sword"));
}