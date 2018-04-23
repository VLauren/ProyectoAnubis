#include "Prota.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/TimerManager.h"
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

	// CapsuleComponent->SetVisibility(true);
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

	// Datos de ataque
	static ConstructorHelpers::FObjectFinder<UNormalAttackData> AttackDataObj(TEXT("/Game/Datos/PlayerAttacks"));
	AttackData = AttackDataObj.Object;

	Instance = this;

	HitPoints = 100;
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
		// hitBox->SetVisibility(false);
		hitBox->SetHiddenInGame(true);

		// evento de overlap del hitbox
		hitBox->OnComponentBeginOverlap.AddDynamic(this, &AProta::OnHitboxOverlap);
	}

	// Mesh->PlayAnimation()
	AnimState = EProtaAnimState::AS_STAND;
	Mesh->PlayAnimation(AnimStand, true);

	ProtaState = EProtaState::PS_MOVING;

	StartMeshRotation = Mesh->RelativeRotation;
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

void AProta::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoAttack();

	// Log de vida restante
	if (GEngine)
	{
		FString msg = FString::Printf(TEXT("Vida: %d - CanMove:%s"), HitPoints, (ProtaState == EProtaState::PS_MOVING ? TEXT("true") : TEXT("false")));
		GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White, msg);
	}

	if (ProtaState == EProtaState::PS_HITSTUN)
	{
		if (hitStuntCount >= 20)
		{
			AnimState = EProtaAnimState::AS_STAND;
			Mesh->PlayAnimation(AnimStand, true);
			ProtaState = EProtaState::PS_MOVING;
		}
		else if(AnimState != EProtaAnimState::AS_HIT)
		{
			AnimState = EProtaAnimState::AS_HIT;
			Mesh->PlayAnimation(AnimHitstun, false);
		}

		hitStuntCount++;
	}
}

void AProta::MoveForward(float AxisValue)
{
	// si estoy atacando, no me muevo
	if (ProtaState != EProtaState::PS_MOVING)
		return;

	// Mesh->PlayAnimation(AnimRun, true);

	if (Movimiento && (Movimiento->UpdatedComponent == RootComponent))
		Movimiento->AddInputVector(FRotator(0,GetControlRotation().Yaw,0).RotateVector(GetActorForwardVector()) * AxisValue);
}

void AProta::MoveRight(float AxisValue)
{
	// si estoy atacando, no me muevo
	if (ProtaState != EProtaState::PS_MOVING)
		return;

	// Mesh->PlayAnimation(AnimRun, true);

	if (Movimiento && (Movimiento->UpdatedComponent == RootComponent))
		Movimiento->AddInputVector(FRotator(0,GetControlRotation().Yaw,0).RotateVector(GetActorRightVector()) * AxisValue);
}

void AProta::Attack()
{
	if (CheckAttackStart())
	{
		StartAttack(0);
	}
	else if (ProtaState != EProtaState::PS_HITSTUN && CheckIfLinkFrame())
	{
		// StartAttack(currentAttackIndex + 1);
		linkAttack = true;

		UE_LOG(LogTemp, Warning, TEXT("LINK! %d"), (currentAttackIndex + 1));
	}
}

void AProta::StartBlock()
{
	// Si estoy en estado neutral
	if (ProtaState == EProtaState::PS_MOVING)
	{
		// Cambio a estado de defensa y cambio la animacion
		AnimState = EProtaAnimState::AS_BLOCK;
		Mesh->PlayAnimation(AnimBlock, false);
		ProtaState = EProtaState::PS_BLOCK;
	}

}

void AProta::StopBlock()
{
	// Al soltar el boton de defensa, si estoy defendiendo
	if (ProtaState == EProtaState::PS_BLOCK)
	{
		// Cambio a estado neutral
		AnimState = EProtaAnimState::AS_STAND;
		Mesh->PlayAnimation(AnimStand, false);
		ProtaState = EProtaState::PS_MOVING;
	}
}
	
// Comprueba si el personaje está en un estado correcto para iniciar un ataque
bool AProta::CheckAttackStart()
{
	// Solo puedo iniciar un ataque desde el estado neutral
	return ProtaState == EProtaState::PS_MOVING;
}

// Comprueba si el personaje puede inicar un ataque desde del actual
bool AProta::CheckIfLinkFrame()
{
	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO HAY DATOS DE ATAQUE"));
		return false;
	}
	bool hasNextAttack = currentAttackIndex < (AttackData->Attacks.Num() - 1);

	return ProtaState == EProtaState::PS_ATTACKING && currentAttackFrame >= AttackData->Attacks[currentAttackIndex].linkStart && hasNextAttack;
}

// Comprueba si estoy en un frame activo de daño
bool AProta::CheckActiveFrame()
{
	return currentAttackFrame >= AttackData->Attacks[currentAttackIndex].hitStart && currentAttackFrame <= AttackData->Attacks[currentAttackIndex].hitEnd;
}

