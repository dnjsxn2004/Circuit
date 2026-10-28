// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "CircuitItemSpawner.generated.h"

UCLASS()
class CIRCUIT_API ACircuitItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACircuitItemSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, Category = "Item Spawn")
	float RespawnDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn")
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedItem;

	FTimerHandle RespawnTimerHandle;

	void SpawnItem();

	UFUNCTION()
	void OnItemDestroyed(AActor* DestroyedActor);
};
