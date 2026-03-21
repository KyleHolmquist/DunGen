// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALDUNGEON4_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RegenStamina(float DeltaTime);

	void InitHealth(int32 InitialHealth);
	void InitWisdom(int32 InitialWisdom);

protected:
	virtual void BeginPlay() override;

private:

	//Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StartingHealth;

	//Current Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Gold;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Wisdom;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 AttackCost;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 DodgeCost = 20.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StaminaRegenRate = 2.f;

public:
	void ReceiveDamage(float Damage);
	void UseStamina(float StaminaCost);
	float GetHealthPercent();
	float GetStaminaPercent();
	bool IsAlive();
	void AddWisdom(int32 WisdomAmount);
	void AddGold(int32 NumGold);

	FORCEINLINE int32 GetTreasure() const { return Gold; }
	FORCEINLINE int32 GetWisdom() const { return Wisdom; }
	FORCEINLINE int32 GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE int32 GetAttackCost() const { return AttackCost; }
	FORCEINLINE int32 GetMaxStamina() const { return MaxStamina; }
	FORCEINLINE int32 GetStamina() const { return Stamina; }
	FORCEINLINE int32 GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE int32 GetHealth() const { return Health; }
	FORCEINLINE float GetStaminaRegenRate() { return StaminaRegenRate; }

	void SetMaxHealth(int Amount);
	void SetMaxStamina(int Amount);
	void SetStaminaRegenRate(float Rate);
	void SetDodgeCost(float Cost);

		
};
