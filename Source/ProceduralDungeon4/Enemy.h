// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "WeaponTypes.h"
#include "DunGenEnums.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class PROCEDURALDUNGEON4_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	/* <AActor> */
    virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/* </AActor> */

	/** <IHitInterface*/
	virtual void GetHit_Implementation(const FVector &ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface*/

protected:

	/* <AActor>*/
    virtual void BeginPlay() override;
    /* </AActor> */

    /* <ABaseCharacter>*/
    virtual void Die_Implementation() override;
    void SpawnWisdom();
    virtual void Attack() override;
    virtual bool CanAttack() override;
    virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/* </ABaseCharacter> */

	EWeaponType WeaponType;


	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) override;

private:
	/* AI Behavior */
	void InitializeEnemy();

	/* Patrolling */
	void CheckPatrolTarget();
	void PatrolTimerFinished();
	void CheckCombatTarget();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	bool IsOutsideCombatRadius();
	bool IsChasing();
	bool IsOutsideAttackRadius();
	bool IsAttacking();
	void ClearPatrolTimer();
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
    void SpawnDefaultWeapon();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn); // Callback foor OnPawnSeen in PawnSensing Component

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;

	/* Combat */
	void StartAttackTimer();
	void ClearAttackTimer();
	void ChaseTarget();

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;

	bool IsInsideAttackRadius();
	bool InTargetRange(AActor* Target, double Radius);
	bool IsDead();
	bool IsEngaged();

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> WeaponClass;


	AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, Category = Combat)
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptanceRadius = 50.f;

	UPROPERTY()
	class AAIController* EnemyController;

	// Currect patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation");
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;
	
	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DeathLifeSpan = 8.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class AWisdom> WisdomClass;

public:
	FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }

};
