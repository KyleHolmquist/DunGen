#pragma once

UENUM(BlueprintType)
enum class ETileType : uint8
{
	ETT_Empty UMETA(DisplayName = "Empty"),
	EWT_Floor UMETA(DisplayName = "Floor"),
	EWT_Wall UMETA(DisplayName = "Wall")
};

UENUM(BlueprintType)
enum class EDungeonTheme : uint8
{
	EDT_Castle UMETA(DisplayName = "Castle"),
	EDT_Fort UMETA(DisplayName = "Fort"),
	EDT_Cave UMETA(DisplayName = "Cave"),
	EDT_Dungeon UMETA(DisplayName = "Dungeon"),
	EDT_Mansion UMETA(DisplayName = "Mansion"),
	EDT_Crypt UMETA(DisplayName = "Crypt"),
	EDT_Temple UMETA(DisplayName = "Temple"),
	EDT_Ruins UMETA(DisplayName = "Ruins"),
	EDT_Palace UMETA(DisplayName = "Palace"),
	EDT_City_Streets UMETA(DisplayName = "City Streets")
};