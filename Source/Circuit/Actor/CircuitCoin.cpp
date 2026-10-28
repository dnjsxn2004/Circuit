// Fill out your copyright notice in the Description page of Project Settings.


#include "CircuitCoin.h"
#include "Circuit/Vehicle/CircuitPawn.h"
#include "Circuit/Player/CircuitPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Game/CIRMatchGameState.h"

// Sets default values
ACircuitCoin::ACircuitCoin()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetNetUpdateFrequency(1.0f);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));

	SetRootComponent(CollisionSphere);

	CollisionSphere->InitSphereRadius(180.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(CollisionSphere);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&ThisClass::OnItemOverlap
	);
}


void ACircuitCoin::OnItemOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	ACircuitPawn* Vehicle = Cast<ACircuitPawn>(OtherActor);

	if (!IsValid(Vehicle))
	{
		return;
	}

	ACircuitPlayerState* PS = Vehicle->GetPlayerState<ACircuitPlayerState>();

	if (!IsValid(PS))
	{
		return;
	}

	PS->AddCoins(CoinAmount);

	if (ACIRMatchGameState* GS = GetWorld()->GetGameState<ACIRMatchGameState>())
	{
		GS->MulticastPlayPickupEffect(
			GetActorLocation(),
			false
		);
	}

	Destroy();
}

