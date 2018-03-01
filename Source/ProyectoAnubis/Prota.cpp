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
	CameraBoom->TargetArmLength = 300.0f;

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
}

void AProta::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProta::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
}

void AProta::StartBlock()
{
	UE_LOG(LogTemp, Warning, TEXT("START BLOCK"));
}

void AProta::StopBlock()
{
	UE_LOG(LogTemp, Warning, TEXT("STOP BLOCK"));
}



