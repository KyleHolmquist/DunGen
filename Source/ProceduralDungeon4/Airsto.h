// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PickupInterface.h"
#include "HitInterface.h"
#include "CharacterTypes.h"
#include "Weapon.h"
#include "Airsto.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UInputMappingContext;
class UInputAction;
class AItem;
//class UAttributeComponent;

UCLASS()
class PROCEDURALDUNGEON4_API AAirsto : public ACharacter, public IHitInterface, public IPickupInterface
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
	void Equip(const FInputActionValue& Value);

    void GetWeaponType(AWeapon *OverlappingWeapon);

    bool HasDodgeStamina();
    bool HasAttackStamina();

	// -- Animation Callbacks--
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayRollMontage();
	void PlayAttackMontage();
	void PlayEquipMontage(const FName& SectionName);

	
	// -- Combat Helpers

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	UFUNCTION(BlueprintCallable)
	void DodgeEnd();
	UFUNCTION(BlueprintCallable)
	void BlockEnd();
	UFUNCTION(BlueprintCallable)
	void DisableMeshCollision();
	UFUNCTION(BlueprintCallable)
	void EnableMeshCollision();
	
	void HandleDamage(float DamageAmount);
	void Die_Implementation();

	bool CanAttack();
	bool CanBlock();
	bool CanDisarm();
	bool CanArm();
	

	UFUNCTION(BlueprintCallable)
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void Arm();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToSpine();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	// -- Animation Variables --
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

	
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	AWeapon* EquippedWeapon;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	// UAttributeComponent* Attributes;

private:

	// -- Enum States -- 
	UPROPERTY(BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;
	EWeaponType WeaponType;

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
	UPROPERTY(EditAnywhere, Category="Input | Combat")
	UInputAction* EquipAction;


	// -- Animation Montages --
    UPROPERTY(EditDefaultsOnly, Category = "Animation Montages");
    UAnimMontage *RollMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Animation Montages");
    UAnimMontage *AttackMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Animation Montages");
    UAnimMontage *EquipMontage;

	// -- Items //
	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;


public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
