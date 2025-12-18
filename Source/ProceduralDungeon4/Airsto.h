// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Airsto.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UInputMappingContext;
class UInputAction;

UCLASS()
class PROCEDURALDUNGEON4_API AAirsto : public ACharacter
{
	GENERATED_BODY()

public:
	AAirsto();

protected:
	virtual void BeginPlay() override;

	void InitializeEnhancedInput();

	void MoveForward(float Value);
	void MoveRight(float Value);

	void Move(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* AirstoContext;

	UPROPERTY(EditAnywhere, Catrgory="Input")
	UInputAction* MoveAction;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
