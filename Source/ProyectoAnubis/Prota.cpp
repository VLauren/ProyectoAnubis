#include "Prota.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Enemy.h"

AProta* AProta::Instance;

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

	Instance = this;
}

void AProta::BeginPlay()
{
	Super::BeginPlay();

	// hitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
	TArray<UObject*> arr;
	Mesh->GetDefaultSubobjects(arr);
	for (UObject* obj : arr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Subobjeto: %s"), *obj->GetName())
	}
	
	hitBox = (UBoxComponent*)GetComponentByClass(UBoxComponent::StaticClass());

	// hitBox = (UBoxComponent*)(Mesh->GetDefaultSubobjectByName(TEXT("Hit")));
	if (hitBox != nullptr)
	{
		hitBox->bGenerateOverlapEvents = false;
		hitBox->SetVisibility(false);
		hitBox->SetHiddenInGame(true);

		// evento de overlap del hitbox
		hitBox->OnComponentBeginOverlap.AddDynamic(this, &AProta::OnOverlap);
	}
}

void AProta::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoAttack();

	MeshRotation(DeltaTime);
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
	// si estoy atacando, no me muevo
	if (attacking)
		return;

	if (Movimiento && (Movimiento->UpdatedComponent == RootComponent))
		Movimiento->AddInputVector(FRotator(0,GetControlRotation().Yaw,0).RotateVector(GetActorForwardVector()) * AxisValue);
}

void AProta::MoveRight(float AxisValue)
{
	// si estoy atacando, no me muevo
	if (attacking)
		return;

	if (Movimiento && (Movimiento->UpdatedComponent == RootComponent))
		Movimiento->AddInputVector(FRotator(0,GetControlRotation().Yaw,0).RotateVector(GetActorRightVector()) * AxisValue);
}

void AProta::Attack()
{
	// UE_LOG(LogTemp, Warning, TEXT("ATTACK"));

	if (CheckAttackStart())
	{
		StartAttack(1);
		// hitBox->SetVisibility(true);
		// hitBox->SetHiddenInGame(false);
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
	
// Comprueba si el personaje está en un estado correcto para iniciar un ataque
bool AProta::CheckAttackStart()
{
	// HACK por ahora puedo inicar un ataque si no estoy atacando
	return !attacking;
}

// Comprueba si el personaje puede inicar un ataque desde del actual
bool AProta::CheckIfLinkFrame()
{
	return attacking && currentAttackFrame >= linkStart;
}

// Comprueba si estoy en un frame activo de daño
bool AProta::CheckActiveFrame()
{
	return currentAttackFrame >= hitStart && currentAttackFrame <= hitEnd;
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
		// UE_LOG(LogTemp, Warning, TEXT("frame de ataque: %d - %s"), currentAttackFrame, (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
		if (GEngine)
		{
			FString msg = FString::Printf(TEXT("frame de ataque: %d - %s"), currentAttackFrame, (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, msg);
		}

		currentAttackFrame++;

		if (hitBox != nullptr)
		{
			if (CheckActiveFrame())
			{
				hitBox->bGenerateOverlapEvents = true;
				hitBox->SetVisibility(true);
				hitBox->SetHiddenInGame(false);
			}
			else
			{
				hitBox->bGenerateOverlapEvents = false;
				hitBox->SetVisibility(false);
				hitBox->SetHiddenInGame(true);
			}
		}

		if (currentAttackFrame >= lastFrame)
		{
			// TODO parar la animacion de ataque

			attacking = false;
		}
	}
}

void AProta::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Hitbox overlap! %s"), OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>() ? TEXT("ES ENEMIGO") : TEXT("no es enemigo"));

	if (OtherComp != nullptr)
	{
		// Si es enemigo, le hago daño
		if(OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>())
			((AEnemy*)OtherComp->GetOwner())->Damage(10);
	}
}

FVector AProta::PlayerLocation()
{
	return Instance->GetActorLocation();
}

void AProta::MeshRotation(float DeltaTime)
{
	// Mesh->SetRelativeRotation(RootComponent->GetComponentRotation());

	// Mesh->AddLocalRotation(FRotator(0, DeltaTime * 90, 0));
}









