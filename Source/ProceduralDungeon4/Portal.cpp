// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Airsto.h"

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

	if(TeleportBox)
	{
		TeleportLocation = TeleportBox->GetComponentLocation();
	}
	
	
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APortal::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent == EntryBox)
	{
		if (bCanTeleport)
		{
			if (AAirsto* Airsto = Cast<AAirsto>(OtherActor))
			{
				TeleportPlayer(Airsto, ConnectedPortal->GetTeleportLocation());
			}
		}
	}
}

void APortal::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComponent == ExitBox)
	{
		if (AAirsto* Airsto = Cast<AAirsto>(OtherActor))
		{
			bCanTeleport = true;
		}
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
	
	Airsto->SetActorLocation(Location);
	
}

void APortal::SetTeleportLocation(FVector SelectedTeleportLocation)
{
	TeleportLocation = SelectedTeleportLocation;
}