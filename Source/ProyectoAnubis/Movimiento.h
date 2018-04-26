#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Movimiento.generated.h"

UCLASS()
class PROYECTOANUBIS_API UMovimiento : public UPawnMovementComponent
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void AddKnockback(float strength, float time, FVector direction, bool forward = false);

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
	bool KBForward;
};
