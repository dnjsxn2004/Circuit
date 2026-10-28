// Fill out your copyright notice in the Description page of Project Settings.


#include "CircuitMatchWidget.h"
#include "Components/TextBlock.h"
#include "../Player/CircuitPlayerState.h"
#include "../Game/CIRMatchGameState.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/Border.h"

void UCircuitMatchWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindGameData();

    if (UWorld* World = GetWorld())
    {
        if (!BoundPlayerState.IsValid() ||
            !BoundGameState.IsValid())
        {
            World->GetTimerManager().SetTimer(
                BindRetryTimerHandle,
                this,
                &ThisClass::RetryBindGameData,
                0.2f,
                true
            );
        }
    }
}

void UCircuitMatchWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            BindRetryTimerHandle
        );
    }

    UnbindGameData();

    Super::NativeDestruct();
}

void UCircuitMatchWidget::UpdateCoins(int32 NewCoins)
{
    if (!IsValid(CoinText))
    {
        return;
    }

    CoinText->SetText( FText::AsNumber(NewCoins));
}

void UCircuitMatchWidget::UpdateSpeed(float NewBonusSpeed)
{
    if (!IsValid(SpeedText))
    {
        return;
    }

    SpeedText->SetText(FText::FromString(FString::Printf(TEXT("+%.0f km/h"), NewBonusSpeed)));
}

void UCircuitMatchWidget::UpdateMatchData()
{
    if (!BoundGameState.IsValid())
    {
        return;
    }

    const ECIRMatchPhase Phase =
        BoundGameState->GetMatchPhase();

    FString PhaseString;

    switch (Phase)
    {
    case ECIRMatchPhase::Waiting:
        PhaseString = TEXT("Waiting");
        break;

    case ECIRMatchPhase::Playing:
        PhaseString = TEXT("Playing");
        break;

    case ECIRMatchPhase::Finished:
        PhaseString = TEXT("Finished");
        break;

    default:
        PhaseString = TEXT("Unknown");
        break;
    }

    if (IsValid(MatchPhaseText))
    {
        MatchPhaseText->SetText(
            FText::FromString(PhaseString)
        );
    }

    if (IsValid(RemainingTimeText))
    {
        const int32 RemainingSeconds =
            BoundGameState->GetRemainingTime();

        const int32 Minutes = RemainingSeconds / 60;
        const int32 Seconds = RemainingSeconds % 60;

        RemainingTimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds)));
    }

    UpdateMatchResult();
}

void UCircuitMatchWidget::BindGameData()
{
    UnbindGameData();

    APlayerController* PC = GetOwningPlayer();

    if (!IsValid(PC))
    {
        return;
    }

    ACircuitPlayerState* PS = PC->GetPlayerState<ACircuitPlayerState>();

    if (IsValid(PS))
    {
        CoinsChangedHandle = PS->OnCoinsChanged.AddUObject(
            this,
            &ThisClass::UpdateCoins
        );

        SpeedChangedHandle = PS->OnSpeedChanged.AddUObject(
            this,
            &ThisClass::UpdateSpeed
        );

        BoundPlayerState = PS;

        UpdateCoins(PS->GetCoins());
        UpdateSpeed(PS->GetBonusMaxSpeed());

        UpdateMatchResult();
    }

    ACIRMatchGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACIRMatchGameState>() : nullptr;

    if (IsValid(GS))
    {
        MatchDataChangedHandle = GS->OnMatchDataChanged.AddUObject(
            this,
            &ThisClass::UpdateMatchData
        );

        BoundGameState = GS;
        UpdateMatchData();
    }
}

void UCircuitMatchWidget::UnbindGameData()
{
    if (BoundPlayerState.IsValid())
    {
        if (CoinsChangedHandle.IsValid())
        {
            BoundPlayerState->OnCoinsChanged.Remove(CoinsChangedHandle);
        }

        if (SpeedChangedHandle.IsValid())
        {
            BoundPlayerState->OnSpeedChanged.Remove(SpeedChangedHandle);
        }
    }

    if (BoundGameState.IsValid() && MatchDataChangedHandle.IsValid())
    {
        BoundGameState->OnMatchDataChanged.Remove(MatchDataChangedHandle);
    }

    CoinsChangedHandle.Reset();
    SpeedChangedHandle.Reset();
    MatchDataChangedHandle.Reset();

    BoundPlayerState.Reset();
    BoundGameState.Reset();
}

void UCircuitMatchWidget::RetryBindGameData()
{
    if (BoundPlayerState.IsValid() && BoundGameState.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(
            BindRetryTimerHandle
        );
        return;
    }

    BindGameData();
}

void UCircuitMatchWidget::UpdateMatchResult()
{
    if (!IsValid(ResultPanel) || !IsValid(ResultText))
    {
        return;
    }

    ResultPanel->SetVisibility(ESlateVisibility::Collapsed);

    if (!BoundGameState.IsValid() ||
        !BoundPlayerState.IsValid())
    {
        return;
    }

    if (BoundGameState->GetMatchPhase() != ECIRMatchPhase::Finished)
    {
        return;
    }

    const TArray<TObjectPtr<APlayerState>>& Winners = BoundGameState->GetWinners();

    if (Winners.IsEmpty())
    {
        return;
    }

    bool bIsWinner = false;

    for (const TObjectPtr<APlayerState>& Winner : Winners)
    {
        if (Winner.Get() == BoundPlayerState.Get())
        {
            bIsWinner = true;
            break;
        }
    }

    FString ResultString;

    if (!bIsWinner)
    {
        ResultString = TEXT("DEFEAT");
    }
    else if (Winners.Num() > 1)
    {
        ResultString = TEXT("DRAW");
    }
    else
    {
        ResultString = TEXT("VICTORY");
    }

    ResultText->SetText(FText::FromString(ResultString));

    ResultPanel->SetVisibility(ESlateVisibility::Visible);
}
