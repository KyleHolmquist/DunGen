// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;
class USceneComponent;

UCLASS()
class PROCEDURALDUNGEON4_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();
 	void Equip(USceneComponent *InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
 	void Equip(USceneComponent *InParent, FName InSocketName);

    void DeactivateEmbers();

    void DisableSphereCollision();

    void PlayEquipSound();

    void AttachMeshToSocket(USceneComponent *InParent, const FName &InSocketName);

    TArray<AActor*> IgnoreActors;

protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable)
    void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    bool ActorIsSameType(AActor *OtherActor);

	UFUNCTION(BlueprintImplementableEvent)
    void CreateFields(const FVector &FieldLocation);

    void ExecuteGetHit(FHitResult &BoxHit);

private:
	void BoxTrace(FHitResult &BoxHit);

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    FVector BoxTraceExtent = FVector(5.f);

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties");
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
	EWeaponType WeaponType = EWeaponType::EWT_Sword;

	APawn* EventInstigator;

public:
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE float GetWeaponDamage() const { return Damage; }
	FORCEINLINE void SetWeaponDamage(float WeaponDamage) { Damage = WeaponDamage; }
	
};
