// Fill out your copyright notice in the Description page of Project Settings.


#include "CIRMatchGameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ACIRMatchGameState::ACIRMatchGameState()
{
	bReplicates = true;
}

void ACIRMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MatchPhase);
	DOREPLIFETIME(ThisClass, RemainingTime);
	DOREPLIFETIME(ThisClass, Winners);
}

void ACIRMatchGameState::SetMatchPhase(ECIRMatchPhase NewPhase)
{
	if (!HasAuthority() || MatchPhase == NewPhase)
	{
		return;
	}

	MatchPhase = NewPhase;
	OnMatchDataChanged.Broadcast();
}

void ACIRMatchGameState::SetRemainingTime(int32 NewTime)
{
	if (!HasAuthority())
	{
		return;
	}

	NewTime = FMath::Max(0, NewTime);

	if (RemainingTime == NewTime)
	{
		return;
	}

	RemainingTime = NewTime;
	OnMatchDataChanged.Broadcast();
}

void ACIRMatchGameState::SetWinners(const TArray<APlayerState*>& NewWinners)
{
	if (!HasAuthority())
	{
		return;
	}

	Winners.Reset();

	for (APlayerState* PS : NewWinners)
	{
		if (IsValid(PS))
		{
			Winners.AddUnique(PS);
		}
	}

	OnMatchDataChanged.Broadcast();
}

void ACIRMatchGameState::OnRep_MatchPhase()
{
	OnMatchDataChanged.Broadcast();
}

void ACIRMatchGameState::OnRep_RemainingTime()
{
	OnMatchDataChanged.Broadcast();
}

void ACIRMatchGameState::OnRep_Winners()
{
	OnMatchDataChanged.Broadcast();
}

void ACIRMatchGameState::MulticastPlayPickupEffect_Implementation(FVector EffectLocation, bool bIsSpeedBuff)
{
	UNiagaraSystem* SelectedEffect = bIsSpeedBuff ? SpeedPickupEffect.Get() : CoinPickupEffect.Get();
	USoundBase* SelectedSound = bIsSpeedBuff ? SpeedPickupSound.Get() : CoinPickupSound.Get();

	if (IsValid(SelectedEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			SelectedEffect,
			EffectLocation,
			FRotator::ZeroRotator
		);
	}

	if (IsValid(SelectedSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			SelectedSound,
			EffectLocation
		);
	}
}
