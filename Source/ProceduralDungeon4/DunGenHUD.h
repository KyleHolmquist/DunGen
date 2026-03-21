#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DunGenHUD.generated.h"

class UDunGenOverlay;
class UDunGenDialogueOverlay;
class UDunGenPauseMenu;
class UDunGenControlsMenu;
class UDunGenMainMenu;

UENUM(BlueprintType)
enum class EControlsMenuSource : uint8
{
    None UMETA(DisplayName = "None"),
    PauseMenu UMETA(DisplayName = "Pause Menu"),
    MainMenu UMETA(DisplayName = "Main Menu")
};

UCLASS()
class PROCEDURALDUNGEON4_API ADunGenHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
private:

	UPROPERTY(EditDefaultsOnly, Category = DunGen)
	TSubclassOf<UDunGenOverlay> DunGenOverlayClass;

	UPROPERTY()
	UDunGenOverlay* DunGenOverlay;

	UPROPERTY(EditDefaultsOnly, Category = DunGen)
	TSubclassOf<UDunGenDialogueOverlay> DunGenDialogueOverlayClass;

	UPROPERTY()
	UDunGenDialogueOverlay* DunGenDialogueOverlay;

	UPROPERTY(EditDefaultsOnly, Category = DunGen)
	TSubclassOf<UDunGenPauseMenu> DunGenPauseMenuClass;

	UPROPERTY()
	UDunGenPauseMenu* DunGenPauseMenu;

	UPROPERTY(EditDefaultsOnly, Category = DunGen)
	TSubclassOf<UDunGenControlsMenu> DunGenControlsMenuClass;

	UPROPERTY()
	UDunGenControlsMenu* DunGenControlsMenu;

	UPROPERTY(EditDefaultsOnly, Category = DunGen)
	TSubclassOf<UDunGenMainMenu> DunGenMainMenuClass;

	UPROPERTY()
	UDunGenMainMenu* DunGenMainMenu;

	UPROPERTY()
	EControlsMenuSource ControlsMenuSource = EControlsMenuSource::None;

public:
	FORCEINLINE UDunGenOverlay* GetDunGenOverlay() const { return DunGenOverlay; }
	FORCEINLINE UDunGenDialogueOverlay* GetDunGenDialogueOverlay() const { return DunGenDialogueOverlay; }

	UFUNCTION(BlueprintCallable)
	void ShowDialogueOverlay();
	UFUNCTION(BlueprintCallable)
	void HideDialogueOverlay();
	UFUNCTION(BlueprintCallable)
	void StartGame();
	UFUNCTION(BlueprintCallable)
	void ShowPauseMenu();
	UFUNCTION(BlueprintCallable)
	void HidePauseMenu();
	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu();
	UFUNCTION(BlueprintCallable)
	void ShowControlsMenu(EControlsMenuSource Source);
	UFUNCTION(BlueprintCallable)
	void ReturnFromControlsMenu();
	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();
	UFUNCTION(BlueprintCallable)
	void HideMainMenu();
	UFUNCTION(BlueprintCallable)
	void GoToMainMenuFromPause();


	
	
};
