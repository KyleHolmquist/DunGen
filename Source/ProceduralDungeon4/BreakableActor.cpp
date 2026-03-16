


#include "BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Treasure.h"
#include "Wisdom.h"
#include "HealthPickup.h"
#include "Components/CapsuleComponent.h"


ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("Geometry Collection"));
	SetRootComponent(GeometryCollection);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	GeometryCollection->SetNotifyBreaks(true);
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnChaosBreakEvent);

	RewardTable.Reset();

	if (WisdomClass)
	{
		RewardTable.Add(WisdomClass);
	}

	if (HealthPickupClass)
	{
		RewardTable.Add(HealthPickupClass);
	}
	
}


void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::OnChaosBreakEvent(const FChaosBreakEvent& BreakEvent)
{
	HandleHit();
}

void ABreakableActor::HandleHit()
{
	if (bBroken) return;
	bBroken = true;
	UWorld* World = GetWorld();
	if (World && RewardTable.Num() > 0)
	{
		FVector Location = GetActorLocation();
		Location.Z += 75.f;

		const int32 Selection = FMath::RandRange(0, RewardTable.Num() - 1);

		AItem* SpawnedItem = World->SpawnActor<AItem>(RewardTable[Selection], Location, GetActorRotation());
	}
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (bBroken) return;

	SetLifeSpan(2.f);

	bBroken = true;
	UWorld* World = GetWorld();
	if (World && RewardTable.Num() > 0)
	{
		FVector Location = GetActorLocation();
		Location.Z += 15.f;

		const int32 Selection = FMath::RandRange(0, RewardTable.Num() - 1);

		World->SpawnActor<AItem>(RewardTable[Selection], Location, GetActorRotation());
	}
}


void ABreakableActor::SetTreasureClass(TSubclassOf<AItem> SelectedTreasureClass )
{
	UE_LOG(LogTemp, Warning, TEXT("We're adding treasure classes"));
	TreasureClass = SelectedTreasureClass;
	RewardTable.Add(TreasureClass);
}