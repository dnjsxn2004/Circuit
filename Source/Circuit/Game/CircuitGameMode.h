// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "CircuitGameMode.generated.h"

class ACIRMatchGameState;

UCLASS(MinimalAPI)
class ACircuitGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACircuitGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

	TSet<int32> UsedChatColorSlots;

	FTimerHandle MatchTimerHandle;

	void StartWaiting();
	void StartPlaying();
	void FinishMatch();

	void UpdateMatchTimer();
	void DetermineWinners();

	ACIRMatchGameState* GetMatchState() const;

private:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Match|Time")
	int32 WaitingDuration = 30;
	
	UPROPERTY(EditDefaultsOnly, Category = "Match|Time")
	int32 PlayingDuration = 180;

};



