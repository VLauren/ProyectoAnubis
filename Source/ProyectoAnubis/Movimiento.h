// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Movimiento.generated.h"

/**
 * 
 */
UCLASS()
class PROYECTOANUBIS_API UMovimiento : public UPawnMovementComponent
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	class USkeletalMeshComponent* Mesh;

	FRotator StartMeshRotation;
	FRotator CurrentRotation;
};
