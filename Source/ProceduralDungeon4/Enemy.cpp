// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "HealthBarComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon.h"
#include "Math/UnrealMathUtility.h"
#include "Wisdom.h"
#include "AttributeComponent.h"
#include "Navigation/PathFollowingComponent.h"

AEnemy::AEnemy()
{
	
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

    HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("Health Bar"));
    HealthBarWidget->SetupAttachment(GetRootComponent());

    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SightRadius = 4000.f;
    PawnSensing->SetPeripheralVisionAngle(45.f);

}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if (IsDead()) return;
    if (EnemyState > EEnemyState::EES_Patrolling)
    {
        CheckCombatTarget();
    }
    else
    {
        CheckPatrolTarget();
    }

}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    HandleDamage(DamageAmount);
    CombatTarget = EventInstigator->GetPawn();
    if (IsInsideAttackRadius())
    {
        EnemyState = EEnemyState::EES_Attacking;
    }
    else if (IsOutsideAttackRadius())
    {
        ChaseTarget();
    }
    return DamageAmount;
}
void AEnemy::Destroyed()
{
    if (EquippedWeapon)
    {
        EquippedWeapon->Destroy();
    }
}
void AEnemy::PatrolTimerFinished()
{
    MoveToTarget(PatrolTarget);
}
void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
    Super::GetHit_Implementation(ImpactPoint, Hitter);
    if (!IsDead()) ShowHealthBar();
    ClearPatrolTimer();
    ClearAttackTimer();
    SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

    StopAttackMontage();
    if (IsInsideAttackRadius())
    {
       if (!IsDead()) StartAttackTimer();
    }
}
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

    if (PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
    InitializeEnemy();
}
void AEnemy::Die_Implementation()
{
    Super::Die_Implementation();
    
    EnemyState = EEnemyState::EES_Dead;
    ClearAttackTimer();
    HideHealthBar();
    UE_LOG(LogTemp, Warning, TEXT("AboutToDisableCapsule"));
    DisableCapsule();
    SetLifeSpan(DeathLifeSpan);
    GetCharacterMovement()->bOrientRotationToMovement = false;
    SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
    SpawnWisdom();
}
void AEnemy::SpawnWisdom()
{
    UWorld *World = GetWorld();
    if (World && WisdomClass && Attributes)
    {
        const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 125.f);
        AWisdom* SpawnedWisdom = World->SpawnActor<AWisdom>(WisdomClass, SpawnLocation, GetActorRotation());
        if (SpawnedWisdom)
        {
            SpawnedWisdom->SetWisdom(Attributes->GetWisdom());
            SpawnedWisdom->SetOwner(this);
        }
    }
}
void AEnemy::Attack()
{
    Super::Attack();
    if (CombatTarget == nullptr) return;
    EnemyState = EEnemyState::EES_Engaged;
    PlayAttackMontage();
}
bool AEnemy::CanAttack()
{
    EquippedWeapon;
    bool bCanAttack = 
        IsInsideAttackRadius() &&
        !IsAttacking() &&
        !IsEngaged() &&
        !IsDead();
    return bCanAttack;
}
void AEnemy::AttackEnd() 
{
    EnemyState = EEnemyState::EES_NoState;
    CheckCombatTarget();
}
void AEnemy::HandleDamage(float DamageAmount)
{
    Super::HandleDamage(DamageAmount);

    if (Attributes && HealthBarWidget)
    {
        HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
    }
}

void AEnemy::InitializeEnemy()
{
    EnemyController = Cast<AAIController>(GetController());
    Tags.Add(FName("Enemy"));
    MoveToTarget(PatrolTarget);
    HideHealthBar();
    SpawnDefaultWeapon();
    if (EquippedWeapon)
    {
        WeaponType = EquippedWeapon->GetWeaponType();
    }
}
void AEnemy::CheckPatrolTarget()
{
    if (InTargetRange(PatrolTarget, PatrolRadius))
    {
        PatrolTarget = ChoosePatrolTarget();
        const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
        GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
    }
}
void AEnemy::CheckCombatTarget()
{
    if (IsOutsideCombatRadius())
    {   
        ClearAttackTimer();
        LoseInterest();
        if (!IsEngaged()) StartPatrolling();
 
    }
    else if (IsOutsideAttackRadius() && !IsChasing())
    {
        ClearAttackTimer();
        if (!IsEngaged()) ChaseTarget();
    }
    else if (CanAttack())
    {
        StartAttackTimer();
    }
}
void AEnemy::SpawnDefaultWeapon()
{
    UWorld *World = GetWorld();
    if (World && WeaponClass)
    {
        AWeapon *DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
        DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
        EquippedWeapon = DefaultWeapon;
    }
}
void AEnemy::HideHealthBar()
{
    if (HealthBarWidget)
    {
        HealthBarWidget->SetVisibility(false);
    }
}

void AEnemy::ShowHealthBar()
{
    if (HealthBarWidget)
    {
        HealthBarWidget->SetVisibility(true);
    }
}

void AEnemy::LoseInterest()
{
    CombatTarget = nullptr;
    HideHealthBar();
}

void AEnemy::StartPatrolling()
{
    EnemyState = EEnemyState::EES_Patrolling;
    GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
    MoveToTarget(PatrolTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
    return !InTargetRange(CombatTarget, CombatRadius);
}

void AEnemy::ChaseTarget()
{
    EnemyState = EEnemyState::EES_Chasing;
    GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
    MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideAttackRadius()
{
    return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
    return EnemyState == EEnemyState::EES_Chasing;
}


bool AEnemy::IsInsideAttackRadius()
{
    return InTargetRange(CombatTarget, AttackRadius);
}


bool AEnemy::IsAttacking()
{
    return EnemyState == EEnemyState::EES_Attacking;
}

void AEnemy::StartAttackTimer()
{
    EnemyState = EEnemyState::EES_Attacking;
    const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
    GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}


void AEnemy::ClearPatrolTimer()
{
    GetWorldTimerManager().ClearTimer(PatrolTimer);
}

bool AEnemy::IsDead()
{
    return EnemyState == EEnemyState::EES_Dead;
}

void AEnemy::ClearAttackTimer()
{
    GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::IsEngaged()
{
    return EnemyState == EEnemyState::EES_Engaged;
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
    if (Target == nullptr) return false;
    const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
    return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
    if (EnemyController == nullptr || Target == nullptr) return;
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalActor(Target);
        MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
        EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
    TArray<AActor*> ValidTargets;
    for (AActor* Target : PatrolTargets)
    {
        if (Target != PatrolTarget)
        {
            ValidTargets.AddUnique(Target);
        }
    }

    const int32 NumPatrolTargets = ValidTargets.Num();
    if (NumPatrolTargets > 0)
    {
        const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
        return ValidTargets[TargetSelection];
    }

    return nullptr;
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
    if (SeenPawn->ActorHasTag(FName("Dead"))) return;

    const bool bShouldChaseTarget =
        EnemyState != EEnemyState::EES_Dead &&
        EnemyState != EEnemyState::EES_Chasing &&
        EnemyState < EEnemyState::EES_Chasing &&
        SeenPawn->ActorHasTag(FName("EngageableTarget"));

    if (bShouldChaseTarget)
    {
        CombatTarget = SeenPawn;
        ClearPatrolTimer();
        ChaseTarget();
    }
}

void AEnemy::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
    if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
    {
    	EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
    	EquippedWeapon->IgnoreActors.Empty();
    }
}

