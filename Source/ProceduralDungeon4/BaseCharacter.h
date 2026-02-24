// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HitInterface.h"
#include "WeaponTypes.h"
#include "CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class PROCEDURALDUNGEON4_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter);

	UFUNCTION(BlueprintNativeEvent)
	void Die();

	virtual bool CanAttack();
	bool IsAlive();
	virtual void Attack();
	virtual void Attack(const FInputActionValue& Value);
    void DirectionalHitReact(const FVector &ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);

	/** Montage */
	void PlayHitReactMontage(const FName& SectionName);
	virtual void PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	virtual void PlayDodgeMontage();
	void StopAttackMontage();

	void SpawnHitParticles(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	virtual void DeductStamina(float Stamina);
	void DisableCapsule();



	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;
	
	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	virtual void DisableMeshCollision();

	UFUNCTION(BlueprintCallable)
	virtual void EnableMeshCollision();

	UFUNCTION(BlueprintImplementableEvent)
	void OnAttack();

	UPROPERTY(EditAnywhere, Category = GAS, meta = (AllowPrivateAccess = "true"))
	float StaminaPerAttack = 5.f;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

private:
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	/** Animation Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Combat");
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat");
    UAnimMontage *HitReactMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Combat");
    UAnimMontage *DeathMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Combat");
    UAnimMontage *DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageSections;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
