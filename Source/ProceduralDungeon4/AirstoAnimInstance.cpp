// Fill out your copyright notice in the Description page of Project Settings.


#include "AirstoAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"



void UAirstoAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

    Airsto = Cast<AAirsto>(TryGetPawnOwner());
    if (Airsto)
    {
        AirstoMovement = Airsto->GetCharacterMovement();
    }
}

void UAirstoAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

    if (AirstoMovement)
    {
        GroundSpeed = UKismetMathLibrary::VSizeXY(AirstoMovement->Velocity); 
        CharacterState = Airsto->GetCharacterState();
        ActionState = Airsto->GetActionState();
        DeathPose = Airsto->GetDeathPose();
    }
}