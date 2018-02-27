// Fill out your copyright notice in the Description page of Project Settings.
// probando git (spidey)
#include "Movimiento.h"

const float VELOCIDAD = 300.0f;

void UMovimiento::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
		return;

	// Calculo el vector de movimiento
	FVector movimientoEsteFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * VELOCIDAD;

	//TODO variable de velocidad

	// Si el movimiento no es cero
	if (!movimientoEsteFrame.IsNearlyZero())
	{
		FHitResult Hit;

		// Movimiento
		SafeMoveUpdatedComponent(movimientoEsteFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

		// Si chocamos con algo, me deslizo sobre el
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(movimientoEsteFrame, 1.f - Hit.Time, Hit.Normal, Hit);
	}
}

