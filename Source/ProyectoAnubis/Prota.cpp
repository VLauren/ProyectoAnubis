// Fill out your copyright notice in the Description page of Project Settings.

#include "Prota.h"


// Sets default values
AProta::AProta()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AProta::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProta::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AProta::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

