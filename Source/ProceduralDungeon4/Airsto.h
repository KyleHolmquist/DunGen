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

	// -- Input Callbacks --
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);

	// -- Animation Callbacks--
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayRollMontage();
	void PlayAttackMontage();

private:

	// -- Character Components --
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	// -- Input --
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* AirstoContext;

	// -- Movement -- 
	UPROPERTY(EditAnywhere, Category="Input | Movement")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category="Input | Movement")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category="Input | Movement")
	UInputAction* RollAction;

	// -- Combat --
	UPROPERTY(EditAnywhere, Category="Input | Combat")
	UInputAction* AttackAction;


	// -- Animation Montages --
    UPROPERTY(EditDefaultsOnly, Category = "Animation Montages");
    UAnimMontage *RollMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Animation Montages");
    UAnimMontage *AttackMontage;


public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
