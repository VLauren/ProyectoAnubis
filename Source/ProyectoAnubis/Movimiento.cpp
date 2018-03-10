// Fill out your copyright notice in the Description page of Project Settings.
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
	}
}

