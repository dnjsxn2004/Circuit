// Fill out your copyright notice in the Description page of Project Settings.


#include "CircuitPlayerState.h"
#include "Net/UnrealNetwork.h"

void ACircuitPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, BonusMaxSpeed, COND_OwnerOnly);
}

void ACircuitPlayerState::SetChatColorSlot(int NewSlot)
{
	if (!HasAuthority())
	{
		return;
	}

	ChatColorSlot = NewSlot;
}

void ACircuitPlayerState::AddCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}

	Coins += Amount;
	OnCoinsChanged.Broadcast(Coins);
}

bool ACircuitPlayerState::TrySpendCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Coins < Amount)
	{
		return false;
	}

	Coins -= Amount;
	OnCoinsChanged.Broadcast(Coins);

	return true;
}

void ACircuitPlayerState::IncreaseBonusMaxSpeed(float Amount)
{
	if (!HasAuthority() || Amount <= 0.0f)
	{
		return;
	}

	BonusMaxSpeed += Amount;
	OnSpeedChanged.Broadcast(BonusMaxSpeed);
}

void ACircuitPlayerState::OnRep_Coins()
{
	OnCoinsChanged.Broadcast(Coins);
}


void ACircuitPlayerState::OnRep_BonusMaxSpeed()
{
	OnSpeedChanged.Broadcast(BonusMaxSpeed);
}
