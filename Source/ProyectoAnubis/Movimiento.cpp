#include "Movimiento.h"
#include "Prota.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"

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
	FVector clampedInput = ConsumeInputVector().GetClampedToMaxSize(1.0f);
	FVector movimientoEsteFrame = clampedInput * DeltaTime * VELOCIDAD;

	// Si el movimiento no es cero
	if (!movimientoEsteFrame.IsNearlyZero())
	{
		// Si estoy moviendo al personaje
		if (prota->ProtaState == EProtaState::PS_MOVING)
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
		// Si estoy atacando
		else if(prota->currentAttackFrame < 3)
		{
			FRotator TargetRotation = movimientoEsteFrame.Rotation() + StartMeshRotation;
			Mesh->SetRelativeRotation(TargetRotation);
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
		if(!KBForward)
			KBFrameMove = KBDir * DeltaTime * KBStrength;
		else
			KBFrameMove = (Mesh->RelativeRotation - StartMeshRotation).Vector() * DeltaTime * KBStrength;
		SafeMoveUpdatedComponent(KBFrameMove, UpdatedComponent->GetComponentRotation(), true, Hit);

		// Cuanto termine el tiempo, dejo de moverme por el knockback
		KBElapsedTime += DeltaTime;
		if (KBElapsedTime >= KBTime)
			KBActive = false;
	}
}

void UMovimiento::AddKnockback(float strength, float time, FVector direction, bool forward)
{
	KBActive = true;
	KBStrength = strength;
	KBTime = time;
	KBElapsedTime = 0;
	KBDir = direction.GetSafeNormal();
	KBForward = forward;
}

