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
class UDunGenDialogueOverlay;
class UAttributeComponent;
struct FDialogueOption;
class ADungeonManager;

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

	void AddToGoldAmount(int Amount);
	void AddToWisdomAmount(int Amount);

	void ShowInteractButton();
	void HideInteractButton();

protected:
	virtual void BeginPlay() override;

	virtual void PawnClientRestart() override;

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
	UInputAction* UnequipAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* DodgeAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* PauseAction;

	
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookXAction;
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookYAction;

	/**
	 * Callbacks for Input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
    void Equip(const FInputActionValue &Value);
    void Unequip(const FInputActionValue &Value);
    void GetWeaponType(AWeapon *OverlappingWeapon);
    void Arm(const FInputActionValue &Value);
    void Dodge(const FInputActionValue &Value);
    void Interact(const FInputActionValue &Value);
    void Pause(const FInputActionValue &Value);

    bool HasDodgeStamina();
    bool HasAttackStamina();;

    virtual void Attack(const FInputActionValue &Value) override;
    virtual void Jump() override;

	
	void LookX(const FInputActionValue& Value);
	void LookY(const FInputActionValue& Value);

    /* Combat */
	virtual void AttackBegin() override;
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

	EWeaponType WeaponType;

	bool IsOccupied();
	bool IsUnoccupied();

	//Dodge Movement
	UPROPERTY(EditAnywhere, Category=Stats)
	float BaseDodgeSpeed = 900.f;

	UPROPERTY(EditAnywhere, Category=Stats)
	float CurrentDodgeSpeed;

	FVector DodgeDirection = FVector::ZeroVector;

	float SavedBrakingFrictionFactor = 0.f;
	float SavedGroundFriction = 0.f;
	bool bSavedDodgeFriction = false;
	bool bHasWeapon = false;

	//Dialogue
	UPROPERTY()
	AActor* DialogueTarget;

	UFUNCTION(BlueprintCallable)
	void InitializeDunGenDialogueOverlay();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Stats);
	float BaseWalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Stats);
	float CurrentWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Stats);
	float WisdomMultiplier = 10.f;

	UPROPERTY(EditAnywhere, Category=Respawn)
	float RespawnDelay = 4.f;

	FTimerHandle RespawnTimerHandle;

	void HandleRespawn();

	void HandleDodgeImpact();

	UFUNCTION(BlueprintImplementableEvent)
    void CreateFields(const FVector &FieldLocation);

	bool bApplyingAttackFields = false;

    UPROPERTY(BlueprintReadOnly, Category = UI)
    FText CurrentInteractPrompt;

    UPROPERTY(BlueprintReadOnly, Category = UI)
    bool bInteractPromptVisible = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
    FString PlayerName = TEXT("");

	ADungeonManager* DungeonManager;

private:

    void InitializeEnhancedInput();
    void InitializeDunGenOverlay();
    void SetHUDHealth();

	UPROPERTY(VisibleAnywhere)
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	/* Character Components */

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages);
	UAnimMontage* EquipMontage;

	UPROPERTY(VisibleAnywhere, Category=HUD)
	UDunGenOverlay* DunGenOverlay;

	UPROPERTY(VisibleAnywhere, Category=HUD)
	UDunGenDialogueOverlay* DunGenDialogueOverlay;


public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE UDunGenOverlay* GetDunGenOverlay() { return DunGenOverlay; }
	FORCEINLINE UDunGenDialogueOverlay* GetDunGenDialogueOverlay() { return DunGenDialogueOverlay; }
	FORCEINLINE bool HasWeapon() { return bHasWeapon; }

	UFUNCTION(BlueprintCallable)
	void SetHasWeapon(bool bPlayerHasWeapon);

	UFUNCTION(BlueprintCallable)
	void SetDialogueTarget(AActor* Target) { DialogueTarget = Target; }

	UFUNCTION(BlueprintCallable)
	void ShowDialogueOptions(const TArray<FDialogueOption>& Options);

	UFUNCTION(BlueprintCallable)
	void SelectDialogueOption(int32 OptionIndex);

	UFUNCTION(BlueprintCallable)
	void HideDialogueOptions();

	UFUNCTION(BlueprintCallable)
	void SetSpringArmLength(float Length);

	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable)
	void SetMaxHealth(float Amount);

	UFUNCTION(BlueprintCallable)
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable)
	void SetMaxStamina(float Amount);

	UFUNCTION(BlueprintCallable)
	float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable)
	void SetStaminaRegenRate(float Rate);

	UFUNCTION(BlueprintCallable)
	float GetBaseWalkSpeed() const { return BaseWalkSpeed; }

	UFUNCTION(BlueprintCallable)
	void SetBaseWalkSpeed(float Speed);

	UFUNCTION(BlueprintCallable)
	void SetCurrentWalkSpeed(float Speed);

	UFUNCTION(BlueprintCallable)
	float GetBaseDodgeSpeed() const { return BaseDodgeSpeed; }

	UFUNCTION(BlueprintCallable)
	void SetBaseDodgeSpeed(float Speed);

	UFUNCTION(BlueprintCallable)
	float GetDodgeCost() const;

	UFUNCTION(BlueprintCallable)
	void SetDodgeCost(float Cost);

	UFUNCTION(BlueprintCallable)
	void SetCurrentDodgeSpeed(float Speed);

	UFUNCTION(BlueprintCallable)
	float GetWeaponDamage() const;

	UFUNCTION(BlueprintCallable)
	void SetWeaponDamage(float Damage);

	UFUNCTION(BlueprintCallable)
	int32 GetTreasureAmount() const;

	UFUNCTION(BlueprintCallable)
	int32 GetWisdomAmount() const;

	UFUNCTION(BlueprintCallable)
	float GetWisdomMultiplier() const { return WisdomMultiplier; }

	UFUNCTION(BlueprintCallable)
	void SetWisdomMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable)
	void ShowDialogue(const FText& SpeakerName, const FText& Text);

	UFUNCTION(BlueprintCallable)
	void EnterDialogueInputMode();

	UFUNCTION(BlueprintCallable)
	void ExitDialogueInputMode();

	void ShowInteractPrompt(const FText& PromptText);
	void HideInteractPrompt();

    UFUNCTION(BlueprintCallable, Category = Player)
    void SetPlayerName(const FString& NewName);

    UFUNCTION(BlueprintCallable, Category = Player)
    FString GetPlayerName() const;

    UFUNCTION(BlueprintCallable, Category = Player)
    bool HasValidPlayerName() const;

	UFUNCTION(BlueprintCallable)
	void SetTreasureAmount(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void RemoveTreasure(int32 Amount);

};
