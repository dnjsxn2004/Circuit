// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CircuitSpeedBuff.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CIRCUIT_API ACircuitSpeedBuff : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACircuitSpeedBuff();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(EditDefaultsOnly, Category = "SpeedBuff")
	float BonusAmout = 10.0f;

public:

	UFUNCTION()
	void OnItemOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
