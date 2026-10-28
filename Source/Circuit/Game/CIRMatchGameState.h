// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "../CIRMatchTypes.h"
#include "CIRMatchGameState.generated.h"

class APlayerState;
class UNiagaraSystem;
class USoundBase;

DECLARE_MULTICAST_DELEGATE(FOnMatchDataChanged);
/**
 * 
 */
UCLASS()
class CIRCUIT_API ACIRMatchGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ACIRMatchGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ECIRMatchPhase GetMatchPhase() const { return MatchPhase; }
	int32 GetRemainingTime() const { return RemainingTime; }

	const TArray<TObjectPtr<APlayerState>>& GetWinners() const { return Winners; }

	void SetMatchPhase(ECIRMatchPhase NewPhase);
	void SetRemainingTime(int32 NewTime);
	void SetWinners(const TArray<APlayerState*>& NewWinners);

	FOnMatchDataChanged OnMatchDataChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	ECIRMatchPhase MatchPhase = ECIRMatchPhase::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	int32 RemainingTime = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Winners)
	TArray<TObjectPtr<APlayerState>> Winners;

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_Winners();

public:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayPickupEffect(FVector EffectLocation, bool bIsSpeedBuff);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Coin")
	TObjectPtr<UNiagaraSystem> CoinPickupEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Coin")
	TObjectPtr<USoundBase> CoinPickupSound;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Speed")
	TObjectPtr<UNiagaraSystem> SpeedPickupEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Speed")
	TObjectPtr<USoundBase> SpeedPickupSound;
};
