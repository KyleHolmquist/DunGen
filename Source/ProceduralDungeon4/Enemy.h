// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "WeaponTypes.h"
#include "DunGenEnums.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;
class AItem;

USTRUCT(BlueprintType)
struct FEnemyConfig
{
    GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Dungeon)
    float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Dungeon)
    float WisdomAmount = 10.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Dungeon)
    TSubclassOf<AItem> InitTreasureClass;

	//WeaponDamage

};

UCLASS()
class PROCEDURALDUNGEON4_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

    void Initialize(const FEnemyConfig& InConfig);
	void SetPatrolPoints(AActor* PatrolTarget1, AActor* PatrolTarget2);
	void SetAttackTimer(float TimerMin, float TimerMax);
	void StartPatrolling();

	/* <AActor> */
    virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/* </AActor> */

	/** <IHitInterface*/
	virtual void GetHit_Implementation(const FVector &ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface*/

	void SetMaxHealth(int SelectedMaxHealth);
	void SetWisdomAmount(int SelectedWisdomAmount);
	void SetHealthPickupAmount(int SelectedHealthPickupAmount);
	void SetTreasureClass(TSubclassOf<AItem> SelectedTreasureClass);
	void AddTreasureToRewardTable();

protected:

	/* <AActor>*/
    virtual void BeginPlay() override;
    /* </AActor> */

    /* <ABaseCharacter>*/
    virtual void Die_Implementation() override;
    virtual void Attack() override;
    virtual bool CanAttack() override;
    virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/* </ABaseCharacter> */

	UFUNCTION(BlueprintCallable)
	void SpawnReward();

	UFUNCTION(BlueprintCallable)
	void SpawnTreasure();

	UFUNCTION(BlueprintCallable)
    void SpawnWisdom();

	UFUNCTION(BlueprintCallable)
	void SpawnHealthPickup();

	UFUNCTION(BlueprintCallable)
	void RagdollCharacter();

	EWeaponType WeaponType;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly
	// TSubclassOf<AHealthPickup> HealthPickupClass;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	TArray<EDungeonTheme> DungeonThemes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Dungeon)
	FEnemyConfig EnemyConfig;

	FTimerHandle PatrolStartRetryHandle;

	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Procedural Dungeon")
	TArray<TSubclassOf<AItem>> RewardTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Procedural Dungeon")
	TSubclassOf<AItem> TreasureClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Procedural Dungeon")
	TSubclassOf<AItem> WisdomClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Procedural Dungeon")
	TSubclassOf<AItem> HealthPickupClass;

	bool bDeathHandled = false;

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
	bool IsOutsideCombatRadius();
	bool IsChasing();
	bool IsOutsideAttackRadius();
	bool IsAttacking();
	void ClearPatrolTimer();
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
    void SpawnDefaultWeapon();

	//Callback for OnPawnSeen in PawnSensing Component
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn); 

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

public:
	FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }
	FORCEINLINE TArray<EDungeonTheme> GetThemes() const { return DungeonThemes; }

};
