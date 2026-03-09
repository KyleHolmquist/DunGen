#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DunGenHUD.generated.h"

class UDunGenOverlay;
class UDunGenDialogueOverlay;

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
public:
	FORCEINLINE UDunGenOverlay* GetDunGenOverlay() const { return DunGenOverlay; }
	FORCEINLINE UDunGenDialogueOverlay* GetDunGenDialogueOverlay() const { return DunGenDialogueOverlay; }

	void ShowDialogueOverlay();
	void HideDialogueOverlay();

	
	
};
