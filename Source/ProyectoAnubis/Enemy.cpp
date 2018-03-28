#include "Enemy.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Prota.h"

/* TODO
* - AIController, como va eso
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

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlap);

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
	if (EnemyState == EEnemyState::ES_CHASING)
	{
		// Movimiento
		direccion = (AProta::PlayerLocation() - GetActorLocation()).GetSafeNormal();
		AddActorLocalOffset(DeltaTime * direccion * 80);

		if (FVector::Dist(AProta::PlayerLocation(), GetActorLocation()) <= 150)
			Attack();
	}

	if (EnemyState != EEnemyState::ES_ATTACKING)
	{
		// Rotacion del modelo
		FRotator TargetRotation = direccion.Rotation() + StartMeshRotation;
		CurrentRotation = FMath::Lerp(CurrentRotation, TargetRotation, 0.1f);
		Mesh->SetRelativeRotation(CurrentRotation);
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

	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::EndAttack, 0.5f, false);
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

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	// if (OverlappedComponent != nullptr)
		// UE_LOG(LogTemp, Warning, TEXT("ENEMY OVERLAPPP: %s"), *OtherComp->GetName())
}

void AEnemy::Damage(int amount)
{
	UE_LOG(LogTemp, Warning, TEXT("ENEMY DAMAGE: %d"), amount)

	// HACK
	Destroy();
}

void AEnemy::OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherComp != nullptr)
	{
		// Si es jugador, le hago daño
		if (OtherComp->GetOwner()->GetClass()->IsChildOf<AProta>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemigo Golpea"))
			((AProta*)OtherComp->GetOwner())->Damage(10);
		}
	}
}

