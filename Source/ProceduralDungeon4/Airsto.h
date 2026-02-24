// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "PickupInterface.h"
#include "Airsto.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UInputMappingContext;
class UInputAction;
class AItem;
class AWisdom;
class ATreasure;
class UDunGenOverlay;
class UAttributeComponent;

UCLASS()
class PROCEDURALDUNGEON4_API AAirsto : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	AAirsto();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
    virtual void GetHit_Implementation(const FVector &ImpactPoint, AActor *Hitter) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddWisdom(AWisdom* Wisdom) override;
	virtual void AddGold(ATreasure* Treasure) override;

protected:
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = Input)
    UInputMappingContext* AirstoMappingContext;

	/** Input Actions */
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* MovementAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* DodgeAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* InteractAction;

	
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookXAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookYAction;

	/**
	 * Callbacks for Input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
    void Equip(const FInputActionValue &Value);
    void GetWeaponType(AWeapon *OverlappingWeapon);
    void Arm(const FInputActionValue &Value);
    void Dodge(const FInputActionValue &Value);
    void Interact(const FInputActionValue &Value);

    bool HasDodgeStamina();
    bool HasAttackStamina();

    virtual void Attack(const FInputActionValue &Value) override;
    virtual void Jump() override;

	
	void LookX(const FInputActionValue& Value);
	void LookY(const FInputActionValue& Value);

    /* Combat */
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual void DisableMeshCollision() override;
	virtual void EnableMeshCollision() override;
	virtual bool CanAttack() override;
	bool CanDisarm();
	bool CanArm();
	void PlayEquipMontage(const FName& SectionName);
	virtual void HandleDamage(float DamageAmount) override;
	virtual void Die_Implementation() override;

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

	void ShowInteractButton();
	void HideInteractButton();

	EWeaponType WeaponType;

	bool IsOccupied();
	bool IsUnoccupied();

private:

    void InitializeEnhancedInput();
    void InitializeDunGenOverlay();
    void SetHUDHealth();

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	/* Character Components */

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = "Montages");
	UAnimMontage* EquipMontage;

	UPROPERTY()
	UDunGenOverlay* DunGenOverlay;


public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE UDunGenOverlay* GetDunGenOverlay() { return DunGenOverlay; }

};
