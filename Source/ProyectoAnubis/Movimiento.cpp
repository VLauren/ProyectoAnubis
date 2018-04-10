#include "Movimiento.h"
#include "Prota.h"

const float VELOCIDAD = 500.0f;

void UMovimiento::BeginPlay()
{
	Super::BeginPlay();

	Mesh = ((AProta*)GetOwner())->Mesh;

	StartMeshRotation = Mesh->RelativeRotation;
}

void UMovimiento::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
		return;

	AProta* prota = (AProta*)GetPawnOwner();

	// Calculo el vector de movimiento
	FVector movimientoEsteFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * VELOCIDAD;

	// Si el movimiento no es cero
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

		// Si el estado de animacion es diferente de corriendo, inicio la animacion de correr
		if (prota->AnimState != EProtaAnimState::AS_RUN)
		{
			Mesh->PlayAnimation(prota->AnimRun, true);
			prota->AnimState = EProtaAnimState::AS_RUN;
		}
	}
	else if (prota->AnimState == EProtaAnimState::AS_RUN)
	{
		Mesh->PlayAnimation(prota->AnimStand, true);
		prota->AnimState = EProtaAnimState::AS_STAND;
	}

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

void UMovimiento::AddKnockback(float strength, float time, FVector direction)
{
	KBActive = true;
	KBStrength = strength;
	KBTime = time;
	KBElapsedTime = 0;
	KBDir = direction.GetSafeNormal();
}

