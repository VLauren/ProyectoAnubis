// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Enemy.generated.h"

UENUM()
enum class EEnemyState : uint8
{
	ES_WAIT			UMETA(DisplayName="Wait"),
	ES_CHASING		UMETA(DisplayName="Chasing"),
	ES_ATTACKING	UMETA(DisplayName="Attacking"),
	ES_HITSTUN			UMETA(DisplayName="Hitstun")
};

UCLASS()
class PROYECTOANUBIS_API AEnemy : public APawn
{
	GENERATED_BODY()

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* HitBox = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UEnemyMovement* Movement;

	UPROPERTY(VisibleAnywhere)		class UParticleSystemComponent* PSC;

protected:

public:
	// Sets default values for this pawn's properties
	AEnemy();

	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Attack();

	void ActivateHitbox();
	void EndAttack();

	void Damage(int amount, FVector sourcePoint, float knockback = 0);
	
	FRotator StartMeshRotation;
	FRotator CurrentRotation;

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;

	UFUNCTION()
		void OnHitboxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UPROPERTY(EditAnywhere, Category=Enum)
		EEnemyState EnemyState;

	FTimerHandle TimerHandle;

private:

	// Vida restante
	int HitPoints;

	int hitStuntCount;

};
