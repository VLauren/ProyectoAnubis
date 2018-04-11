
#include "EnemyMovement.h"
#include "Enemy.h"
#include "Prota.h"


const float VELOCIDAD = 80;

void UEnemyMovement::BeginPlay()
{
	Super::BeginPlay();
	Mesh = ((AEnemy*)GetPawnOwner())->Mesh;
	StartMeshRotation = Mesh->RelativeRotation;
}

void UEnemyMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
		return;
		
	AEnemy* enemy = (AEnemy*)GetPawnOwner();


	// Si estoy persiguiendo
	if (enemy->EnemyState == EEnemyState::ES_CHASING)
	{
		// Movimiento
		FVector direccion;
		direccion = (AProta::PlayerLocation() - GetActorLocation()).GetSafeNormal();
		FVector movimientoEsteFrame = direccion * DeltaTime * VELOCIDAD;


		if (!movimientoEsteFrame.IsNearlyZero())
		{
			FHitResult Hit;

			// Movimiento
			SafeMoveUpdatedComponent(movimientoEsteFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

			// Si chocamos con algo, me deslizo sobre el
			if (Hit.IsValidBlockingHit())
				SlideAlongSurface(movimientoEsteFrame, 1.f - Hit.Time, Hit.Normal, Hit);

			// Rotacion de la malla
			FRotator TargetRotation = movimientoEsteFrame.Rotation() + StartMeshRotation;
			CurrentRotation = FMath::Lerp(CurrentRotation, TargetRotation, 0.1f);
			Mesh->SetRelativeRotation(CurrentRotation);

			// TODO animacion de correr

			// Si el estado de animacion es diferente de corriendo, inicio la animacion de correr
			// if (prota->AnimState != EProtaAnimState::AS_RUN)
			// {
				// Mesh->PlayAnimation(prota->AnimRun, true);
				// prota->AnimState = EProtaAnimState::AS_RUN;
			// }
		}

		// if (FVector::Dist(AProta::PlayerLocation(), GetActorLocation()) <= 150)
			// Attack();
	}

	// Siempre que no este atacando
	// if (EnemyState != EEnemyState::ES_ATTACKING)
	// {
		// oriento el modelo hacia el jugador
		// FRotator TargetRotation = direccion.Rotation() + StartMeshRotation;
		// CurrentRotation = FMath::Lerp(CurrentRotation, TargetRotation, 0.1f);
		// Mesh->SetRelativeRotation(CurrentRotation);
	// }

	if (KBActive)
	{
		FHitResult Hit;
		FVector KBFrameMove;
		KBFrameMove = KBDir * DeltaTime * KBStrength;
		SafeMoveUpdatedComponent(KBFrameMove, UpdatedComponent->GetComponentRotation(), true, Hit);

		// Cuanto termine el tiempo, dejo de moverme por el knockback
		KBElapsedTime += DeltaTime;
		if (KBElapsedTime >= KBTime)
			KBActive = false;
	}
}

void UEnemyMovement::AddKnockback(float strength, float time, FVector direction)
{
	KBActive = true;
	KBStrength = strength;
	KBTime = time;
	KBElapsedTime = 0;
	KBDir = direction.GetSafeNormal();
}




