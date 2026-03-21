// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HitInterface.h"
#include "NiagaraComponent.h"
#include "Airsto.h"
#include "Components/WidgetComponent.h"
#include "InteractPromptWidget.h"

AWeapon::AWeapon()
{ 
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

    InteractPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractPromptWidget"));
    InteractPromptWidget->SetupAttachment(RootComponent);
    InteractPromptWidget->SetWidgetSpace(EWidgetSpace::Screen); // usually easiest for readability
    InteractPromptWidget->SetDrawAtDesiredSize(true);
    InteractPromptWidget->SetVisibility(false);
    InteractPromptWidget->SetHiddenInGame(true);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
    ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
    AttachMeshToSocket(InParent, InSocketName);
    DisableSphereCollision();
    PlayEquipSound();
    DeactivateEmbers();
}

void AWeapon::Equip(USceneComponent *InParent, FName InSocketName)
{
    FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
    //ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
	AttachMeshToSocket(InParent, InSocketName);
    ItemState = EItemState::EIS_Equipped;
	PlayEquipSound();
    DisableSphereCollision();
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	if (AActor* OwnerActor = GetOwner())
	{
		ActorsToIgnore.AddUnique(OwnerActor);
	}

	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		BoxTraceExtent,
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		//bShowBoxDebug? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		EDrawDebugTrace::None,
		BoxHit,
		true,
		FColor::Red,
		FColor::Green,
		5.f
		);
	
	if (AActor* HitActor = BoxHit.GetActor())
	{
		IgnoreActors.AddUnique(BoxHit.GetActor());

	}
}

void AWeapon::ActivateEmbers()
{
    if (ItemEffect)
    {
        ItemEffect->Activate();
    }
}

void AWeapon::DeactivateEmbers()
{
    if (ItemEffect)
    {
        ItemEffect->Deactivate();
    }
}
void AWeapon::DisableSphereCollision()
{
    if (Sphere)
    {
        Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}
void AWeapon::PlayEquipSound()
{
    if (EquipSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
    }
}
void AWeapon::AttachMeshToSocket(USceneComponent *InParent, const FName &InSocketName)
{
    FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
    ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
	FString SocketName = InSocketName.ToString();
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *SocketName);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
    AAirsto* Airsto = Cast<AAirsto>(OtherActor);
    if (Airsto && InteractPromptWidget && !Airsto->HasWeapon())
    {
        InteractPromptWidget->SetVisibility(true);
        InteractPromptWidget->SetHiddenInGame(false);
    }

    if (InteractPromptWidget)
	{
		if (UInteractPromptWidget* PromptWidget = Cast<UInteractPromptWidget>(InteractPromptWidget->GetUserWidgetObject()))
		{
			PromptWidget->SetPromptText(InteractionPrompt);
		}
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    AAirsto* Airsto = Cast<AAirsto>(OtherActor);

    if (Airsto && InteractPromptWidget)
    {
        InteractPromptWidget->SetVisibility(false);
        InteractPromptWidget->SetHiddenInGame(true);
    }


}

void AWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (!OtherActor) return;
	if (OtherActor == this) return;
	if (!OtherComp) return;

    if (ActorIsSameType(OtherActor)) return;

	FHitResult BoxHit;
	BoxTrace(BoxHit);

	AActor* MyOwner = GetOwner();
	if (!MyOwner) return;
	if (BoxHit.GetActor() == nullptr) return;
	if (BoxHit.GetActor() == MyOwner) return;
	
	if (BoxHit.GetActor())
	{	
		if (ActorIsSameType(BoxHit.GetActor()))
		{
			return;
		} 

		UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, MyOwner->GetInstigatorController(), this, UDamageType::StaticClass());
        ExecuteGetHit(BoxHit);
	}
}
bool AWeapon::ActorIsSameType(AActor *OtherActor)
{
	if (!OtherActor) return false;
	if (OtherActor == this) return true;

	AActor* WeaponOwner = GetOwner();
	APawn* WeaponInstigator = GetInstigator();
	
	if (OtherActor == WeaponOwner) return true;
	if (OtherActor == WeaponInstigator) return true;

    return false;
}
void AWeapon::ExecuteGetHit(FHitResult &BoxHit)
{
    IHitInterface *HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
    if (HitInterface)
    {
        HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
    }
}

void AWeapon::Drop(const FVector& Impulse)
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	AActor* FormerOwner = GetOwner();

	SetOwner(nullptr);
	SetInstigator(nullptr);

	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (FormerOwner)
	{
		WeaponBox->IgnoreActorWhenMoving(FormerOwner, true);
		ItemMesh->IgnoreActorWhenMoving(FormerOwner, true);
	}

	ActivateEmbers();

    //ItemState = EItemState::EIS_Hovering;

    //Re-enable pickup collision
    if (Sphere)
    {
        Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    //Enable physics so it falls
    if (ItemMesh)
    {
        ItemMesh->SetSimulatePhysics(true);
        ItemMesh->SetEnableGravity(true);
   		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ItemMesh->AddImpulse(Impulse, NAME_None, true);
    }

    //Disable weapon damage collision
    if (WeaponBox)
    {
        WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    IgnoreActors.Empty();
}