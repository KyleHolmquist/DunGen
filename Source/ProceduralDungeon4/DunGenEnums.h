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
	EDS_Castle UMETA(DisplayName = "Castle"),
	EDS_Fort UMETA(DisplayName = "Fort"),
	EDS_Cave UMETA(DisplayName = "Cave"),
	EDS_Dungeon UMETA(DisplayName = "Dungeon"),
	EDS_Mansion UMETA(DisplayName = "Mansion"),
	EDS_Crypt UMETA(DisplayName = "Crypt"),
	EDS_Temple UMETA(DisplayName = "Temple"),
	EDS_Ruins UMETA(DisplayName = "Ruins"),
	EDS_Palace UMETA(DisplayName = "Palace"),
	EDS_City_Streets UMETA(DisplayName = "City Streets")
};