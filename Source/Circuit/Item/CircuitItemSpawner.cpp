// Fill out your copyright notice in the Description page of Project Settings.


#include "CircuitItemSpawner.h"
#include "Engine/World.h"

// Sets default values
ACircuitItemSpawner::ACircuitItemSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;

}

// Called when the game starts or when spawned
void ACircuitItemSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	SpawnItem();
	
}

// Called every frame
void ACircuitItemSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACircuitItemSpawner::SpawnItem()
{
	if (!HasAuthority() || !IsValid(GetWorld()) || !ItemClass || IsValid(SpawnedItem))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewItem = GetWorld()->SpawnActor<AActor>(
		ItemClass,
		GetActorTransform(),
		SpawnParams
	);

	if (!IsValid(NewItem))
	{
		return;
	}

	SpawnedItem = NewItem;

	SpawnedItem->OnDestroyed.AddDynamic(
		this,
		&ThisClass::OnItemDestroyed
	);
}

void ACircuitItemSpawner::OnItemDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority() || DestroyedActor != SpawnedItem.Get())
	{
		return;
	}

	SpawnedItem = nullptr;

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&ThisClass::SpawnItem,
		RespawnDelay,
		false
	);
}

void ACircuitItemSpawner::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	if (IsValid(SpawnedItem))
	{
		SpawnedItem->OnDestroyed.RemoveDynamic(
			this,
			&ThisClass::OnItemDestroyed
		);
	}

	Super::EndPlay(EndPlayReason);
}

