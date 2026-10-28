// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CircuitPlayerState.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCoinsChanged, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpeedChanged, float);

UCLASS()
class CIRCUIT_API ACircuitPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetChatColorSlot() const{ return ChatColorSlot; }
	int32 GetCoins() const{ return Coins; }

	void SetChatColorSlot(int NewSlot);

	void AddCoins(int32 Amount);	
	bool TrySpendCoins(int32 Amount);

	void IncreaseBonusMaxSpeed(float Amount);
	
	float GetBonusMaxSpeed() const { return BonusMaxSpeed; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_BonusMaxSpeed)
	float BonusMaxSpeed = 0.0f;

private:
	UPROPERTY()
	int32 ChatColorSlot = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_Coins)
	int32 Coins = 0;

private:
	UFUNCTION()
	void OnRep_Coins();

	UFUNCTION()
	void OnRep_BonusMaxSpeed();

public:
	FOnCoinsChanged OnCoinsChanged;
	FOnSpeedChanged OnSpeedChanged;

};
