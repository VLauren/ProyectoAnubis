#include "Enemy.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Prota.h"

/* TODO
* - tomar referencia al prota y funcion que devuelva la posicion
* - funcion de recibir daño
*	- Test con log y con espera
* - Morir
* - colision de arma -> recibir daño (en casita)
* - AIController, como va eso
*/

AEnemy::AEnemy()
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

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlap);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorLocalOffset(DeltaTime * ((AProta::PlayerLocation() - GetActorLocation()).GetSafeNormal()) * 80);
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	// if (OverlappedComponent != nullptr)
		// UE_LOG(LogTemp, Warning, TEXT("ENEMY OVERLAPPP: %s"), *OtherComp->GetName())
}

void AEnemy::Damage(int amount)
{
	UE_LOG(LogTemp, Warning, TEXT("ENEMY DAMAGE: %d"), amount)

	// HACK
	Destroy();
}
