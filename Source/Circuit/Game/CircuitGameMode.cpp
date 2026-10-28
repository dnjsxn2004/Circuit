// Copyright Epic Games, Inc. All Rights Reserved.

#include "CircuitGameMode.h"
#include "../Player/CircuitPlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "../Player/CircuitPlayerState.h"
#include "CIRMatchGameState.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "CIRMatchGameState.h"

ACircuitGameMode::ACircuitGameMode()
{
	PlayerControllerClass = ACircuitPlayerController::StaticClass();
	PlayerStateClass = ACircuitPlayerState::StaticClass();

	GameStateClass = ACIRMatchGameState::StaticClass();
}

void ACircuitGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!IsValid(NewPlayer))
	{
		return;
	}

	ACircuitPlayerState* PS = NewPlayer->GetPlayerState<ACircuitPlayerState>();

	if (!IsValid(PS))
	{
		return;
	}

	for (int32 Slot = 0; Slot < 4; ++Slot)
	{
		if (!UsedChatColorSlots.Contains(Slot))
		{
			UsedChatColorSlots.Add(Slot);
			PS->SetChatColorSlot(Slot);
			break;
		}
	}

	ACIRMatchGameState* GS = GetMatchState();

	if (!IsValid(GS))
	{
		return;
	}

	if (GS->GetMatchPhase() != ECIRMatchPhase::Waiting)
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(MatchTimerHandle))
	{
		return;
	}

	StartWaiting();
}

void ACircuitGameMode::Logout(AController* Exiting)
{
	if (IsValid(Exiting))
	{
		if (ACircuitPlayerState* PS = Exiting->GetPlayerState<ACircuitPlayerState>())
		{
			UsedChatColorSlots.Remove(PS->GetChatColorSlot());
			PS->SetChatColorSlot(INDEX_NONE);
		}
	}

	Super::Logout(Exiting);
}

AActor* ACircuitGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	if (AvailablePlayerStarts.IsEmpty())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("There is no available Vehicle PlayerStart.")
		);

		return nullptr;
	}

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());

	return AvailablePlayerStarts[RandomIndex];
	
}

void ACircuitGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	Super::EndPlay(EndPlayReason);
}

bool ACircuitGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (!IsValid(PlayerStart) || !IsValid(GetWorld()))
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();

	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);

	const FQuat CheckRotation = PlayerStartTransform.GetRotation();

	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);

	FCollisionObjectQueryParams ObjectQueryParams;

	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists = GetWorld()->OverlapAnyTestByObjectType(
		CheckLocation,
		CheckRotation,
		ObjectQueryParams,
		CheckShape,
		QueryParams
	);

	return !bVehicleExists;
}

void ACircuitGameMode::StartWaiting()
{
	ACIRMatchGameState* GS = GetMatchState();

	if (!IsValid(GS))
	{
		return;
	}

	GS->SetMatchPhase(ECIRMatchPhase::Waiting);
	GS->SetRemainingTime(WaitingDuration);

	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ThisClass::UpdateMatchTimer,
		1.0f,
		true
	);
}

void ACircuitGameMode::StartPlaying()
{
	ACIRMatchGameState* GS = GetMatchState();

	if (!IsValid(GS))
	{
		return;
	}

	GS->SetRemainingTime(PlayingDuration);
	GS->SetMatchPhase(ECIRMatchPhase::Playing);
}

void ACircuitGameMode::FinishMatch()
{
	ACIRMatchGameState* GS = GetMatchState();

	if (!IsValid(GS))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	DetermineWinners();

	GS->SetRemainingTime(0);
	GS->SetMatchPhase(ECIRMatchPhase::Finished);
}

void ACircuitGameMode::UpdateMatchTimer()
{
	ACIRMatchGameState* GS = GetMatchState();

	if (!IsValid(GS))
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		return;
	}

	const int32 NewTime = GS->GetRemainingTime() - 1;

	GS->SetRemainingTime(NewTime);

	if (NewTime > 0)
	{
		return;
	}

	switch (GS->GetMatchPhase())
	{
	case ECIRMatchPhase::Waiting:
		StartPlaying();
		break;

	case ECIRMatchPhase::Playing:
		FinishMatch();
		break;

	case ECIRMatchPhase::Finished:
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		break;
	}
}

void ACircuitGameMode::DetermineWinners()
{
	ACIRMatchGameState* GS = GetMatchState();

	if (!IsValid(GS))
	{
		return;
	}

	int32 HighestCoins = MIN_int32;
	TArray<APlayerState*> WinnerList;

	for (APlayerState* PS : GS->PlayerArray)
	{
		ACircuitPlayerState* CircuitPS = Cast<ACircuitPlayerState>(PS);
		
		if (!IsValid(CircuitPS))
		{
			continue;
		}

		const int32 PlayerCoins = CircuitPS->GetCoins();

		if (PlayerCoins > HighestCoins)
		{
			HighestCoins = PlayerCoins;

			WinnerList.Reset();
			WinnerList.Add(CircuitPS);
		}
		else if (PlayerCoins == HighestCoins)
		{
			WinnerList.Add(CircuitPS);
		}
	}

	GS->SetWinners(WinnerList);
}

ACIRMatchGameState* ACircuitGameMode::GetMatchState() const
{
	return GetGameState<ACIRMatchGameState>();
}
