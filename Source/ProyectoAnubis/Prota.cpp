#include "Prota.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AProta::AProta()
{
	PrimaryActorTick.bCanEverTick = true;

	// Creo la capsula de colision del personaje
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsula"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName("Felipe");
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->bShouldUpdatePhysicsVolume = true;
	CapsuleComponent->bCheckAsyncSceneOnMove = false;
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);

	// Agrero el spring arm para la camara
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 700.0f;

	// EL BRAZO ROTA SEGUN EL INPUT QUE LE LLEGA AL PEON
	CameraBoom->bUsePawnControlRotation = true; 

	// Agrego la camara
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	// LA CAMARA EN SI NO ROTA SEGUN EL INPUT QUE LE LLEGA AL PEON
	FollowCamera->bUsePawnControlRotation = false;

	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(CapsuleComponent);
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->bGenerateOverlapEvents = false;
		Mesh->SetCanEverAffectNavigation(false);
	}

	// Componente de movimiento
	Movimiento = CreateDefaultSubobject<UMovimiento>(TEXT("Movimiento"));
	Movimiento->UpdatedComponent = RootComponent;

	// HACK Hitbox provisional
	hitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
	if (hitBox)
	{
		hitBox->InitBoxExtent(FVector(30, 30, 30));
		hitBox->SetVisibility(true);
		hitBox->SetHiddenInGame(false);
		// hitBox->SetRelativeLocation(FVector(3, 3, 3));
	}

}

void AProta::BeginPlay()
{
	Super::BeginPlay();

	hitBox->SetVisibility(false);
	hitBox->SetHiddenInGame(true);
}

void AProta::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoAttack();
}

void AProta::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// input de camara
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// input de movimiento (controller)
	PlayerInputComponent->BindAxis("MoveRight", this, &AProta::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AProta::MoveForward);

	// input de acciones
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AProta::Attack);
	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AProta::StartBlock);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AProta::StopBlock);
}

UPawnMovementComponent * AProta::GetMovementComponent() const
{
	return Movimiento;
}

void AProta::MoveForward(float AxisValue)
{
	if (Movimiento && (Movimiento->UpdatedComponent == RootComponent))
		Movimiento->AddInputVector(FRotator(0,GetControlRotation().Yaw,0).RotateVector(GetActorForwardVector()) * AxisValue);
}

void AProta::MoveRight(float AxisValue)
{
	if (Movimiento && (Movimiento->UpdatedComponent == RootComponent))
		Movimiento->AddInputVector(FRotator(0,GetControlRotation().Yaw,0).RotateVector(GetActorRightVector()) * AxisValue);
}

void AProta::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("ATTACK"));

	if (CheckAttackStart())
	{
		StartAttack(1);
		hitBox->SetVisibility(true);
		hitBox->SetHiddenInGame(false);
	}
}

void AProta::StartBlock()
{
	UE_LOG(LogTemp, Warning, TEXT("START BLOCK"));
}

void AProta::StopBlock()
{
	UE_LOG(LogTemp, Warning, TEXT("STOP BLOCK"));
}

bool AProta::CheckAttackStart()
{
	return true;
}

bool AProta::CheckIfLinkFrame()
{
	return true;
}

bool AProta::CheckActiveFrame()
{
	return true;
}

void AProta::StartAttack(int index)
{
	// StartAttackAnimation(index);
	attacking = true;
	currentAttackFrame = 0;
	currentAttackIndex = index;
}

void AProta::DoAttack()
{
	if (attacking)
	{
		UE_LOG(LogTemp, Warning, TEXT("frame de ataque: %d"), currentAttackFrame);

		currentAttackFrame++;

		if (CheckActiveFrame())
			hitBox->bGenerateOverlapEvents = true;
		else
			hitBox->bGenerateOverlapEvents = false;

		if (currentAttackFrame >= lastFrame)
		{
			attacking = false;
			hitBox->SetVisibility(false);
			hitBox->SetHiddenInGame(true);
		}
	}

	/*
	// hitbox
	if (CheckFrameActivo())
		hitBox->bGenerateOverlapEvents = true;
	else
		hitBox->bGenerateOverlapEvents = false;

	// fin de ataque
	if (CheckAttackEnd())
	{
		attacking = false;
		StopAttackAnimation();
	}
	*/
}












