// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CircuitCoin.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CIRCUIT_API ACircuitCoin : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACircuitCoin();

	UFUNCTION()
	void OnItemOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Coin")
	int32 CoinAmount = 10;
};
