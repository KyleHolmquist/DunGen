// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Airsto.h"
#include "DungeonManager.h"

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
	

}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	if (!bTeleportLocationSet)
	{
		TeleportLocation = GetTeleportPointLocation();
	}
	

	DungeonManager = Cast<ADungeonManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonManager::StaticClass()));
	
	
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

	if (TeleportLocation.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal '%s' has no TeleportLocation set."), *GetName());
		return;
	}

	bCanTeleport = false;

	SpawnEntrySound();
	TeleportPlayer(Airsto, TeleportLocation);
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

void APortal::TeleportPlayer(AAirsto* Airsto, FVector Location)
{
	if (!Airsto) return;

	if (DungeonManager)
	{
		DungeonManager->SetPlayerEnteredPortal(true);
	}
		
	if (ConnectedPortal)
	{
		ConnectedPortal->bCanTeleport = false;
	}

	const FVector MoveUp = Location + FVector(0.f, 0.f, 100.f);
	Airsto->SetActorLocation(MoveUp);
	
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