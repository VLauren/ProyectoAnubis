// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Movimiento.h"
#include "NormalAttackData.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Prota.generated.h"

UCLASS()
class PROYECTOANUBIS_API AProta : public APawn
{
	GENERATED_BODY()

	/** The CapsuleComponent being used for movement collision (by CharacterMovement). Always treated as being vertically aligned in simple collision check functions. */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* hitBox = nullptr;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UMovimiento* Movimiento;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this pawn's properties
	AProta();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Funcion que devuelve el componente de movimiento
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	static FVector PlayerLocation();

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
		UNormalAttackData* AttackData = nullptr;

private:

	static AProta* Instance;

	// HACK
	// TODO esto sera parte de un struct, cada ataque tendra sus datos y
	// se podra enlazar con el siguiente
	//------------------
	// int hitStart = 24;
	// int hitEnd = 44;
	// int linkStart = 40;
	// int lastFrame = 60;
	//------------------


	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Attack();
	void StartBlock();
	void StopBlock();

	bool CheckAttackStart();
	bool CheckIfLinkFrame();
	bool CheckActiveFrame();

	void StartAttack(int index);

	void DoAttack();

	void MeshRotation(float DeltaTime);

	UFUNCTION()
		void OnOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	bool attacking;

	int currentAttackFrame;
	int currentAttackIndex;
};

