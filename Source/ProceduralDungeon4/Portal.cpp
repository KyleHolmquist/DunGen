// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Airsto.h"
#include "DungeonManager.h"
#include "Components/ArrowComponent.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	SetRootComponent(PortalMesh);

	PortalEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	PortalEffect->SetupAttachment(GetRootComponent());

	EntryBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryBox"));
	EntryBox->SetupAttachment(GetRootComponent());
	EntryBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnBoxOverlap);
	EntryBox->OnComponentEndOverlap.AddDynamic(this, &APortal::OnBoxEndOverlap);

	ExitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBox"));
	ExitBox->SetupAttachment(GetRootComponent());
	ExitBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnBoxOverlap);
	ExitBox->OnComponentEndOverlap.AddDynamic(this, &APortal::OnBoxEndOverlap);

	TeleportBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TeleportBox"));
	TeleportBox->SetupAttachment((GetRootComponent()));

	ExitDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirectionArrow"));
	ExitDirectionArrow->SetupAttachment(GetRootComponent());
	

}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	if (!bTeleportLocationSet)
	{
		TeleportLocation = GetTeleportPointLocation();
	}

	DungeonManager = Cast<ADungeonManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonManager::StaticClass()));

	if (bIsHomePortal)
	{
		SetPortalActive(false);
	}
	
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APortal::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != EntryBox) return;
	if (!bCanTeleport) return;

	AAirsto* Airsto = Cast<AAirsto>(OtherActor);
	if (!Airsto) return;

	bCanTeleport = false;

	SpawnEntrySound();
	TeleportPlayer(Airsto);

	// const FString LocationString = TeleportLocation.ToString();
	// UE_LOG(LogTemp, Warning, TEXT("Teleporting to %s"), *LocationString);
}

void APortal::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComponent != EntryBox && OverlappedComponent != ExitBox) return;

	if (Cast<AAirsto>(OtherActor))
	{
		bCanTeleport = true;
	}
}
	
void APortal::SpawnEntrySound()
{
	if (TeleportSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			TeleportSound,
			GetActorLocation()
		);
	}
}

void APortal::TeleportPlayer(AAirsto* Airsto)
{
	if (!Airsto) return;

	if (DungeonManager)
	{
		DungeonManager->SetPlayerEnteredPortal(true);
	}
		
	if (ConnectedPortal)
	{
		ConnectedPortal->bCanTeleport = false;
		FVector Location = ConnectedPortal->GetActorLocation();
		const FVector MoveUp = Location + FVector(0.f, 0.f, 50.f);
		const FVector ExitDirection = ConnectedPortal->GetExitDirection();
		Airsto->SetActorLocation(MoveUp);
		Airsto->SetActorRotation(ExitDirection.Rotation());
		FVector TargetScale = bIsHomePortal ? FVector(0.35f) : FVector(1.0f);
		float TargetArmLength = bIsHomePortal ? 150.f : 450.f;
		float TargetWalkSpeed = bIsHomePortal ? (Airsto->GetBaseWalkSpeed() / 3) : Airsto->GetBaseWalkSpeed();
		float TargetDodgeSpeed = bIsHomePortal ? (Airsto->GetBaseDodgeSpeed() / 3) : Airsto->GetBaseDodgeSpeed(); 
		Airsto->SetActorScale3D(TargetScale);
		Airsto->SetSpringArmLength(TargetArmLength);
		Airsto->SetCurrentWalkSpeed(TargetWalkSpeed);
		Airsto->SetCurrentDodgeSpeed(TargetDodgeSpeed);

	}
	
}

void APortal::SetTeleportLocation(FVector SelectedTeleportLocation)
{
	TeleportLocation = SelectedTeleportLocation;
	bTeleportLocationSet = true;
}

FVector APortal::GetTeleportPointLocation() const
{
	if (TeleportBox)
	{
		return TeleportBox->GetComponentLocation();
	}

	return GetActorLocation() + FVector(0.f, 0.f, 100.f);
}

void APortal::SetCanTeleport(bool State)
{
	bCanTeleport = State;
}

FVector APortal::GetPortalForwardVector() const
{
	return PortalMesh ? PortalMesh->GetForwardVector() : GetActorForwardVector();
}

FVector APortal::GetExitDirection() const
{
	if (ExitDirectionArrow)
	{
		return ExitDirectionArrow->GetForwardVector();
	}

	return GetActorForwardVector();
}

void APortal::SetPortalActive(bool bActive)
{
	if (PortalMesh)
	{
		PortalMesh->SetVisibility(bActive, true);
		PortalMesh->SetHiddenInGame(!bActive, true);
	}

	if (PortalEffect)
	{
		PortalEffect->SetVisibility(bActive, true);
		PortalEffect->SetHiddenInGame(!bActive, true);

		if (bActive)
		{
			PortalEffect->Activate(true);
		}
		else
		{
			PortalEffect->Deactivate();
		}
	}

	if (EntryBox)
	{
		EntryBox->SetVisibility(false, true);
		EntryBox->SetHiddenInGame(true, true);
		EntryBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (ExitBox)
	{
		ExitBox->SetVisibility(false, true);
		ExitBox->SetHiddenInGame(true, true);
		ExitBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (TeleportBox)
	{
		TeleportBox->SetVisibility(false, true);
		TeleportBox->SetHiddenInGame(true, true);
		TeleportBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	bCanTeleport = bActive;
}