#include "Enemy.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Prota.h"
#include "EnemyMovement.h"

/*
* TODO agregar animaciones
* TODO mover enemigo con componente de mov
* - TODO Spawner de enemigo basico
* - TODO AIController, como va eso
*/

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Creo la capsula de colision del personaje
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsula"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName("Felipe");
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->bShouldUpdatePhysicsVolume = true;
	CapsuleComponent->bCheckAsyncSceneOnMove = false;
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);

	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(CapsuleComponent);
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->bGenerateOverlapEvents = false;
		Mesh->SetCanEverAffectNavigation(false);
	}

	// Componente de movimiento
	Movement = CreateDefaultSubobject<UEnemyMovement>(TEXT("Movement"));
	Movement->UpdatedComponent = RootComponent;

	HitPoints = 100;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	StartMeshRotation = Mesh->RelativeRotation;
	// CurrentRotation = Mesh->RelativeRotation;

	EnemyState = EEnemyState::ES_CHASING;

	HitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());
	if (HitBox != nullptr)
	{
		HitBox->bGenerateOverlapEvents = false;
		HitBox->SetVisibility(false);
		HitBox->SetHiddenInGame(true);

		// evento de overlap del hitbox
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnHitboxOverlap);
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector direccion;

	// Si estoy persiguiendo
	if (EnemyState == EEnemyState::ES_CHASING)
	{
		// Movimiento
		// direccion = (AProta::PlayerLocation() - GetActorLocation()).GetSafeNormal();
		// AddActorLocalOffset(DeltaTime * direccion * 80);

		if (FVector::Dist(AProta::PlayerLocation(), GetActorLocation()) <= 150)
			Attack();
	}

	// Siempre que no este atacando
	// if (EnemyState != EEnemyState::ES_ATTACKING)
	// {
		// oriento el modelo hacia el jugador
		// FRotator TargetRotation = direccion.Rotation() + StartMeshRotation;
		// CurrentRotation = FMath::Lerp(CurrentRotation, TargetRotation, 0.1f);
		// Mesh->SetRelativeRotation(CurrentRotation);
	// }


	if (EnemyState == EEnemyState::ES_HITSTUN)
	{
		if (hitStuntCount >= 20)
		{
			// AnimState = EProtaAnimState::AS_STAND;
			// Mesh->PlayAnimation(AnimStand, true);
			EnemyState = EEnemyState::ES_CHASING;
		}
		// else if(AnimState != EProtaAnimState::AS_HIT)
		// {
			// AnimState = EProtaAnimState::AS_HIT;
			// Mesh->PlayAnimation(AnimHitstun, false);
		// }

		hitStuntCount++;
	}
}

void AEnemy::Attack()
{
	EnemyState = EEnemyState::ES_ATTACKING;

	// TODO iniciar animacion ataque

	if (HitBox != nullptr)
	{
		HitBox->bGenerateOverlapEvents = true;
		HitBox->SetVisibility(true);
		HitBox->SetHiddenInGame(false);
	}

	FTimerHandle th;
	// GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::ActivateHitbox, 0.75f, false);
	GetWorldTimerManager().SetTimer(th, this, &AEnemy::EndAttack, 1.0f, false);
}

void AEnemy::ActivateHitbox()
{
	if (HitBox != nullptr)
	{
		HitBox->bGenerateOverlapEvents = true;
		HitBox->SetVisibility(true);
		HitBox->SetHiddenInGame(false);
	}
}

void AEnemy::EndAttack()
{
	EnemyState = EEnemyState::ES_CHASING;

	if (HitBox != nullptr)
	{
		HitBox->bGenerateOverlapEvents = false;
		HitBox->SetVisibility(false);
		HitBox->SetHiddenInGame(true);
	}

}

void AEnemy::Damage(int amount, FVector sourcePoint)
{
	// UE_LOG(LogTemp, Warning, TEXT("ENEMY DAMAGE: %d"), amount)

	hitStuntCount = 0;
	EnemyState = EEnemyState::ES_HITSTUN;

	// TODO animacion de hit stun + timer de volver a chasing
	// TODO origen del damage para knockback

	// Knockback
	FVector kbDirection = GetActorLocation() - sourcePoint;
	kbDirection.Z = 0;
	kbDirection.Normalize();
	Movement->AddKnockback(500, 0.07f, kbDirection);

	HitPoints -= amount;
	if (HitPoints <= 0)
	{
		Destroy();
		// TODO animacion de muerte + estado muriendo + cancelar colision + timer Destroy
	}
}

void AEnemy::OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherComp != nullptr)
	{
		// Si es jugador, le hago daño
		if (OtherComp->GetOwner()->GetClass()->IsChildOf<AProta>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemigo Golpea"))
			((AProta*)OtherComp->GetOwner())->Damage(10, OverlappedComponent->GetOwner()->GetActorLocation());

			if (HitBox != nullptr)
			{
				HitBox->bGenerateOverlapEvents = false;
				HitBox->SetVisibility(false);
				HitBox->SetHiddenInGame(true);
			}
		}
	}
}

