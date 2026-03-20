// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UAttributeComponent::ReceiveDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
}

void UAttributeComponent::UseStamina(float StaminaCost)
{
	Stamina = FMath::Clamp(Stamina - StaminaCost, 0.f, MaxStamina);
}

float UAttributeComponent::GetHealthPercent()
{
	return Health / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent()
{
	return Stamina / MaxStamina;
}

bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::AddWisdom(int32 WisdomAmount)
{
	Wisdom += WisdomAmount;
}

void UAttributeComponent::AddGold(int32 NumGold)
{
	Gold += NumGold;
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttributeComponent::RegenStamina(float DeltaTime)
{
	Stamina = FMath::Clamp(Stamina + StaminaRegenRate * DeltaTime, 0.f, MaxStamina);
}

void UAttributeComponent::InitHealth(int32 InitialHealth)
{
	Health = InitialHealth;
}
void UAttributeComponent::InitWisdom(int32 InitialWisdom)
{
	Wisdom = InitialWisdom;
}

void UAttributeComponent::SetMaxHealth(int Amount)
{
	MaxHealth = Amount;
}
void UAttributeComponent::SetMaxStamina(int Amount)
{
	MaxStamina = Amount;
}
void UAttributeComponent::SetStaminaRegenRate(float Rate)
{
	StaminaRegenRate = Rate;
}
void UAttributeComponent::UAttributeComponent::SetDodgeCost(float Cost)
{
	DodgeCost = Cost;
}