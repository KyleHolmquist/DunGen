#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DunGenHUD.generated.h"

class UDunGenOverlay;

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
public:
	FORCEINLINE UDunGenOverlay* GetDunGenOverlay() const { return DunGenOverlay; }
	
};