void AProta::StartAttack(int index)
{
	linkAttack = false;

	if (AttackData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO HAY DATOS DE ATAQUE"));
		return;
	}

	// StartAttackAnimation(index);
	ProtaState = EProtaState::PS_ATTACKING;
	currentAttackFrame = 0;
	currentAttackIndex = index;

	// Inicio animacion
	if (currentAttackIndex == 0)
		Mesh->PlayAnimation(AnimAttack1, false);
	if (currentAttackIndex == 1)
		Mesh->PlayAnimation(AnimAttack2, false);
	if (currentAttackIndex == 2)
		Mesh->PlayAnimation(AnimAttack3, false);
	AnimState = EProtaAnimState::AS_ATTACK;

	// Mesh->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage();
	// https://docs.unrealengine.com/en-us/Engine/Animation/AnimationComposite

	// TODO avanzar con cada ataque con timer y valores hardcodeados
	// GetWorldTimerManager().SetTimer(TimerHandle, this, lafun)
	AttackMove(1, 0.5f);
}

void AProta::AttackMove(float amount, float time)
{
	// O ALGO
	Movimiento->AddKnockback(500, 0.07f, (Mesh->RelativeRotation - StartMeshRotation).Vector());
}


// Se ejecuta cada frame. Gestiona los ataques del prota.
void AProta::DoAttack()
{
	if (ProtaState == EProtaState::PS_ATTACKING)
	{
		// UE_LOG(LogTemp, Warning, TEXT("frame de ataque: %d - %s"), currentAttackFrame, (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
		if (GEngine)
		{
			FString msg = FString::Printf(TEXT("ATAQUE: %d - frame: %d - activo: %s"), currentAttackIndex, currentAttackFrame, (CheckActiveFrame() ? TEXT("true") : TEXT("false")));
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, msg);
		}

		currentAttackFrame++;

		if (hitBox != nullptr)
		{
			if (CheckActiveFrame())
			{
				hitBox->bGenerateOverlapEvents = true;
				// hitBox->SetVisibility(true);
				hitBox->SetHiddenInGame(false);
			}
			else
			{
				hitBox->bGenerateOverlapEvents = false;
				// hitBox->SetVisibility(false);
				hitBox->SetHiddenInGame(true);
			}
		}

		// compruebo si ha terminado el ataque
		if (currentAttackFrame >= AttackData->Attacks[currentAttackIndex].lastFrame)
		{
			if (linkAttack)
			{
				// lanzo el siguiente ataque
				StartAttack(currentAttackIndex + 1);
			}
			else
			{
				// Paro la animacion de ataque
				AnimState = EProtaAnimState::AS_STAND;
				Mesh->PlayAnimation(AnimStand, false);

				ProtaState = EProtaState::PS_MOVING;
			}
		}
	}
}

void AProta::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Hitbox overlap! %s"), OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>() ? TEXT("ES ENEMIGO") : TEXT("no es enemigo"));

	if (OtherComp != nullptr)
	{
		// Si es enemigo, le hago daño
		if(OtherComp->GetOwner()->GetClass()->IsChildOf<AEnemy>())
			((AEnemy*)OtherComp->GetOwner())->Damage(10, GetActorLocation());
	}
}

FVector AProta::PlayerLocation()
{
	return Instance->GetActorLocation();
}

void AProta::Damage(int amount, FVector sourcePoint, bool unblockable)
{
	// Si no estoy defendiendome
	if (unblockable || ProtaState != EProtaState::PS_BLOCK)
	{
		HitPoints -= amount;
		if (HitPoints <= 0)
		{
			// TODO hacer pantalla + que cargue la pantalla buena

			// HACK por ahora cuando muero: recargo la pantalla
			// UGameplayStatics::OpenLevel(this, *GetWorld()->GetMapName());
			UGameplayStatics::OpenLevel(this, TEXT("/Game/Mapas/Mapa3"));
		}


		// TODO esto en funcion de hitstun

		// Animacion de hit stun
		AnimState = EProtaAnimState::AS_HIT;
		Mesh->PlayAnimation(AnimHitstun, false);

		hitStuntCount = 0;
		ProtaState = EProtaState::PS_HITSTUN;

		// Knockback
		FVector kbDirection = GetActorLocation() - sourcePoint;
		kbDirection.Z = 0;
		kbDirection.Normalize();
		Movimiento->AddKnockback(700, 0.1f, kbDirection);

		// FTimerDelegate TimerCallback;
		// TimerCallback.BindLambda([this]
		// {
			// Tras el hit stun, vuelvo a neutral
			// TODO usar Tick en lugar de timer para esto

			// ProtaState = EProtaState::PS_MOVING;
			// if (Mesh != nullptr)
			// {
				// Mesh->PlayAnimation(AnimStand, true);
			// }
			// UE_LOG(LogTemp, Warning, TEXT("Paco"));
		// });

		// FTimerHandle Handle;
		// GetWorldTimerManager().SetTimer(Handle, TimerCallback, 0.5f, false);
	}
}

UPawnMovementComponent * AProta::GetMovementComponent() const
{
	return Movimiento;
}







