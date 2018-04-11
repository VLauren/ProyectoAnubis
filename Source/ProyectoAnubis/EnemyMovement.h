// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "EnemyMovement.generated.h"

/**
 * 
 */
UCLASS()
class PROYECTOANUBIS_API UEnemyMovement : public UPawnMovementComponent
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void AddKnockback(float strength, float time, FVector direction);

private:
	class USkeletalMeshComponent* Mesh;

	FRotator StartMeshRotation;
	FRotator CurrentRotation;

	// Variables para knockback
	FVector KBDir;
	float KBStrength;
	float KBTime;
	float KBActive;
	float KBElapsedTime;
	
	
	
};
