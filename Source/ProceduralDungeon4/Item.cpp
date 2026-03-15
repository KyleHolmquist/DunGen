// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PickupInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Airsto.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment((GetRootComponent()));

	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	ItemEffect->SetupAttachment(GetRootComponent());


}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	if (bSetItemLaunching)
	{
		if (ItemMesh)
		{
			//Physics values
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetEnableGravity(true);
			ItemMesh->SetLinearDamping(LinearDamping);
			ItemMesh->SetAngularDamping(AngularDamping);
			ItemMesh->SetMobility(EComponentMobility::Movable);

			//Launch the Item
			FVector Dir = FMath::VRand();
			//Ensure upward direction
			Dir.Z = FMath::Abs(Dir.Z);

			float Strength = FMath::RandRange(LaunchStrengthMin, LaunchStrengthMax);

			ItemMesh->AddImpulse(Dir * Strength, NAME_None, true);
		}
	}
	
}

float AItem::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

float AItem::TransformedCos()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ItemState == EItemState::EIS_Hovering)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
	}

}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->SetOverlappingItem(this);
	}

	AAirsto* Airsto = Cast<AAirsto>(OtherActor);
	if (Airsto)
	{
		Airsto->SetOverlappingItem(this);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->SetOverlappingItem(nullptr);
	}

	AAirsto* Airsto = Cast<AAirsto>(OtherActor);
	if (Airsto)
	{
		Airsto->SetOverlappingItem(nullptr);
	}

}

void AItem::SpawnPickupSystem()
{
    if (PickupEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            PickupEffect,
            GetActorLocation()
        );
    }
}

void AItem::SpawnPickupSound()
{
	if (PickupSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
}