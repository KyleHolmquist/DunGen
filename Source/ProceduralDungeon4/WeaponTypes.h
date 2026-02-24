#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Sword UMETA(DisplayName = "Sword"),
	EWT_Axe UMETA(DisplayName = "Axe"),
	EWT_Mace UMETA(DisplayName = "Mace"),
	EWT_Mallet UMETA(DisplayName = "Mallet")
};